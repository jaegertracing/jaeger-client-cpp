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

#include "CustomerService.h"

#include <iostream>
#include <mutex>
#include <sstream>
#include <unordered_map>

#include <opentracing/tracer.h>

#include "jaegertracing/Logging.h"
#include "jaegertracing/Tracer.h"

#include "Delay.h"
#include "HTTPServer.h"

namespace jaegertracing {
namespace examples {
namespace hotrod {
namespace {

class Database {
  public:
    using Table = std::unordered_map<std::string, Customer>;

    Database(opentracing::Tracer& tracer, logging::Logger& logger)
        : _tracer(tracer)
        , _logger(logger)
        , _customers(
              { { "123",
                  Customer("123", "Rachel's Floral Designs", "115,227") },
                { "567",
                  Customer("567", "Amazing Coffee Roasters", "211,653") },
                { "392", Customer("392", "Trom Chocolatier", "577,322") },
                { "731", Customer("731", "Japanese Deserts", "728,326") } })
    {
    }

    Customer get(const std::string& customerID,
                 const opentracing::Span& parentSpan) const
    {
        _logger.info("Loading customer, customer_id=" + customerID);
        opentracing::StartSpanOptions options;
        options.start_system_timestamp = std::chrono::system_clock::now();
        options.start_steady_timestamp = std::chrono::steady_clock::now();
        options.references.emplace_back(
            opentracing::SpanReferenceType::ChildOfRef, &parentSpan.context());

        auto span = _tracer.StartSpanWithOptions("SQL SELECT", options);
        const auto query =
            "SELECT * FROM customer WHERE customer_id=" + customerID;
        span->SetTag("sql.query", query);
        // Span destructor calls `Finish`, so not invoking explicitly.

        std::lock_guard<std::mutex> lock(_mutex);
        delay::sleep(std::chrono::seconds(1), std::chrono::milliseconds(100));
        auto itr = _customers.find(customerID);
        if (itr == std::end(_customers)) {
            return Customer();
        }
        return itr->second;
    }

  private:
    opentracing::Tracer& _tracer;
    logging::Logger& _logger;
    Table _customers;
    mutable std::mutex _mutex;
};

class CustomerServiceImpl : public CustomerService {
  public:
    CustomerServiceImpl()
        : _logger(logging::consoleLogger())
        , _tracer(Tracer::make(
              "customer",
              Config(false,
                     samplers::Config("const", 1),
                     reporters::Config(
                         reporters::Config::kDefaultQueueSize,
                         reporters::Config::defaultBufferFlushInterval(),
                         true)),
              _logger))
        , _database(*_tracer, *_logger)
    {
    }

    Customer get(const std::string& customerID) override
    {
        // TODO: Consider client sending span to server.
        auto span = _tracer->StartSpan("GET /customer");
        return _database.get(customerID, *span);
    }

  private:
    std::shared_ptr<logging::Logger> _logger;
    std::shared_ptr<opentracing::Tracer> _tracer;
    Database _database;
};

}  // anonymous namespace

}  // namespace hotrod
}  // namespace examples
}  // namespace jaegertracing

int main(int argc, const char* argv[])
{
    namespace hotrod = jaegertracing::examples::hotrod;

    hotrod::CustomerServiceImpl impl;
    hotrod::HTTPServer server(
        jaegertracing::net::IPAddress::v4("127.0.0.1:8080"));
    server.registerHandler(
        std::regex("/customer"),
        [&impl](jaegertracing::net::Socket&& socket,
                const jaegertracing::net::http::Request& request) {
            const auto uri = jaegertracing::net::URI::parse(
                "http://127.0.0.1:8080" + request.target());
            const auto queryValues = uri.parseQueryValues();
            auto itr = queryValues.find("customer");
            if (itr == std::end(queryValues)) {
                const std::string message(
                    "HTTP/1.1 400 Bad Request\r\n\r\n"
                    "Missing required 'customer' parameter");
                const auto numWritten =
                    ::write(socket.handle(), &message[0], message.size());
                (void)numWritten;
                return;
            }

            const auto customerID = itr->second;
            const auto customer = impl.get(customerID);

            if (customer.id().empty()) {
                const std::string message(
                    "HTTP/1.1 500 Internal Server Error\r\n\r\n"
                    "Request failed, cannot find customer");
                const auto numWritten =
                    ::write(socket.handle(), &message[0], message.size());
                (void)numWritten;
                return;
            }

            std::ostringstream oss;
            oss << "{ \"ID\": \"" << customer.id() << '"' << ", \"name\": \""
                << customer.name() << '"' << ", \"location\": \""
                << customer.location() << '"' << " }";
            const auto jsonData = oss.str();
            oss.clear();
            oss.str("");
            oss << "HTTP/1.1 200 OK\r\n"
                << "Content-Length: " << jsonData.size() << "\r\n"
                << "Content-Type: application/json\r\n\r\n"
                << jsonData;
            const auto message = oss.str();
            const auto numWritten =
                ::write(socket.handle(), &message[0], message.size());
            (void)numWritten;
        });
    server.start();
    return 0;
}
