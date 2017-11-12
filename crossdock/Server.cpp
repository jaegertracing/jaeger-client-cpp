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
#include "jaegertracing/thrift-gen/tracetest_types.h"

namespace jaegertracing {
namespace crossdock {
namespace {

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

}  // anonymous namespace

using Handler = std::function<std::string(const net::http::Request&)>;

class Server::SocketListener {
  public:
    SocketListener(const net::IPAddress& ip,
                   Handler handler)
        : _handler(handler)
        , _running(true)
    {
        std::promise<void> started;
        _thread = std::thread([this, ip, &started]() { start(ip, started); });
        started.get_future().get();
    }

    ~SocketListener()
    {
        stop();
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
        _socket.bind(ip);
        _socket.listen();
        started.set_value();

        constexpr auto kBufferSize = 256;
        std::array<char, kBufferSize> buffer;
        while (_running) {
            net::Socket client = _socket.accept();
            std::string requestStr;
            auto numRead = ::read(client.handle(), &buffer[0], buffer.size());
            requestStr.append(buffer[0], numRead);
            while (numRead == kBufferSize) {
                numRead = ::read(client.handle(), &buffer[0], buffer.size());
                requestStr.append(buffer[0], numRead);
            }

            try {
                std::istringstream iss(requestStr);
                const auto request = net::http::Request::parse(iss);
                const auto responseStr = _handler(request);
                const auto numWritten =
                    ::write(_socket.handle(),
                            &responseStr[0],
                            responseStr.size());
                (void)numWritten;
            } catch (...) {
                constexpr auto message =
                    "HTTP/1.1 500 Internal Server Error\r\n\r\n";
                const auto numWritten =
                    ::write(_socket.handle(), message, sizeof(message) - 1);
                (void)numWritten;
            }
        }
    }

    net::Socket _socket;
    Handler _handler;
    std::atomic<bool> _running;
    std::thread _thread;
};

Server::Server(const net::IPAddress& ip)
{
    auto logger = std::shared_ptr<logging::Logger>(logging::consoleLogger());
    _tracer = Tracer::make("cppserver", Config(), logger);
    auto handler = [this](const net::http::Request& request) {
        return handleRequest(request);
    };
    _listener.reset(new SocketListener(ip, handler));
}

Server::~Server() = default;

std::string Server::handleRequest(const net::http::Request& request)
{
    if (request.target() == "/") {
        return "HTTP/1.1 200 OK\r\n\r\n";
    }
    if (request.target() == "/start_trace") {
        return startTrace(convertFromJSON<thrift::StartTraceRequest>(
            request.body()));
    }
    if (request.target() == "/join_trace") {
        return joinTrace(convertFromJSON<thrift::JoinTraceRequest>(
            request.body()));
    }
    if (request.target() == "/create_traces") {
        return generateTraces(request);
    }
    return "HTTP/1.1 404 Not Found\r\n\r\n";
}

std::string Server::startTrace(
    const crossdock::thrift::StartTraceRequest& request)
{
    // TODO
    return "";
}

std::string Server::joinTrace(
    const crossdock::thrift::JoinTraceRequest& request)
{
    // TODO
    return "";
}

std::string Server::generateTraces(const net::http::Request& request)
{
    // TODO
    return "";
}

}  // namespace crossdock
}  // namespace jaegertracing

int main()
{
    return 0;
}
