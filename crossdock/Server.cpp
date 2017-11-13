/*
 * Copyright (c) 2017 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Server.h"

#include <atomic>
#include <cstdlib>
#include <future>
#include <sstream>
#include <thread>

#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/transport/TBufferTransports.h>

#include "jaegertracing/Tracer.h"
#include "jaegertracing/net/IPAddress.h"
#include "jaegertracing/net/Socket.h"
#include "jaegertracing/net/http/Request.h"
#include "jaegertracing/net/http/Response.h"

namespace jaegertracing {
namespace crossdock {
namespace {

constexpr auto kBaggageKey = "crossdock-baggage-key";
constexpr auto kDefaultTracerServiceName = "crossdock-cpp";

std::string bufferedRead(net::Socket& socket)
{
    constexpr auto kBufferSize = 256;
    std::array<char, kBufferSize> buffer;
    std::string data;
    auto numRead = ::read(socket.handle(), &buffer[0], buffer.size());
    data.append(&buffer[0], numRead);
    while (numRead == kBufferSize) {
        numRead = ::read(socket.handle(), &buffer[0], buffer.size());
        data.append(&buffer[0], numRead);
    }
    return data;
}

template <typename ResultType>
ResultType convertFromJSON(const std::string& jsonStr)
{
    boost::shared_ptr<apache::thrift::transport::TMemoryBuffer> transport(
        new apache::thrift::transport::TMemoryBuffer());
    apache::thrift::protocol::TJSONProtocol protocol(transport);
    std::vector<uint8_t> buffer;
    buffer.reserve(jsonStr.size());
    buffer.insert(std::end(buffer), std::begin(jsonStr), std::end(jsonStr));
    transport->write(&buffer[0], buffer.size());
    ResultType result;
    result.read(&protocol);
    return result;
}

class RequestReader : public opentracing::HTTPHeadersReader {
  public:
    explicit RequestReader(const net::http::Request& request)
        : _request(request)
    {
    }

    opentracing::expected<void> ForeachKey(
        std::function<opentracing::expected<void>(opentracing::string_view,
                                                  opentracing::string_view)> f)
        const override
    {
        for (auto&& header : _request.headers()) {
            const auto result = f(header.key(), header.value());
            if (!result) {
                return result;
            }
        }
        return opentracing::make_expected();
    }

  private:
    const net::http::Request& _request;
};

thrift::ObservedSpan observeSpan(const opentracing::SpanContext& ctx)
{
    const auto& sc = static_cast<const SpanContext&>(ctx);
    thrift::ObservedSpan observedSpan;
    std::ostringstream oss;
    oss << sc.traceID();
    observedSpan.__set_traceId(oss.str());
    observedSpan.__set_sampled(sc.isSampled());
    auto itr = sc.baggage().find(kBaggageKey);
    if (itr != std::end(sc.baggage())) {
        observedSpan.__set_baggage(itr->second);
    }
    return observedSpan;
}

thrift::TraceResponse callDownstreamHTTP(const opentracing::SpanContext& ctx,
                                         const thrift::Downstream& target)
{
    thrift::JoinTraceRequest request;
    request.__set_serverRole(target.serverRole);
    if (target.downstream) {
        request.__set_downstream(*target.downstream);
    }
    const auto requestJSON = apache::thrift::ThriftJSONString(request);
    net::Socket socket;
    socket.open(AF_INET, SOCK_STREAM);
    const auto authority = target.host + ':' + target.port;
    socket.connect("http://" + authority);
    std::ostringstream oss;
    oss << "POST /join_trace HTTP/1.1\r\n"
           "Host: "
        << authority << "\r\n"
                        "Content-Type: application/json\r\n"
                        "Content-Length: "
        << requestJSON.size() << "\r\n\r\n"
        << requestJSON;
    const auto message = oss.str();
    const auto numWritten =
        ::write(socket.handle(), &message[0], message.size());
    (void)numWritten;

    const auto responseStr = bufferedRead(socket);
    std::istringstream iss(responseStr);
    auto response = net::http::Response::parse(iss);
    const auto thriftResponse =
        convertFromJSON<thrift::TraceResponse>(response.body());
    return thriftResponse;
}

thrift::TraceResponse callDownstream(const opentracing::SpanContext& ctx,
                                     const std::string& /* role */,
                                     const thrift::Downstream& downstream)
{
    thrift::TraceResponse response;

    switch (downstream.transport) {
    case thrift::Transport::HTTP: {
        response = callDownstreamHTTP(ctx, downstream);
    } break;
    case thrift::Transport::TCHANNEL: {
        response.__set_notImplementedError(
            "TCHANNEL transport not implemented");
    } break;
    case thrift::Transport::DUMMY: {
        response.__set_notImplementedError("DUMMY transport not implemented");
    } break;
    default: {
        throw std::invalid_argument("Unrecognized protocol " +
                                    std::to_string(downstream.transport));
    } break;
    }

    return response;
}

thrift::TraceResponse prepareResponse(const opentracing::SpanContext& ctx,
                                      const std::string& role,
                                      const thrift::Downstream& downstream)
{
    const auto observedSpan = observeSpan(ctx);
    thrift::TraceResponse response;
    response.__set_span(observedSpan);
    if (downstream.downstream) {
        response.__set_downstream(
            callDownstream(ctx, role, *downstream.downstream));
    }
    return response;
}

}  // anonymous namespace

using Handler = std::function<std::string(const net::http::Request&)>;

class Server::SocketListener {
  public:
    SocketListener(const net::IPAddress& ip, Handler handler)
        : _ip(ip)
        , _handler(handler)
        , _running(false)
    {
    }

    ~SocketListener() { stop(); }

    void start()
    {
        std::promise<void> started;
        _thread = std::thread([this, &started]() { start(_ip, started); });
        started.get_future().get();
    }

    void stop() noexcept
    {
        if (_running) {
            _running = false;
            _thread.join();
            _socket.close();
        }
    }

  private:
    void start(const net::IPAddress& ip, std::promise<void>& started)
    {
        _socket.open(AF_INET, SOCK_STREAM);
        const auto enable = 1;
        ::setsockopt(_socket.handle(),
                     SOL_SOCKET,
                     SO_REUSEADDR,
                     &enable,
                     sizeof(enable));
        _socket.bind(ip);
        _socket.listen();
        _running = true;
        started.set_value();

        while (_running) {
            net::Socket client = _socket.accept();
            auto requestStr = bufferedRead(client);

            try {
                std::istringstream iss(requestStr);
                const auto request = net::http::Request::parse(iss);
                const auto responseStr = _handler(request);
                const auto numWritten = ::write(
                    client.handle(), &responseStr[0], responseStr.size());
                (void)numWritten;
            } catch (...) {
                constexpr auto message =
                    "HTTP/1.1 500 Internal Server Error\r\n\r\n";
                const auto numWritten =
                    ::write(client.handle(), message, sizeof(message) - 1);
                (void)numWritten;
            }

            client.close();
        }
    }

    net::IPAddress _ip;
    net::Socket _socket;
    Handler _handler;
    std::atomic<bool> _running;
    std::thread _thread;
};

class Server::EndToEndHandler {
  public:
    using TracerPtr = std::shared_ptr<opentracing::Tracer>;

    EndToEndHandler(const std::string& agentHostPort,
                    const std::string& samplingServerURL)
        : _agentHostPort(agentHostPort)
        , _samplingServerURL(samplingServerURL)
    {
    }

    TracerPtr findOrMakeTracer(std::string samplerType)
    {
        if (samplerType.empty()) {
            samplerType = kSamplerTypeRemote;
        }

        std::lock_guard<std::mutex> lock(_mutex);
        auto itr = _tracers.find(samplerType);
        if (itr != std::end(_tracers)) {
            return itr->second;
        }
        return init(samplerType);
    }

  private:
    Config makeEndToEndConfig(const std::string& samplerType) const
    {
        return Config(
            false,
            samplers::Config(samplerType,
                             1.0,
                             _samplingServerURL,
                             samplers::Config::kDefaultMaxOperations,
                             std::chrono::seconds(5)),
            reporters::Config(reporters::Config::kDefaultQueueSize,
                              std::chrono::seconds(1),
                              false,
                              _agentHostPort));
    }

    TracerPtr init(const std::string& samplerType)
    {
        const auto config = makeEndToEndConfig(samplerType);
        auto tracer = Tracer::make(kDefaultTracerServiceName, config);
        _tracers[config.sampler().type()] = tracer;
        return tracer;
    }

    std::string _agentHostPort;
    std::string _samplingServerURL;
    std::unordered_map<std::string, TracerPtr> _tracers;
    std::mutex _mutex;
};

Server::Server(const net::IPAddress& ip,
               const std::string& agentHostPort,
               const std::string& samplingServerURL)
    : _logger(logging::consoleLogger())
    , _tracer(Tracer::make(kDefaultTracerServiceName, Config(), _logger))
    , _listener(
          new SocketListener(ip, [this](const net::http::Request& request) {
              return handleRequest(request);
          }))
    , _handler(new EndToEndHandler(agentHostPort, samplingServerURL))
{
}

Server::~Server() = default;

void Server::serve() { _listener->start(); }

template <typename RequestType>
std::string Server::handleJSON(
    const net::http::Request& request,
    std::function<thrift::TraceResponse(
        const RequestType&, const opentracing::SpanContext&)> handler)
{
    RequestReader reader(request);
    auto result = _tracer->Extract(reader);
    if (!result) {
        std::ostringstream oss;
        oss << "HTTP/1.1 400 Bad Request\r\n\r\n"
               "Cannot read request body: opentracing error code "
            << result.error().value();
        return oss.str();
    }

    std::unique_ptr<opentracing::SpanContext> ctx(result->release());
    opentracing::StartSpanOptions options;
    options.start_system_timestamp = std::chrono::system_clock::now();
    options.start_steady_timestamp = std::chrono::steady_clock::now();
    if (ctx) {
        options.references.emplace_back(std::make_pair(
            opentracing::SpanReferenceType::ChildOfRef, ctx.get()));
    }
    auto span = _tracer->StartSpanWithOptions("post", options);

    _logger->info("Server request: " + request.body());
    RequestType thriftRequest;
    try {
        thriftRequest = convertFromJSON<RequestType>(request.body());
    } catch (const std::exception& ex) {
        std::ostringstream oss;
        oss << "HTTP/1.1 500 Internal Server Error\r\n\r\n"
            << "Cannot parse request JSON: " << ex.what();
        return oss.str();
    } catch (...) {
        return "HTTP/1.1 500 Internal Server Error\r\n\r\n"
               "Cannot parse request JSON";
    }

    const auto thriftResponse = handler(thriftRequest, span->context());
    std::string responseJSONStr;
    try {
        responseJSONStr = apache::thrift::ThriftJSONString(thriftResponse);
    } catch (const std::exception& ex) {
        std::ostringstream oss;
        oss << "HTTP/1.1 500 Internal Server Error\r\n\r\n"
            << "Cannot marshal response to JSON: " << ex.what();
        return oss.str();
    } catch (...) {
        return "HTTP/1.1 500 Internal Server Error\r\n\r\n"
               "Cannot marshal response to JSON";
    }
    _logger->info("Server response: " + responseJSONStr);
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json\r\n"
           "Content-Length: "
        << responseJSONStr.size() << "\r\n\r\n"
        << responseJSONStr;
    return oss.str();
}

std::string Server::handleRequest(const net::http::Request& request)
{
    if (request.target() == "/") {
        return "HTTP/1.1 200 OK\r\n\r\n";
    }
    if (request.target() == "/start_trace") {
        return handleJSON<thrift::StartTraceRequest>(
            request,
            [this](const thrift::StartTraceRequest& request,
                   const opentracing::SpanContext& /* ctx */) {
                return startTrace(request);
            });
    }
    if (request.target() == "/join_trace") {
        return handleJSON<thrift::JoinTraceRequest>(
            request,
            [this](const thrift::JoinTraceRequest& request,
                   const opentracing::SpanContext& ctx) {
                return joinTrace(request, ctx);
            });
    }
    if (request.target() == "/create_traces") {
        return generateTraces(request);
    }
    return "HTTP/1.1 404 Not Found\r\n\r\n";
}

thrift::TraceResponse
Server::startTrace(const crossdock::thrift::StartTraceRequest& request)
{
    auto span = _tracer->StartSpan(request.serverRole);
    if (request.sampled) {
        span->SetTag("sampling.priority", 1);
    }
    span->SetBaggageItem(kBaggageKey, request.baggage);

    return prepareResponse(
        span->context(), request.serverRole, request.downstream);
}

thrift::TraceResponse
Server::joinTrace(const crossdock::thrift::JoinTraceRequest& request,
                  const opentracing::SpanContext& ctx)
{
    return prepareResponse(ctx, request.serverRole, request.downstream);
}

std::string Server::generateTraces(const net::http::Request& request)
{
    using StrMap = std::unordered_map<std::string, std::string>;

    std::string type;
    std::string operation;
    StrMap tags;
    int count = 0;

    try {
        auto node = YAML::Load(request.body());
        type = node["type"].as<std::string>();
        operation = node["operation"].as<std::string>();
        count = node["count"].as<int>();

        auto tagNode = node["tags"];
        if (tagNode.IsMap()) {
            for (auto itr = tagNode.begin(); itr != tagNode.end(); ++itr) {
                tags.emplace(
                    std::make_pair(itr->first.as<std::string>(),
                                   itr->second.as<std::string>()));
            }
        }
    } catch (const std::exception& ex) {
        std::ostringstream oss;
        oss << "HTTP/1.1 400 Bad Request\r\n\r\n"
               "JSON payload is invalid: " << ex.what();
        return oss.str();
    } catch (...) {
        return "HTTP/1.1 400 Bad Request\r\n\r\n"
               "JSON payload is invalid";
    }

    auto tracer = _handler->findOrMakeTracer(type);
    if (!tracer) {
        return "HTTP/1.1 500 Internal Server Error\r\n\r\n"
               "Tracer is not initialized";
    }

    for (auto i = 0; i < count; ++i) {
        auto span = tracer->StartSpan(operation);
        for (auto&& pair : tags) {
            span->SetTag(pair.first, pair.second);
        }
        span->Finish();
    }

    return "HTTP/1.1 200 OK\r\n\r\n";
}

}  // namespace crossdock
}  // namespace jaegertracing

int main()
{
    const auto rawAgentHostPort = std::getenv("AGENT_HOST_PORT");
    const std::string agentHostPort(rawAgentHostPort ? rawAgentHostPort : "");
    if (agentHostPort.empty()) {
        std::cerr << "env AGENT_HOST_PORT is not specified!\n";
        return 1;
    }

    const auto rawSamplingServerURL = std::getenv("SAMPLING_SERVER_URL");
    const std::string samplingServerURL(
        rawSamplingServerURL ? rawSamplingServerURL : "");
    if (samplingServerURL.empty()) {
        std::cerr << "env SAMPLING_SERVER_URL is not specified!\n";
        return 1;
    }

    jaegertracing::crossdock::Server server(
        jaegertracing::net::IPAddress::v4("127.0.0.1:8888"),
        agentHostPort,
        samplingServerURL);
    server.serve();
    std::this_thread::sleep_for(std::chrono::minutes(10));
    return 0;
}
