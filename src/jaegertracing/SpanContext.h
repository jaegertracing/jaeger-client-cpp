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

#ifndef JAEGERTRACING_SPANCONTEXT_H
#define JAEGERTRACING_SPANCONTEXT_H

#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>

#include <opentracing/span.h>

#include "jaegertracing/TraceID.h"

namespace jaegertracing {

class SpanContext : public opentracing::SpanContext {
  public:
    using StrMap = std::unordered_map<std::string, std::string>;

    enum class Flag : unsigned char { kSampled = 1, kDebug = 2 };

    static SpanContext fromStream(std::istream& in);

    SpanContext() = default;

    SpanContext(const TraceID& traceID,
                uint64_t spanID,
                uint64_t parentID,
                unsigned char flags,
                const StrMap& baggage,
                const std::string& debugID = "")
        : _traceID(traceID)
        , _spanID(spanID)
        , _parentID(parentID)
        , _flags(flags)
        , _baggage(baggage)
        , _debugID(debugID)
    {
    }

    const TraceID& traceID() const { return _traceID; }

    uint64_t spanID() const { return _spanID; }

    uint64_t parentID() const { return _parentID; }

    const StrMap& baggage() const { return _baggage; }

    SpanContext withBaggage(const StrMap& baggage) const
    {
        return SpanContext(
            _traceID, _spanID, _parentID, _flags, baggage, _debugID);
    }

    template <typename Function>
    void forEachBaggageItem(Function f) const
    {
        for (auto&& pair : _baggage) {
            if (!f(pair.first, pair.second)) {
                break;
            }
        }
    }

    template <typename Function>
    void forEachBaggageItem(Function f)
    {
        for (auto&& pair : _baggage) {
            if (!f(pair.first, pair.second)) {
                break;
            }
        }
    }

    unsigned char flags() const { return _flags; }

    bool isSampled() const
    {
        return _flags & static_cast<unsigned char>(Flag::kSampled);
    }

    bool isDebug() const
    {
        return _flags & static_cast<unsigned char>(Flag::kDebug);
    }

    bool isDebugIDContainerOnly() const
    {
        return !_traceID.isValid() && !_debugID.empty();
    }

    bool isValid() const { return _traceID.isValid() && _spanID != 0; }

    template <typename Stream>
    void print(Stream& out) const
    {
        _traceID.print(out);
        out << ':' << std::hex << _spanID << ':' << std::hex << _parentID << ':'
            << std::hex << static_cast<size_t>(_flags);
    }

    void ForeachBaggageItem(
        std::function<bool(const std::string& key, const std::string& value)> f)
        const override
    {
        forEachBaggageItem(f);
    }

    bool operator==(const SpanContext& rhs) const
    {
        return _traceID == rhs._traceID && _spanID == rhs._spanID &&
               _parentID == rhs._parentID && _flags == rhs._flags &&
               _baggage == rhs._baggage && _debugID == rhs._debugID;
    }

  private:
    TraceID _traceID;
    uint64_t _spanID;
    uint64_t _parentID;
    unsigned char _flags;
    StrMap _baggage;
    std::string _debugID;
};

}  // namespace jaegertracing

inline std::ostream& operator<<(std::ostream& out,
                                const jaegertracing::SpanContext& spanContext)
{
    spanContext.print(out);
    return out;
}

inline std::istream& operator>>(std::istream& in,
                                jaegertracing::SpanContext& spanContext)
{
    spanContext = jaegertracing::SpanContext::fromStream(in);
    return in;
}

#endif  // JAEGERTRACING_SPANCONTEXT_H
