#ifndef JAEGER_SMARTPTR_H
#define JAEGER_SMARTPTR_H

#if defined(JAEGER_USE_BOOST_SMART_PTR)
#include <boost/smart_ptr.hpp>
#else
#include <memory>
#endif

namespace jaegertracing { namespace stdcxx {

/* Borrowed from Apache Thrift */
#if defined(JAEGER_USE_BOOST_SMART_PTR)

  using ::boost::const_pointer_cast;
  using ::boost::dynamic_pointer_cast;
  using ::boost::enable_shared_from_this;
  using ::boost::make_shared;
  using ::boost::scoped_ptr;
  using ::boost::shared_ptr;
  using ::boost::static_pointer_cast;
  using ::boost::weak_ptr;

#else

  using ::std::const_pointer_cast;
  using ::std::dynamic_pointer_cast;
  using ::std::enable_shared_from_this;
  using ::std::make_shared;
  template <typename T> using scoped_ptr = std::unique_ptr<T>;
  using ::std::shared_ptr;
  using ::std::static_pointer_cast;
  using ::std::weak_ptr;

#endif

}}

#endif
