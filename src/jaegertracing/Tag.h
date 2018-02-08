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

#ifndef JAEGERTRACING_TAG_H
#define JAEGERTRACING_TAG_H

#include "jaegertracing/thrift-gen/jaeger_types.h"
#include <algorithm>
#include <cstdint>
#include <opentracing/string_view.h>
#include <opentracing/value.h>
#include <opentracing/variant/variant.hpp>
#include <string>

namespace jaegertracing {

class Tag {
  public:
    using ValueType = opentracing::Value;

    template <typename ValueArg>
    Tag(const std::string& key, ValueArg&& value)
        : _key(key)
        , _value(std::forward<ValueArg>(value))
    {
    }

    template <typename ValueArg>
    Tag(const std::pair<std::string,ValueArg> & tag_pair)
        : _key(tag_pair.first)
        , _value(tag_pair.second)
    {
    }

    bool operator==(const Tag& rhs) const
    {
        return _key == rhs._key && _value == rhs._value;
    }

    const std::string& key() const { return _key; }

    const ValueType& value() const { return _value; }

    thrift::Tag thrift() const
    {
        thrift::Tag tag;
        tag.__set_key(_key);
        ThriftVisitor visitor(tag);
        opentracing::util::apply_visitor(visitor, _value);
        return tag;
    }

  private:
    class ThriftVisitor {
      public:
        using result_type = void;

        explicit ThriftVisitor(thrift::Tag& tag)
            : _tag(tag)
        {
        }

        void operator()(const std::string& value) const { setString(value); }

        void operator()(const char* value) const { setString(value); }

        void operator()(double value) const
        {
            _tag.__set_vType(thrift::TagType::DOUBLE);
            _tag.__set_vDouble(value);
        }

        void operator()(bool value) const
        {
            _tag.__set_vType(thrift::TagType::BOOL);
            _tag.__set_vBool(value);
        }

        void operator()(int64_t value) const { setLong(value); }

        void operator()(uint64_t value) const { setLong(value); }

        template <typename Arg>
        void operator()(Arg&& value) const
        {
            // No-op
        }

      private:
        void setString(opentracing::string_view value) const
        {
            _tag.__set_vType(thrift::TagType::STRING);
            _tag.__set_vStr(value);
        }

        void setLong(int64_t value) const
        {
            _tag.__set_vType(thrift::TagType::LONG);
            _tag.__set_vLong(value);
        }

        thrift::Tag& _tag;
    };

    std::string _key;
    ValueType _value;
};

}  // namespace jaegertracing

#endif  // JAEGERTRACING_TAG_H
