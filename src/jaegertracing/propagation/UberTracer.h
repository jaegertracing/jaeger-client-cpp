#ifndef JAEGERTRACING_UBERTRACER_H
#define JAEGERTRACING_UBERTRACER_H

namespace jaegertracing {
namespace propagation {

template<typename ReaderT, typename WriterT>
class Propagator;

class HeadersConfig;

template <typename ReaderType, typename WriterType>
class UberTracer
{
  public:

    using Reader = ReaderType;
    using Writer = WriterType;

    UberTracer() = default;
    ~UberTracer() = default;
    //void set(Propagator* p) const;
 
    SpanContext  extractImpl(const Propagator<ReaderType, WriterType>* p, const Reader& reader) const;
    void inject(const Propagator<ReaderType, WriterType>* p, const SpanContext& ctx, const Writer& writer) const;
};

}
}
#include "UberTracer.cpp"

#endif
