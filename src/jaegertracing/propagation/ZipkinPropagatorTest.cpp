//
// Created by Fengjin Chen on 2020/7/28.
//

#include "ZipkinPropagator.h"
#include "jaegertracing/SpanContext.h"
#include "jaegertracing/TraceID.h"
#include "jaegertracing/propagation/Propagator.h"
#include <gtest/gtest.h>
#include <iosfwd>
#include <map>
#include <random>
#include <stddef.h>
#include <string>
#include <unordered_map>
using namespace std;
namespace jaegertracing {
namespace propagation {
namespace {
using StrMap = map<string, string>;

template <typename RandomGenerator>
std::string randomStr(RandomGenerator& gen)
{
    constexpr auto kMaxSize = 10;
    static constexpr char kLetters[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";
    constexpr auto kNumLetters = sizeof(kLetters) - 1;
    const auto size = gen() % kMaxSize;
    std::string result(size, '\0');
    for (auto i = static_cast<size_t>(0); i < size; ++i) {
        const auto pos = gen() % kNumLetters;
        result[i] = kLetters[pos];
    }
    return result;
}

}  // anonymous namespace

TEST(ZipKinHTTPPropogation, testZipkinHTTPPropagationInject)
{
    StrMap headers;
    HTTPKeyCarrier carrier(&headers);
    ZipkinPropagator propogator;
    std::random_device randomDevice;
    std::default_random_engine engine(randomDevice());
    constexpr auto kMaxNumBaggageItems = 10;
    const auto numBaggageItems = engine() % (kMaxNumBaggageItems - 1) + 1;
    SpanContext::StrMap baggage;
    for (auto i = static_cast<size_t>(0); i < numBaggageItems; ++i) {
        const auto key = randomStr(engine);
        const auto value = randomStr(engine);
        baggage[key] = value;
    }
    SpanContext ctx(TraceID(0xaaaa, 0xbbbb), 0xaccbdf8765432199, 0xdddd, 1, baggage);
    propogator.inject(ctx, carrier);
    ASSERT_EQ(headers.at(kzipkinParentSpanIdHeaderName), "000000000000dddd");
    ASSERT_EQ(headers.at(kzipkinSpanIdHeaderName), "accbdf8765432199");
    ASSERT_EQ(headers.at(kzipkinTraceSampledHeaderName), "1");
    ASSERT_EQ(headers.at(kzipkinTraceIdHeaderName), "000000000000aaaa000000000000bbbb");
}

TEST(ZipKinHTTPPropogation, testZipkinHTTPPropagationExact)
{
    StrMap m;
    m.insert(std::pair<string, string>(kzipkinSpanIdHeaderName, "12345a"));
    m.insert(std::pair<string, string>(kzipkinTraceIdHeaderName, "23456b"));

    m.insert(
        std::pair<string, string>(kzipkinParentSpanIdHeaderName, "34567c"));
    m.insert(std::pair<string, string>(kzipkinTraceSampledHeaderName, "1"));
    HTTPKeyCarrier carrier(&m);
    ZipkinPropagator propogator;
    std::random_device randomDevice;
    std::default_random_engine engine(randomDevice());
    constexpr auto kMaxNumBaggageItems = 10;
    const auto numBaggageItems = engine() % (kMaxNumBaggageItems - 1) + 1;
    SpanContext::StrMap baggage;
    for (auto i = static_cast<size_t>(0); i < numBaggageItems; ++i) {
        const auto key = randomStr(engine);
        const auto value = randomStr(engine);
        baggage[key] = value;
    }
    auto ctx = propogator.extract(carrier);

    ostringstream out;

    out << std::hex << ctx.spanID();
    ASSERT_EQ(out.str(), m[kzipkinSpanIdHeaderName]);
    out.str("");

    out << std::hex << ctx.parentID();
    ASSERT_EQ(out.str(), m[kzipkinParentSpanIdHeaderName]);
    out.str("");

    out << std::hex << ctx.traceID();
    ASSERT_EQ(out.str(), m[kzipkinTraceIdHeaderName]);
    out.str("");
}
}  // namespace propagation
}  // namespace jaegertracing
