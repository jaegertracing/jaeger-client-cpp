#ifndef JAEGERTRACING_B3TRACER_H
#define JAEGERTRACING_B3TRACER_H

namespace jaegertracing {
namespace propagation {

template<typename ReaderT, typename WriterT>
class Propagator;

class HeadersConfig;

template <typename ReaderType, typename WriterType>
class B3Tracer
{
  public:
    using Reader = ReaderType;
    using Writer = WriterType;

    B3Tracer() = default;
    ~B3Tracer() = default;

    SpanContext extractImpl(const Propagator<ReaderType, WriterType>* p, const Reader& reader) const;
    void inject(const Propagator<ReaderType, WriterType>* p, const SpanContext& ctx, const Writer& writer) const;
};
}
}
#include "B3Tracer.cpp"
#endif
