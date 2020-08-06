//
// Created by Fengjin Chen on 2020/7/28.
//

#ifndef JAEGERTRACING_ZIPKINPROPAGATOR_H
#define JAEGERTRACING_ZIPKINPROPAGATOR_H

#include "Propagator.h"

using namespace std;
namespace jaegertracing {
namespace propagation {

class HTTPKeyCarrier : public opentracing::HTTPHeadersReader,
                       public opentracing::HTTPHeadersWriter {
  public:
    HTTPKeyCarrier(map<string, string>* carrier) { _carrier = carrier; }
    opentracing::expected<void>
    Set(opentracing::string_view key,
        opentracing::string_view value) const override
    {
        _carrier->insert(pair<string, string>(key, value));
        return {};
    }
    opentracing::expected<opentracing::string_view>
    LookupKey(opentracing::string_view key) const override
    {
        return opentracing::expected<opentracing::string_view>(
            _carrier->at(key));
    }
    opentracing::expected<void> ForeachKey(
        std::function<opentracing::expected<void>(
            opentracing::string_view key, opentracing::string_view value)> f)
        const override
    {
        map<string, string>::iterator iter;

        for (iter = _carrier->begin(); iter != _carrier->end(); iter++) {
            f(iter->first, iter->second);
        }
        return opentracing::make_expected();
    };

  private:
    std::map<string, string>* _carrier;
};

class ZipkinPropagator : public HTTPHeaderPropagator {
  public:
    using Reader = opentracing::HTTPHeadersReader;
    using Writer = opentracing::HTTPHeadersWriter;
    using StrMap = SpanContext::StrMap;

    ZipkinPropagator() = default;

    ZipkinPropagator(const HeadersConfig& headerKeys,
                     const std::shared_ptr<metrics::Metrics>& metrics)
        : HTTPHeaderPropagator(headerKeys, metrics)
    {
    }

    ~ZipkinPropagator() = default;

    SpanContext extract(const Reader& reader) const override
    {
        SpanContext ctx = HTTPHeaderPropagator::extract(reader);
        // The core concern of this piece of code is that how to deal with
        // requests that contain both jaeger headers and zipkin b3 headers In
        // this case, we believe in zipkin b3 headers as jaeger
        // HTTPHeaderPropagator::extract return an completely new
        // jaegertracing::SpanContex even no propogation  header

        uint64_t zipkinTraceIdLow = 0, zipkinTraceIdHigh = 0;
        bool zipkinTraceSampled = false;
        uint64_t zipkinParentSpanId = 0, zipkinSpanId = 0;

        const auto result = reader.ForeachKey([this,
                                               &zipkinSpanId,
                                               &zipkinParentSpanId,
                                               &zipkinTraceIdLow,
                                               &zipkinTraceIdHigh,
                                               &zipkinTraceSampled](
                                                  const opentracing::
                                                      string_view& rawKey,
                                                  const opentracing::
                                                      string_view&
                                                          value) mutable {
            const auto key = normalizeKey(rawKey);
            std::istringstream in(value.data());
            if (key == _headerKeys.zipkinSpanIdHeaderName()) {
                in >> std::hex >> zipkinSpanId;
            }
            else if (key == _headerKeys.zipkinParentSPanIdHeaderName()) {
                in >> std::hex >> zipkinParentSpanId;
            }
            else if (key == _headerKeys.zipkinTraceSampledHeaderName()) {
                zipkinTraceSampled = value == "1" or value == "true";
            }
            else if (key == _headerKeys.zipkinTraceIdHeaderName()) {
                in >> std::hex >>
                    zipkinTraceIdLow;  // for 128 bit traceID, see
                                       // https://github.com/openzipkin/b3-propagation/blob/master/STATUS.md
                in >> std::hex >> zipkinTraceIdHigh;  // fsing istream to deal
                                                      // with 128bit traceID
            }
            return opentracing::make_expected();
        });
        jaegertracing::SpanContext::StrMap map;
        unsigned char flags = (0);
        if (zipkinTraceSampled) {
            flags |= static_cast<uint64_t>(SpanContext::Flag::kSampled);
        }
        if (not zipkinSpanId or not zipkinTraceSampled) {
            return ctx;
        }

        if (not zipkinParentSpanId) {
            zipkinParentSpanId = zipkinSpanId;
            ;
        }
        TraceID traceID(zipkinTraceIdHigh, zipkinTraceIdLow);
        if (not traceID.isValid()) {
            return ctx;
        }
        if (zipkinSpanId && zipkinParentSpanId) {
            return SpanContext(traceID,
                               zipkinSpanId,
                               zipkinParentSpanId,
                               flags,
                               ctx.baggage(),
                               "");
        }
        return ctx;
    }

    void inject(const SpanContext& ctx, const Writer& writer) const override
    {
        std::stringstream out;
        out << std::hex << ctx.traceID().high() << std::hex
            << ctx.traceID().low();

        HTTPHeaderPropagator::inject(ctx, writer);
        writer.Set(HTTPHeaderPropagator::_headerKeys.zipkinSpanIdHeaderName(),
                   std::to_string(ctx.spanID()));
        writer.Set(
            HTTPHeaderPropagator::_headerKeys.zipkinParentSPanIdHeaderName(),
            std::to_string(ctx.parentID()));
        writer.Set(HTTPHeaderPropagator::_headerKeys.zipkinTraceIdHeaderName(),
                   out.str());
        writer.Set(
            HTTPHeaderPropagator::_headerKeys.zipkinTraceSampledHeaderName(),
            std::to_string(ctx.isSampled()));
    }
};
}  // namespace propagation
}  // namespace jaegertracing

#endif  // JAEGERTRACING_ZIPKINPROPAGATOR_H
