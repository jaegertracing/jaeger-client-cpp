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

#ifndef JAEGERTRACING_LOGRECORD_H
#define JAEGERTRACING_LOGRECORD_H

#include "jaegertracing/Tag.h"
#include "jaegertracing/thrift-gen/jaeger_types.h"
#include <algorithm>
#include <chrono>
#include <iterator>
#include <type_traits>
#include <vector>

#include <opentracing/span.h>

namespace jaegertracing {

class LogRecord {
  public:
    using Clock = std::chrono::system_clock;

    LogRecord()
        : _timestamp(Clock::now())
    {
    }

    template <typename FieldIterator>
    LogRecord(const Clock::time_point& timestamp,
              FieldIterator first,
              FieldIterator last)
        : _timestamp(timestamp)
        , _fields(first, last)
    {
    }

    LogRecord(const opentracing::LogRecord & other)
        : _timestamp(other.timestamp),
          _fields(other.fields.begin(), other.fields.end())
    {
    }

    const Clock::time_point& timestamp() const { return _timestamp; }

    const std::vector<Tag>& fields() const { return _fields; }

    thrift::Log thrift() const
    {
        thrift::Log log;
        log.__set_timestamp(
            std::chrono::duration_cast<std::chrono::microseconds>(
                _timestamp.time_since_epoch())
                .count());

        std::vector<thrift::Tag> fields;
        fields.reserve(_fields.size());
        std::transform(std::begin(_fields),
                       std::end(_fields),
                       std::back_inserter(fields),
                       [](const Tag& tag) { return tag.thrift(); });
        log.__set_fields(fields);

        return log;
    }

  private:
    Clock::time_point _timestamp;
    std::vector<Tag> _fields;
};

}  // namespace jaegertracing

#endif  // JAEGERTRACING_LOGRECORD_H
