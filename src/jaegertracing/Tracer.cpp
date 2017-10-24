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

#include "jaegertracing/Tracer.h"

#include "jaegertracing/Reference.h"

namespace jaegertracing {

using StrMap = SpanContext::StrMap;

std::unique_ptr<opentracing::Span>
Tracer::StartSpanWithOptions(string_view operationName,
                             const opentracing::StartSpanOptions& options) const
    noexcept
{
    try {
        std::vector<Reference> references;
        SpanContext parent;
        auto hasParent = false;
        for (auto&& ref : options.references) {
            auto ctx = dynamic_cast<const SpanContext*>(ref.second);
            if (!ctx) {
                _logger->error(
                    "Reference contains invalid type of SpanReference");
                continue;
            }
            if (!ctx->isValid() || ctx->isDebugIDContainerOnly()) {
                continue;
            }
            references.push_back(Reference(*ctx, ref.first));

            if (!hasParent) {
                parent = *ctx;
                hasParent =
                    (ref.first == opentracing::SpanReferenceType::ChildOfRef);
            }
        }

        if (!hasParent && parent.isValid()) {
            hasParent = true;
        }

        std::vector<Tag> samplerTags;
        auto newTrace = false;
        SpanContext ctx;
        if (!hasParent || !parent.isValid()) {
            newTrace = true;
            const TraceID traceID(randomID(), randomID());
            const auto spanID = traceID.low();
            const auto parentID = 0;
            auto flags = static_cast<unsigned char>(0);
            if (hasParent && parent.isDebugIDContainerOnly()) {
                flags |=
                    (static_cast<unsigned char>(SpanContext::Flag::kSampled) |
                     static_cast<unsigned char>(SpanContext::Flag::kDebug));
            }
            else {
                const auto samplingStatus =
                    _sampler->isSampled(traceID, operationName);
                if (samplingStatus.isSampled()) {
                    flags |=
                        static_cast<unsigned char>(SpanContext::Flag::kSampled);
                    samplerTags = samplingStatus.tags();
                }
            }
            ctx = SpanContext(traceID, spanID, parentID, flags, StrMap());
        }
        else {
            const auto traceID = parent.traceID();
            const auto spanID = randomID();
            const auto parentID = parent.spanID();
            const auto flags = parent.flags();
            ctx = SpanContext(traceID, spanID, parentID, flags, StrMap());
        }

        if (hasParent && !parent.baggage().empty()) {
            ctx = ctx.withBaggage(parent.baggage());
        }

        return startSpanInternal(ctx,
                                 operationName,
                                 options.start_steady_timestamp,
                                 samplerTags,
                                 options.tags,
                                 newTrace,
                                 references);
    } catch (...) {
        utils::ErrorUtil::logError(
            *_logger, "Error occurred in Tracer::StartSpanWithOptions");
        return nullptr;
    }
}

std::unique_ptr<Span>
Tracer::startSpanInternal(const SpanContext& context,
                          const std::string& operationName,
                          const Clock::time_point& startTime,
                          const std::vector<Tag>& internalTags,
                          const std::vector<OpenTracingTag>& tags,
                          bool newTrace,
                          const std::vector<Reference>& references) const
{
    const auto firstInProcess = (context.parentID() == 0);

    std::vector<Tag> spanTags;
    spanTags.reserve(tags.size() + internalTags.size());
    std::transform(
        std::begin(tags),
        std::end(tags),
        std::back_inserter(spanTags),
        [](const OpenTracingTag& tag) { return Tag(tag.first, tag.second); });
    spanTags.insert(
        std::end(spanTags), std::begin(internalTags), std::end(internalTags));

    std::unique_ptr<Span> span(new Span(shared_from_this(),
                                        context,
                                        operationName,
                                        startTime,
                                        spanTags,
                                        references));

    _metrics->spansStarted().inc(1);
    if (span->context().isSampled()) {
        _metrics->spansSampled().inc(1);
        if (newTrace) {
            _metrics->tracesStartedSampled().inc(1);
        }
        else if (firstInProcess) {
            _metrics->tracesJoinedSampled().inc(1);
        }
    }
    else {
        _metrics->spansNotSampled().inc(1);
        if (newTrace) {
            _metrics->tracesStartedNotSampled().inc(1);
        }
        else if (firstInProcess) {
            _metrics->tracesJoinedNotSampled().inc(1);
        }
    }

    return span;
}

}  // namespace jaegertracing
