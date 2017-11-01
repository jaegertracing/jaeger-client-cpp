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

#include "HTTPServer.h"

#include <iostream>
#include <sstream>

#include "jaegertracing/utils/ErrorUtil.h"

namespace jaegertracing {
namespace examples {
namespace hotrod {
namespace {

auto defaultHandler =
    [](net::Socket&& socket, const net::http::Request& /* request */) {
        const std::string message =
            "HTTP/1.1 404 Not found\r\n";
        const auto numWritten =
            ::write(socket.handle(), &message[0], message.size());
        if (numWritten < static_cast<int>(message.size())) {
            std::cerr
                << "Failed to write entire response to client"
                   ", message=" << message << '\n';
        }
    };

}  // anonymous namespace

HTTPServer::HTTPServer(const net::IPAddress& address)
    : _address(address)
{
    _socket.open(AF_INET, SOCK_STREAM);
}

void HTTPServer::registerHandler(const std::regex& pattern, Handler handler)
{
    _handlers.emplace_back(std::make_pair(pattern, handler));
}

void HTTPServer::start()
{
    const auto enable = 1;
    ::setsockopt(
        _socket.handle(), SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    _socket.bind(_address);
    _socket.listen();
    while (true) {
        auto clientSocket = _socket.accept();

        constexpr auto kBufferSize = 256;
        std::string buffer(kBufferSize, '\0');
        std::string requestStr;
        auto numRead =
            ::read(clientSocket.handle(), &buffer[0], buffer.size());
        while (numRead == static_cast<int>(buffer.size())) {
            requestStr.append(&buffer[0], numRead);
            numRead = ::read(clientSocket.handle(), &buffer[0], buffer.size());
        }

        std::istringstream iss(requestStr);
        try {
            const auto request = net::http::Request::parse(iss);
            auto itr =
                std::find_if(std::begin(_handlers),
                             std::end(_handlers),
                             [&request](const HandlerVec::value_type& pair) {
                                 return std::regex_search(request.target(),
                                                          pair.first);
                             });

            Handler handler = ((itr == std::end(_handlers)) ? defaultHandler
                                                            : itr->second);

            std::packaged_task<void(net::Socket&&,
                                    const net::http::Request&)> task(
                handler);
            _tasks.push_back(task.get_future());
            task(std::move(clientSocket), request);
        } catch (...) {
            auto logger = logging::consoleLogger();
            utils::ErrorUtil::logError(*logger, "Error parsing request");
        }

        _tasks.erase(
            std::remove_if(std::begin(_tasks),
                           std::end(_tasks),
                           [](const TaskList::value_type& task) {
                               return task.valid();
                           }),
            std::end(_tasks));
    }
}

void HTTPServer::close() noexcept
{
    std::for_each(std::begin(_tasks),
                  std::end(_tasks),
                  [](TaskList::value_type& task) {
                      try {
                          task.get();
                      } catch (...) {
                          auto logger = logging::consoleLogger();
                          utils::ErrorUtil::logError(
                              *logger, "Error finishing task");
                      }
                  });
    _socket.close();
}

}  // namespace hotrod
}  // namespace examples
}  // namespace jaegertracing
