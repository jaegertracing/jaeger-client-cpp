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

#include "jaegertracing/reporters/RemoteReporter.h"

#include <iostream>
#include <sstream>

#include "jaegertracing/utils/ErrorUtil.h"

namespace jaegertracing {
namespace reporters {

RemoteReporter::RemoteReporter(const Clock::duration& bufferFlushInterval,
                               int fixedQueueSize,
                               std::unique_ptr<Transport>&& sender,
                               logging::Logger& logger,
                               metrics::Metrics& metrics)
    : _bufferFlushInterval(bufferFlushInterval)
    , _fixedQueueSize(fixedQueueSize)
    , _sender(std::move(sender))
    , _logger(logger)
    , _metrics(metrics)
    , _queue()
    , _queueLength(0)
    , _running(true)
    , _lastFlush(Clock::now())
    , _cv()
    , _cv_flush()
    , _mutex()
    , _thread()
{
    _thread = std::thread([this]() { sweepQueue(); });
}

void RemoteReporter::report(const Span& span)
{
    std::unique_lock<std::mutex> lock(_mutex);
    const auto pushed = (static_cast<int>(_queue.size()) < _fixedQueueSize);
    if (pushed) {
        _queue.push_back(span);
        lock.unlock();
        _cv.notify_one();
        ++_queueLength;
    }
    else {
        _metrics.reporterDropped().inc(1);
    }
}

void RemoteReporter::close()
{
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (!_running) {
            return;
        }
        _running = false;
        lock.unlock();
        _cv.notify_one();
    }
    _thread.join();
}

void RemoteReporter::sweepQueue()
{
    while (true) {
        try {
            std::unique_lock<std::mutex> lock(_mutex);
            _cv.wait_until(lock, _lastFlush + _bufferFlushInterval, [this]() {
                return !_running || !_queue.empty();
            });

            if (!_running && _queue.empty()) {
                return;
            }

            if (!_queue.empty()) {
                const auto span = _queue.front();
                _queue.pop_front();
                --_queueLength;
                sendSpan(span);
            }
            else if (bufferFlushIntervalExpired()) {
                async_flush();
            }
			/* If anyone's waiting on flush completion, notify them */
            _cv_flush.notify_one();
        } catch (...) {
            auto logger = logging::consoleLogger();
            assert(logger);
            utils::ErrorUtil::logError(*logger,
                                       "Failed in Reporter::sweepQueue");
        }
    }
}

void RemoteReporter::sendSpan(const Span& span)
{
    try {
        const auto flushed = _sender->append(span);
        if (flushed > 0) {
            _metrics.reporterSuccess().inc(flushed);
            _metrics.reporterQueueLength().update(_queueLength);
        }
    } catch (const Transport::Exception& ex) {
        _metrics.reporterFailure().inc(ex.numFailed());
        std::ostringstream oss;
        oss << "error reporting span " << span.operationName() << ": "
            << ex.what();
        _logger.error(oss.str());
    }
}

void RemoteReporter::async_flush()
{
    try {
        const auto flushed = _sender->flush();
        if (flushed > 0) {
            _metrics.reporterSuccess().inc(flushed);
            _cv.notify_one();
        }
    } catch (const Transport::Exception& ex) {
        _metrics.reporterFailure().inc(ex.numFailed());
        _logger.error(ex.what());
    }

    _lastFlush = Clock::now();
}

void RemoteReporter::flush()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _cv_flush.wait(lock, [this]() {
        async_flush();
        return !_running || _queue.empty();
    });
}

}  // namespace reporters
}  // namespace jaegertracing
