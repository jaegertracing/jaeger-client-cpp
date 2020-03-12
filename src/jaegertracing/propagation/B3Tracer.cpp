#include "jaegertracing/propagation/Propagator.h"
#include "jaegertracing/propagation/B3Tracer.h"

namespace jaegertracing {
namespace propagation {

#if 0
void B3Tracer::set(Propagator* p) const
{
    _p = p;
}
#endif
template <typename ReaderType, typename WriterType>
SpanContext B3Tracer<ReaderType, WriterType>::extractImpl(const Propagator<ReaderType, WriterType>* p, const Reader& reader) const
{
    using StrMap = SpanContext::StrMap;
    std::cout << "B3 Extraction" << std::endl;
    SpanContext ctx;
    StrMap baggage;
    std::string debugID;
    std::string b3TraceId;
    std::string b3SpanId;
    std::string b3ParentSpanId;
    std::string b3Sampled;

    const auto result = reader.ForeachKey(
            [&debugID, &baggage, &b3TraceId, &b3SpanId, &b3ParentSpanId,
            &b3Sampled, &p](const std::string& rawKey, const std::string& value) {
            const auto key = p->normalizeKey(rawKey);
            if (key == p->_headerKeys.jaegerDebugHeader()) {
            debugID = value;
            }
            else if (key == p->_headerKeys.jaegerBaggageHeader()) {
            for (auto&& pair : p->parseCommaSeparatedMap(value)) {
            baggage[pair.first] = pair.second;
            }
            }
            else if (key == kB3TraceIdHeaderName) {
            b3TraceId = p->decodeValue(value);
            }
            else if (key == kB3SpanIdHeaderName) {
            b3SpanId = p->decodeValue(value);
            }
            else if (key == kB3ParentSpanIdHeaderName) {
            b3ParentSpanId = p->decodeValue(value);
            }
            else if (key == kB3SampledHeaderName) {
                b3Sampled = p->decodeValue(value);
            }
            else {
                const auto prefix = p->_headerKeys.traceBaggageHeaderPrefix();
                if (key.size() >= prefix.size() &&
                        key.substr(0, prefix.size()) == prefix) {
                    const auto safeKey = p->removeBaggageKeyPrefix(key);
                    const auto safeValue = p->decodeValue(value);
                    baggage[safeKey] = safeValue;
                }
            }
            return opentracing::make_expected();
            });

    if (!b3TraceId.empty() && !b3SpanId.empty() && !b3ParentSpanId.empty() &&
            !b3Sampled.empty()) {
        std::istringstream issB3(b3TraceId+":"+b3SpanId+":"+b3ParentSpanId+":"+b3Sampled);
        if (!(issB3 >> ctx) || ctx == SpanContext())
            return SpanContext();
    }

    if (!result &&
            result.error() == opentracing::span_context_corrupted_error) {
        p->_metrics->decodingErrors().inc(1);
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
            debugID,
            TracerType::TRACER_TYPE_B3);
}

template <typename ReaderType, typename WriterType>
void B3Tracer<ReaderType, WriterType>::inject(const Propagator<ReaderType, WriterType>* p, const SpanContext& ctx, const Writer& writer) const
{
    std::ostringstream ss;
    ss << ctx.traceID();
    writer.Set(kB3TraceIdHeaderName, ss.str());
    ss.str(std::string());
    ss.clear();
    ss << std::hex << ctx.spanID();
    writer.Set(kB3SpanIdHeaderName, ss.str());
    ss.str(std::string());
    ss.clear();
    ss << std::hex << ctx.parentID();
    writer.Set(kB3ParentSpanIdHeaderName, ss.str());
    writer.Set(kB3SampledHeaderName, std::to_string(ctx.isSampled()));
}


}
}

