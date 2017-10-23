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

#ifndef JAEGERTRACING_TESTUTILS_MOCKAGENT_H
#define JAEGERTRACING_TESTUTILS_MOCKAGENT_H

#include <atomic>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

#include "jaegertracing/testutils/SamplingManager.h"
#include "jaegertracing/testutils/TUDPTransport.h"
#include "jaegertracing/thrift-gen/Agent.h"
#include "jaegertracing/thrift-gen/jaeger_types.h"
#include "jaegertracing/utils/UDPClient.h"

namespace jaegertracing {
namespace testutils {

class MockAgent : public agent::thrift::AgentIf,
                  public std::enable_shared_from_this<MockAgent> {
  public:
    static std::shared_ptr<MockAgent> make()
    {
        // Avoid `make_shared` when `weak_ptr` might be used.
        std::shared_ptr<MockAgent> newInstance(new MockAgent());
        return newInstance;
    }

    ~MockAgent();

    void start();

    void close();

    void
    emitZipkinBatch(const std::vector<twitter::zipkin::thrift::Span>&) override
    {
        throw std::logic_error("emitZipkinBatch not implemented");
    }

    void emitBatch(const thrift::Batch& batch) override;

    bool isServingUDP() const { return _servingUDP; }

    bool isServingHTTP() const { return _servingHTTP; }

    template <typename... Args>
    void addSamplingStrategy(Args&&... args)
    {
        _samplingMgr.addSamplingStrategy(std::forward<Args>(args)...);
    }

    std::vector<thrift::Batch> batches() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _batches;
    }

    net::IPAddress spanServerAddress() const { return _transport.addr(); }

    std::unique_ptr<agent::thrift::AgentIf> spanServerClient()
    {
        return std::unique_ptr<agent::thrift::AgentIf>(
            new utils::UDPClient(spanServerAddress(), 0));
    }

    net::IPAddress samplingServerAddr() const { return _httpAddress; }

    void resetBatches()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _batches.clear();
    }

  private:
    MockAgent();

    void serveUDP(std::promise<void>& started);

    void serveHTTP(std::promise<void>& started);

    TUDPTransport _transport;
    std::vector<thrift::Batch> _batches;
    std::atomic<bool> _servingUDP;
    std::atomic<bool> _servingHTTP;
    SamplingManager _samplingMgr;
    mutable std::mutex _mutex;
    std::thread _udpThread;
    std::thread _httpThread;
    net::IPAddress _httpAddress;
};

}  // namespace testutils
}  // namespace jaegertracing

#endif  // JAEGERTRACING_TESTUTILS_MOCKAGENT_H
