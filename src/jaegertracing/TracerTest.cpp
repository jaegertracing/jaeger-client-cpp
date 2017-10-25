/*
 * Copyright (c) 2017 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sstream>

#include <gtest/gtest.h>

#include "jaegertracing/Tracer.h"
#include "jaegertracing/testutils/TracerUtil.h"

namespace jaegertracing {
namespace {

class FakeSpanContext : public opentracing::SpanContext {
    void ForeachBaggageItem(
      std::function<bool(const std::string&,
                         const std::string&)> /* unused */) const override
    {
        // Do nothing
    }
};

}  // anonymous namespace

TEST(Tracer, testTracer)
{
    auto mockAgent = testutils::TracerUtil::installGlobalTracer();
    auto tracer = std::static_pointer_cast<Tracer>(
        opentracing::Tracer::Global());

    opentracing::StartSpanOptions options;
    options.tags.push_back({ "tag-key", 1.23 });

    const FakeSpanContext fakeCtx;
    options.references.emplace_back(
        opentracing::SpanReferenceType::ChildOfRef, &fakeCtx);
    const SpanContext emptyCtx(TraceID(), 0, 0, 0, SpanContext::StrMap());
    options.references.emplace_back(
        opentracing::SpanReferenceType::ChildOfRef, &emptyCtx);
    const SpanContext parentCtx(
        TraceID(0xDEAD, 0xBEEF), 0xBEEF, 1234, 0, SpanContext::StrMap());
    options.references.emplace_back(
        opentracing::SpanReferenceType::ChildOfRef, &parentCtx);
    options.references.emplace_back(
        opentracing::SpanReferenceType::FollowsFromRef, &parentCtx);
    const SpanContext debugCtx(TraceID(),
                               0,
                               0,
                               static_cast<unsigned char>(
                                   SpanContext::Flag::kSampled) |
                               static_cast<unsigned char>(
                                   SpanContext::Flag::kDebug),
                               SpanContext::StrMap({
                                   {"debug-baggage-key", "debug-baggage-value"}
                               }),
                               "123");
    options.references.emplace_back(
        opentracing::SpanReferenceType::ChildOfRef, &debugCtx);

    std::unique_ptr<Span> span(static_cast<Span*>(
        tracer->StartSpanWithOptions("test-operation", options).release()));
    ASSERT_TRUE(static_cast<bool>(span));
    ASSERT_EQ(static_cast<opentracing::Tracer*>(tracer.get()), &span->tracer());

    span->SetOperationName("test-set-operation");
    span->SetTag("tag-key", "tag-value");
    span->SetBaggageItem("test-baggage-item-key", "test-baggage-item-value");
    ASSERT_EQ("test-baggage-item-value",
              span->BaggageItem("test-baggage-item-key"));
    span->Log({ { "log-bool", true } });
    span->Finish();
    ASSERT_GE(Span::Clock::now(), span->startTime() + span->duration());
    span->SetOperationName("test-set-operation-after-finish");
    ASSERT_EQ("test-set-operation", span->operationName());
    span->SetTag("tagged-after-finish-key", "tagged-after-finish-value");

    span.reset(static_cast<Span*>(
        tracer->StartSpanWithOptions(
            "test-span-with-default-options", {}).release()));

    options.references.clear();
    options.references.emplace_back(
        opentracing::SpanReferenceType::FollowsFromRef, &parentCtx);
    span.reset(static_cast<Span*>(
        tracer->StartSpanWithOptions(
            "test-span-with-default-options", options).release()));

    options.references.clear();
    options.references.emplace_back(opentracing::SpanReferenceType::ChildOfRef,
                                    &debugCtx);
    span.reset(static_cast<Span*>(
        tracer->StartSpanWithOptions(
            "test-span-with-debug-parent", options).release()));

    span.reset();

    opentracing::Tracer::InitGlobal(opentracing::MakeNoopTracer());
}

TEST(Tracer, testConstructorFailure)
{
    Config config;
    ASSERT_THROW(Tracer::make("", config), std::invalid_argument);
}

TEST(Tracer, testDisabledConfig)
{
    Config config(true,
                  samplers::Config(),
                  reporters::Config(),
                  propagation::HeadersConfig(),
                  baggage::RestrictionsConfig());
    ASSERT_FALSE(
        static_cast<bool>(
            std::dynamic_pointer_cast<Tracer>(
                Tracer::make("test-service", config))));
}

}  // namespace jaegertracing
