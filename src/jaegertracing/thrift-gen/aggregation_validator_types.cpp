/**
 * Autogenerated by Thrift Compiler (0.11.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "aggregation_validator_types.h"

#include <algorithm>
#include <ostream>

#include <thrift/TToString.h>

namespace jaegertracing { namespace thrift {


ValidateTraceResponse::~ValidateTraceResponse() throw() {
}


void ValidateTraceResponse::__set_ok(const bool val) {
  this->ok = val;
}

void ValidateTraceResponse::__set_traceCount(const int64_t val) {
  this->traceCount = val;
}
std::ostream& operator<<(std::ostream& out, const ValidateTraceResponse& obj)
{
  obj.printTo(out);
  return out;
}


uint32_t ValidateTraceResponse::read(::apache::thrift::protocol::TProtocol* iprot) {

  ::apache::thrift::protocol::TInputRecursionTracker tracker(*iprot);
  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;

  bool isset_ok = false;
  bool isset_traceCount = false;

  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_BOOL) {
          xfer += iprot->readBool(this->ok);
          isset_ok = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_I64) {
          xfer += iprot->readI64(this->traceCount);
          isset_traceCount = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  if (!isset_ok)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  if (!isset_traceCount)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t ValidateTraceResponse::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  ::apache::thrift::protocol::TOutputRecursionTracker tracker(*oprot);
  xfer += oprot->writeStructBegin("ValidateTraceResponse");

  xfer += oprot->writeFieldBegin("ok", ::apache::thrift::protocol::T_BOOL, 1);
  xfer += oprot->writeBool(this->ok);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("traceCount", ::apache::thrift::protocol::T_I64, 2);
  xfer += oprot->writeI64(this->traceCount);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(ValidateTraceResponse &a, ValidateTraceResponse &b) {
  using ::std::swap;
  swap(a.ok, b.ok);
  swap(a.traceCount, b.traceCount);
}

ValidateTraceResponse::ValidateTraceResponse(const ValidateTraceResponse& other0) {
  ok = other0.ok;
  traceCount = other0.traceCount;
}
ValidateTraceResponse& ValidateTraceResponse::operator=(const ValidateTraceResponse& other1) {
  ok = other1.ok;
  traceCount = other1.traceCount;
  return *this;
}
void ValidateTraceResponse::printTo(std::ostream& out) const {
  using ::apache::thrift::to_string;
  out << "ValidateTraceResponse(";
  out << "ok=" << to_string(ok);
  out << ", " << "traceCount=" << to_string(traceCount);
  out << ")";
}

}} // namespace
