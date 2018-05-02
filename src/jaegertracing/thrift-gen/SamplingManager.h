/**
 * Autogenerated by Thrift Compiler (0.11.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef SamplingManager_H
#define SamplingManager_H

#include <thrift/TDispatchProcessor.h>
#include <thrift/async/TConcurrentClientSyncInfo.h>
#include "sampling_types.h"

namespace jaegertracing { namespace sampling_manager { namespace thrift {

#ifdef _MSC_VER
  #pragma warning( push )
  #pragma warning (disable : 4250 ) //inheriting methods via dominance 
#endif

class SamplingManagerIf {
 public:
  virtual ~SamplingManagerIf() {}
  virtual void getSamplingStrategy(SamplingStrategyResponse& _return, const std::string& serviceName) = 0;
};

class SamplingManagerIfFactory {
 public:
  typedef SamplingManagerIf Handler;

  virtual ~SamplingManagerIfFactory() {}

  virtual SamplingManagerIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(SamplingManagerIf* /* handler */) = 0;
};

class SamplingManagerIfSingletonFactory : virtual public SamplingManagerIfFactory {
 public:
  SamplingManagerIfSingletonFactory(const ::apache::thrift::stdcxx::shared_ptr<SamplingManagerIf>& iface) : iface_(iface) {}
  virtual ~SamplingManagerIfSingletonFactory() {}

  virtual SamplingManagerIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(SamplingManagerIf* /* handler */) {}

 protected:
  ::apache::thrift::stdcxx::shared_ptr<SamplingManagerIf> iface_;
};

class SamplingManagerNull : virtual public SamplingManagerIf {
 public:
  virtual ~SamplingManagerNull() {}
  void getSamplingStrategy(SamplingStrategyResponse& /* _return */, const std::string& /* serviceName */) {
    return;
  }
};

typedef struct _SamplingManager_getSamplingStrategy_args__isset {
  _SamplingManager_getSamplingStrategy_args__isset() : serviceName(false) {}
  bool serviceName :1;
} _SamplingManager_getSamplingStrategy_args__isset;

class SamplingManager_getSamplingStrategy_args {
 public:

  SamplingManager_getSamplingStrategy_args(const SamplingManager_getSamplingStrategy_args&);
  SamplingManager_getSamplingStrategy_args& operator=(const SamplingManager_getSamplingStrategy_args&);
  SamplingManager_getSamplingStrategy_args() : serviceName() {
  }

  virtual ~SamplingManager_getSamplingStrategy_args() throw();
  std::string serviceName;

  _SamplingManager_getSamplingStrategy_args__isset __isset;

  void __set_serviceName(const std::string& val);

  bool operator == (const SamplingManager_getSamplingStrategy_args & rhs) const
  {
    if (!(serviceName == rhs.serviceName))
      return false;
    return true;
  }
  bool operator != (const SamplingManager_getSamplingStrategy_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SamplingManager_getSamplingStrategy_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SamplingManager_getSamplingStrategy_pargs {
 public:


  virtual ~SamplingManager_getSamplingStrategy_pargs() throw();
  const std::string* serviceName;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SamplingManager_getSamplingStrategy_result__isset {
  _SamplingManager_getSamplingStrategy_result__isset() : success(false) {}
  bool success :1;
} _SamplingManager_getSamplingStrategy_result__isset;

class SamplingManager_getSamplingStrategy_result {
 public:

  SamplingManager_getSamplingStrategy_result(const SamplingManager_getSamplingStrategy_result&);
  SamplingManager_getSamplingStrategy_result& operator=(const SamplingManager_getSamplingStrategy_result&);
  SamplingManager_getSamplingStrategy_result() {
  }

  virtual ~SamplingManager_getSamplingStrategy_result() throw();
  SamplingStrategyResponse success;

  _SamplingManager_getSamplingStrategy_result__isset __isset;

  void __set_success(const SamplingStrategyResponse& val);

  bool operator == (const SamplingManager_getSamplingStrategy_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const SamplingManager_getSamplingStrategy_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SamplingManager_getSamplingStrategy_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SamplingManager_getSamplingStrategy_presult__isset {
  _SamplingManager_getSamplingStrategy_presult__isset() : success(false) {}
  bool success :1;
} _SamplingManager_getSamplingStrategy_presult__isset;

class SamplingManager_getSamplingStrategy_presult {
 public:


  virtual ~SamplingManager_getSamplingStrategy_presult() throw();
  SamplingStrategyResponse* success;

  _SamplingManager_getSamplingStrategy_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class SamplingManagerClient : virtual public SamplingManagerIf {
 public:
  SamplingManagerClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  SamplingManagerClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void getSamplingStrategy(SamplingStrategyResponse& _return, const std::string& serviceName);
  void send_getSamplingStrategy(const std::string& serviceName);
  void recv_getSamplingStrategy(SamplingStrategyResponse& _return);
 protected:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class SamplingManagerProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  ::apache::thrift::stdcxx::shared_ptr<SamplingManagerIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (SamplingManagerProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_getSamplingStrategy(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  SamplingManagerProcessor(::apache::thrift::stdcxx::shared_ptr<SamplingManagerIf> iface) :
    iface_(iface) {
    processMap_["getSamplingStrategy"] = &SamplingManagerProcessor::process_getSamplingStrategy;
  }

  virtual ~SamplingManagerProcessor() {}
};

class SamplingManagerProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  SamplingManagerProcessorFactory(const ::apache::thrift::stdcxx::shared_ptr< SamplingManagerIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::apache::thrift::stdcxx::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::apache::thrift::stdcxx::shared_ptr< SamplingManagerIfFactory > handlerFactory_;
};

class SamplingManagerMultiface : virtual public SamplingManagerIf {
 public:
  SamplingManagerMultiface(std::vector<apache::thrift::stdcxx::shared_ptr<SamplingManagerIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~SamplingManagerMultiface() {}
 protected:
  std::vector<apache::thrift::stdcxx::shared_ptr<SamplingManagerIf> > ifaces_;
  SamplingManagerMultiface() {}
  void add(::apache::thrift::stdcxx::shared_ptr<SamplingManagerIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void getSamplingStrategy(SamplingStrategyResponse& _return, const std::string& serviceName) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->getSamplingStrategy(_return, serviceName);
    }
    ifaces_[i]->getSamplingStrategy(_return, serviceName);
    return;
  }

};

// The 'concurrent' client is a thread safe client that correctly handles
// out of order responses.  It is slower than the regular client, so should
// only be used when you need to share a connection among multiple threads
class SamplingManagerConcurrentClient : virtual public SamplingManagerIf {
 public:
  SamplingManagerConcurrentClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  SamplingManagerConcurrentClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void getSamplingStrategy(SamplingStrategyResponse& _return, const std::string& serviceName);
  int32_t send_getSamplingStrategy(const std::string& serviceName);
  void recv_getSamplingStrategy(SamplingStrategyResponse& _return, const int32_t seqid);
 protected:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
  ::apache::thrift::async::TConcurrentClientSyncInfo sync_;
};

#ifdef _MSC_VER
  #pragma warning( pop )
#endif

}}} // namespace

#endif
