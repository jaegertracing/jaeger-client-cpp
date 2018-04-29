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

#ifndef JAEGERTRACING_PROPAGATION_PROPAGATOR_H
#define JAEGERTRACING_PROPAGATION_PROPAGATOR_H

#include "jaegertracing/SpanContext.h"
#include "jaegertracing/metrics/Metrics.h"
#include "jaegertracing/net/URI.h"
#include "jaegertracing/platform/Endian.h"
#include "jaegertracing/propagation/Extractor.h"
#include "jaegertracing/propagation/HeadersConfig.h"
#include "jaegertracing/propagation/Injector.h"
#include <cctype>
#include <climits>
#include <opentracing/propagation.h>
#include <sstream>

namespace jaegertracing {

class Tracer;

namespace propagation {

template <typename ReaderType, typename WriterType>
class Propagator : public Extractor<ReaderType>, public Injector<WriterType> {
  public:
    using Reader = ReaderType;
    using Writer = WriterType;
    using StrMap = SpanContext::StrMap;

    Propagator()
        : _headerKeys()
        , _metrics(metrics::Metrics::makeNullMetrics())
    {
    }

    Propagator(const HeadersConfig& headerKeys,
               const std::shared_ptr<metrics::Metrics>& metrics)
        : _headerKeys(headerKeys)
        , _metrics(metrics)
    {
    }

    virtual ~Propagator() = default;

    SpanContext extract(const Reader& reader) const override
    {
        SpanContext ctx;
        StrMap baggage;
        std::string debugID;
        const auto result = reader.ForeachKey(
            [this, &ctx, &debugID, &baggage](const std::string& rawKey,
                                             const std::string& value) {
                const auto key = normalizeKey(rawKey);
                if (key == _headerKeys.traceContextHeaderName()) {
                    const auto safeValue = decodeValue(value);
                    std::istringstream iss(safeValue);
                    if (!(iss >> ctx) || ctx == SpanContext()) {
                        return opentracing::make_expected_from_error<void>(
                            opentracing::span_context_corrupted_error);
                    }
                }
                else if (key == _headerKeys.jaegerDebugHeader()) {
                    debugID = value;
                }
                else if (key == _headerKeys.jaegerBaggageHeader()) {
                    for (auto&& pair : parseCommaSeparatedMap(value)) {
                        baggage[pair.first] = pair.second;
                    }
                }
                else {
                    const auto prefix = _headerKeys.traceBaggageHeaderPrefix();
                    if (key.size() >= prefix.size() &&
                        key.substr(0, prefix.size()) == prefix) {
                        const auto safeKey = removeBaggageKeyPrefix(key);
                        const auto safeValue = decodeValue(value);
                        baggage[safeKey] = safeValue;
                    }
                }
                return opentracing::make_expected();
            });

        if (!result &&
            result.error() == opentracing::span_context_corrupted_error) {
            _metrics->decodingErrors().inc(1);
            return SpanContext();
        }

        if (!ctx.traceID().isValid() && debugID.empty() && baggage.empty()) {
            return SpanContext();
        }

        int flags = ctx.flags();
        if (!debugID.empty()) {
            flags |= static_cast<unsigned char>(SpanContext::Flag::kDebug) |
                     static_cast<unsigned char>(SpanContext::Flag::kSampled);
        }
        return SpanContext(ctx.traceID(),
                           ctx.spanID(),
                           ctx.parentID(),
                           flags,
                           baggage,
                           debugID);
    }

    void inject(const SpanContext& ctx, const Writer& writer) const override
    {
        std::ostringstream oss;
        oss << ctx;
        writer.Set(_headerKeys.traceContextHeaderName(), oss.str());
        ctx.forEachBaggageItem(
            [this, &writer](const std::string& key, const std::string& value) {
                const auto safeKey = addBaggageKeyPrefix(key);
                const auto safeValue = encodeValue(value);
                writer.Set(safeKey, safeValue);
                return true;
            });
    }

  protected:
    virtual std::string encodeValue(const std::string& str) const
    {
        return str;
    }

    virtual std::string decodeValue(const std::string& str) const
    {
        return str;
    }

    virtual std::string normalizeKey(const std::string& rawKey) const
    {
        return rawKey;
    }

  private:
    static StrMap parseCommaSeparatedMap(const std::string& escapedValue)
    {
        StrMap map;
        std::istringstream iss(net::URI::queryUnescape(escapedValue));
        std::string piece;
        while (std::getline(iss, piece, ',')) {
            const auto eqPos = piece.find('=');
            if (eqPos != std::string::npos) {
                const auto key = piece.substr(0, eqPos);
                const auto value = piece.substr(eqPos + 1);
                map[key] = value;
            }
        }
        return map;
    }

    std::string addBaggageKeyPrefix(const std::string& key) const
    {
        return _headerKeys.traceBaggageHeaderPrefix() + key;
    }

    std::string removeBaggageKeyPrefix(const std::string& key) const
    {
        return key.substr(_headerKeys.traceBaggageHeaderPrefix().size());
    }

    HeadersConfig _headerKeys;
    std::shared_ptr<metrics::Metrics> _metrics;
};

using TextMapPropagator = Propagator<const opentracing::TextMapReader&,
                                     const opentracing::TextMapWriter&>;

class HTTPHeaderPropagator
    : public Propagator<const opentracing::HTTPHeadersReader&,
                        const opentracing::HTTPHeadersWriter&> {
  public:
    using Propagator<Reader, Writer>::Propagator;

  protected:
    std::string encodeValue(const std::string& str) const override
    {
        return net::URI::queryEscape(str);
    }

    std::string decodeValue(const std::string& str) const override
    {
        return net::URI::queryUnescape(str);
    }

    std::string normalizeKey(const std::string& rawKey) const override
    {
        std::string key;
        key.reserve(rawKey.size());
        std::transform(std::begin(rawKey),
                       std::end(rawKey),
                       std::back_inserter(key),
                       [](char ch) { return std::tolower(ch); });
        return key;
    }
};

class BinaryPropagator : public Extractor<std::istream&>,
                         public Injector<std::ostream&> {
  public:
    using StrMap = SpanContext::StrMap;

    explicit BinaryPropagator(const std::shared_ptr<metrics::Metrics>& metrics =
                                  std::shared_ptr<metrics::Metrics>())
        : _metrics(metrics == nullptr ? metrics::Metrics::makeNullMetrics()
                                      : metrics)
    {
    }

    void inject(const SpanContext& ctx, std::ostream& out) const override
    {
        writeBinary(out, ctx.traceID().high());
        writeBinary(out, ctx.traceID().low());
        writeBinary(out, ctx.spanID());
        writeBinary(out, ctx.parentID());
        // `flags` is a single byte, so endianness is not an issue.
        out.put(ctx.flags());

        writeBinary(out, static_cast<uint32_t>(ctx.baggage().size()));
        for (auto&& pair : ctx.baggage()) {
            auto&& key = pair.first;
            writeBinary(out, static_cast<uint32_t>(key.size()));
            out.write(key.c_str(), key.size());

            auto&& value = pair.second;
            writeBinary(out, static_cast<uint32_t>(value.size()));
            out.write(value.c_str(), value.size());
        }
    }

    SpanContext extract(std::istream& in) const override
    {
        const auto traceIDHigh = readBinary<uint64_t>(in);
        const auto traceIDLow = readBinary<uint64_t>(in);
        TraceID traceID(traceIDHigh, traceIDLow);
        const auto spanID = readBinary<uint64_t>(in);
        const auto parentID = readBinary<uint64_t>(in);

        auto ch = '\0';
        in.get(ch);
        const auto flags = static_cast<unsigned char>(ch);

        const auto numBaggageItems = readBinary<uint32_t>(in);
        StrMap baggage;
        baggage.reserve(numBaggageItems);
        for (auto i = static_cast<uint32_t>(0); i < numBaggageItems; ++i) {
            const auto keyLength = readBinary<uint32_t>(in);
            std::string key(keyLength, '\0');
            if (!in.read(&key[0], keyLength)) {
                _metrics->decodingErrors().inc(1);
                return SpanContext();
            }

            const auto valueLength = readBinary<uint32_t>(in);
            std::string value(valueLength, '\0');
            if (!in.read(&value[0], valueLength)) {
                _metrics->decodingErrors().inc(1);
                return SpanContext();
            }

            baggage[key] = value;
        }

        SpanContext ctx(traceID, spanID, parentID, flags, baggage);
        return ctx;
    }

  private:
    template <typename ValueType>
    static
        typename std::enable_if<std::is_integral<ValueType>::value, void>::type
        writeBinary(std::ostream& out, ValueType value)
    {
        const ValueType outValue = platform::endian::toBigEndian(value);
        for (auto i = static_cast<size_t>(0); i < sizeof(ValueType); ++i) {
            const auto numShiftBits = (sizeof(ValueType) - i - 1) * CHAR_BIT;
            const auto byte = outValue >> numShiftBits;
            out.put(static_cast<unsigned char>(byte));
        }
    }

    template <typename ValueType>
    static typename std::enable_if<std::is_integral<ValueType>::value,
                                   ValueType>::type
    readBinary(std::istream& in)
    {
        auto value = static_cast<ValueType>(0);
        auto ch = '\0';
        for (auto i = static_cast<size_t>(0);
             i < sizeof(ValueType) && in.get(ch);
             ++i) {
            const auto byte = static_cast<uint8_t>(ch);
            value <<= CHAR_BIT;
            value |= byte;
        }
        return platform::endian::fromBigEndian(value);
    }

  private:
    std::shared_ptr<metrics::Metrics> _metrics;
};

}  // namespace propagation
}  // namespace jaegertracing

#endif  // JAEGERTRACING_PROPAGATION_PROPAGATOR_H
