//
// Created by Fengjin Chen on 2020/7/28.
//

#ifndef JAEGERTRACING_ZIPKINPROPAGATOR_H
#define JAEGERTRACING_ZIPKINPROPAGATOR_H

#include "Propagator.h"

namespace jaegertracing {
namespace propagation {

class ZipkinPropagator : public HTTPHeaderPropagator {
  public:
    using Reader = opentracing::HTTPHeadersReader;
    using Writer = opentracing::HTTPHeadersWriter;
    using StrMap = SpanContext::StrMap;
    ZipkinPropagator(const HeadersConfig& headerKeys,
                     const std::shared_ptr<metrics::Metrics>& metrics)
        : HTTPHeaderPropagator(headerKeys, metrics)
    {
    }
    ~ZipkinPropagator() = default;

    SpanContext extract(const Reader& reader) const override
    {
        SpanContext ctx = HTTPHeaderPropagator::extract(reader);
        const auto result =
            reader.ForeachKey([this](const opentracing::string_view& rawKey,
                                     const opentracing::string_view& value) {
                const auto key = normalizeKey(rawKey);
                if (key == _headerKeys.zipkinSpanIdHeaderName()) {

                }
                else if (key == _headerKeys.zipkinParentSPanIdHeaderName()) {
                }
                else if (key == _headerKeys.zipkinTraceSampledHeaderName()) {
                }
                return opentracing::make_expected();
            });
        return ctx;
    }

    void inject(const SpanContext& ctx, const Writer& writer) const override
    {
        // TODO fix 128 bit trace id problem
        HTTPHeaderPropagator::inject(ctx, writer);
        writer.Set(HTTPHeaderPropagator::_headerKeys.zipkinSpanIdHeaderName(),
                   std::to_string(ctx.spanID()));
        writer.Set(
            HTTPHeaderPropagator::_headerKeys.zipkinParentSPanIdHeaderName(),
            std::to_string(ctx.parentID()));
        writer.Set(HTTPHeaderPropagator::_headerKeys.zipkinTraceIdHeaderName(),
                   std::to_string(ctx.traceID().low()));
        writer.Set(
            HTTPHeaderPropagator::_headerKeys.zipkinTraceSampledHeaderName(),
            std::to_string(ctx.isSampled()));
    }
};

}  // namespace propagation
}  // namespace jaegertracing

#endif  // JAEGERTRACING_ZIPKINPROPAGATOR_H
