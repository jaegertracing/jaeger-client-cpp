#include "jaegertracing/propagation/Propagator.h"
#include "jaegertracing/propagation/UberTracer.h"

namespace jaegertracing {
namespace propagation {
#if 0
void UberTracer::set(Propagator* p) const
{
    _p = p;
}
#endif

template <typename ReaderType, typename WriterType>
SpanContext UberTracer<ReaderType, WriterType>::extractImpl(const Propagator<ReaderType, WriterType>* p, const Reader& reader) const
{
    using StrMap = SpanContext::StrMap;
    std::cout << "UBER Extraction" << std::endl;
    SpanContext ctx;
    StrMap baggage;
    std::string debugID;

    const auto result = reader.ForeachKey(
            [&ctx, &debugID, &baggage,
            &p](const std::string& rawKey, const std::string& value) {
            const auto key = p->normalizeKey(rawKey);
            if (key == p->_headerKeys.traceContextHeaderName()) {
            const auto safeValue = p->decodeValue(value);
            std::istringstream iss(safeValue);
            if (!(iss >> ctx) || ctx == SpanContext()) {
            return opentracing::make_expected_from_error<void>(
                opentracing::span_context_corrupted_error);
            }
            }
            else if (key == p->_headerKeys.jaegerDebugHeader()) {
            debugID = value;
            }
            else if (key == p->_headerKeys.jaegerBaggageHeader()) {
            for (auto&& pair : p->parseCommaSeparatedMap(value)) {
            baggage[pair.first] = pair.second;
            std::cout<<"1-ADD:"<<pair.first<<"::"<<pair.second<<std::endl;
            }
            }
            else {
                const auto prefix = p->_headerKeys.traceBaggageHeaderPrefix();
                if (key.size() >= prefix.size() &&
                        key.substr(0, prefix.size()) == prefix) {
                    const auto safeKey = p->removeBaggageKeyPrefix(key);
                    const auto safeValue = p->decodeValue(value);
                    baggage[safeKey] = safeValue;
                    std::cout<<"2-ADD:"<<safeKey<<"::"<<safeValue<<std::endl;
                }
            }
            return opentracing::make_expected();
            });

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
            TracerType::TRACER_TYPE_UBER);

}

template <typename ReaderType, typename WriterType>
void UberTracer<ReaderType, WriterType>::inject(const Propagator<ReaderType, WriterType>* p, const SpanContext& ctx, const Writer& writer) const
{
    std::ostringstream oss;
    oss << ctx;
    writer.Set(p->_headerKeys.traceContextHeaderName(), oss.str());
}



}
}
