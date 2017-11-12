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

#ifndef JAEGERTRACING_CROSSDOCK_SERVER_H
#define JAEGERTRACING_CROSSDOCK_SERVER_H

#include <memory>

#include <opentracing/tracer.h>

namespace jaegertracing {
namespace net {

class IPAddress;

namespace http {

class Request;

}  // namespace http
}  // namespace net

namespace crossdock {
namespace thrift {

class JoinTraceRequest;
class StartTraceRequest;

}  // namespace thrift

class Server {
  public:
    explicit Server(const net::IPAddress& ip);

    ~Server();

    void serve();

  private:
    std::string handleRequest(const net::http::Request& request);

    std::string startTrace(const crossdock::thrift::StartTraceRequest& request);

    std::string joinTrace(const crossdock::thrift::JoinTraceRequest& request);

    std::string generateTraces(const net::http::Request& request);

    class SocketListener;

    std::unique_ptr<SocketListener> _listener;
    std::shared_ptr<opentracing::Tracer> _tracer;
};

}  // namespace crossdock
}  // namespace jaegertracing

#endif  // JAEGERTRACING_CROSSDOCK_SERVER_H
