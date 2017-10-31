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

#ifndef JAEGERTRACING_EXAMPLES_HOTROD_HTTPSERVER_H
#define JAEGERTRACING_EXAMPLES_HOTROD_HTTPSERVER_H

#include <deque>
#include <functional>
#include <future>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "jaegertracing/net/IPAddress.h"
#include "jaegertracing/net/Socket.h"
#include "jaegertracing/net/http/Request.h"

namespace jaegertracing {
namespace examples {
namespace hotrod {

class HTTPServer {
  public:
    using Handler =
        std::function<void(net::Socket&&, const net::http::Request&)>;
    using HandlerVec = std::vector<std::pair<std::regex, Handler>>;
    using TaskList = std::deque<std::future<void>>;

    explicit HTTPServer(const net::IPAddress& address);

    ~HTTPServer() { close(); }

    void registerHandler(const std::regex& pattern, Handler handler);

    void start();

    void close() noexcept;

  private:
    net::IPAddress _address;
    net::Socket _socket;
    HandlerVec _handlers;
    TaskList _tasks;
};

}  // namespace hotrod
}  // namespace examples
}  // namespace jaegertracing

#endif  // JAEGERTRACING_EXAMPLES_HOTROD_HTTPSERVER_H
