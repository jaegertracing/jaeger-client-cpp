#include "jaegertracing/Tracer.h"

#include "cdemo_tracing.h"

#include <iostream>
#include <sstream>

using std::string;

/*
 * You'll want to load your configuration from a file
 * or use the C++ programmatic configuration interfaces, but
 * this is a demo:
 */

const string cfgstr {R"endyaml(
disabled: false
sampler:
    type: const
    param: 1
reporter:
    queueSize: 100
    bufferFlushInterval: 2
    logSpans: false
    localAgentHostPort: 127.0.0.1:6831
headers:
    jaegerDebugHeader: debug-id
    jaegerBaggageHeader: baggage
    TraceContextHeaderName: trace-id
    traceBaggageHeaderPrefix: "testctx-"
baggage_restrictions:
    denyBaggageOnInitializationFailure: false
    hostPort: 127.0.0.1:5778
    refreshInterval: 60
)endyaml"};

/*
 * You'll probably want a C++ class or set of classes to contain your trace
 * functionality, logic around application scopes, etc. But we're thunking
 * opentracing to C++ as thinly as we can, so:
 */
static std::shared_ptr<opentracing::Tracer> tracer;

extern "C" void
cdemo_tracing_start(void)
{
	const auto config = jaegertracing::Config::parse(YAML::Load(cfgstr));
	tracer = jaegertracing::Tracer::make("cdemo", config);
}

extern "C" void
cdemo_tracing_finish(void)
{
	if (tracer)
	{
		tracer->Close();
		tracer.reset();
	}
}

/*
 * You'll probably want multiple trace functions to take different arguments.
 * Code generation for the thunks would be wise.
 *
 * You'll likely also need some integration with some sort of application
 * context/scope information. Or a Span stack you keep in the tracing module.
 *
 * But for demo purposes we pass around the info manually.
 *
 * Look into StartSpanWithOptions for how to set up relationships dynamically,
 * pass sets of tags generated from helper functions, etc.
 */
extern "C" CDemoTraceSpan*
cdemo_trace_start(CDemoTraceSpanContext *ctx,
	const char *oprname, const char *something_for_a_tag)
{
	assert(oprname != NULL);
	auto span = tracer->StartSpan(oprname, {opentracing::ChildOf(ctx)});
	if (something_for_a_tag)
		span->SetTag("some_tag", something_for_a_tag);

	/* It's a std::unique_ptr, and we're taking over memory management now */
	return span.release();
}

/*
 * Finishing a span will schedule it for sending to the trace collector. You
 * may add additional tags before finishing if desired, as well as Log entries
 * associated with the span, etc.
 */
extern "C" void
cdemo_trace_done(CDemoTraceSpan *span)
{
	assert(span);
	span->Finish();
	delete span;
}

extern "C" char*
cdemo_trace_context_to_hex(CDemoTraceSpanContext *ctx)
{
	std::stringstream ss (std::ios::out | std::ios::binary);
	char *hexout, *hexout_it;

	assert(tracer);

	if (!ctx)
		return nullptr;

	/*
	 * Inject doesn't accept an output-itertor so we must copy the context
	 * into a stream.
	 */
	if (!tracer->Inject(*ctx, ss))
		return nullptr;

	/*
	 * We want the resulting memory to be free()able so malloc() the buffer. We'd
	 * have to make a copy at some point to hex-format it; if we used C++ streams
	 * we'd have to copy the resulting std::string's c_str() again.
	 */
	string os { ss.str() };
	hexout = (char*)malloc(os.length() * 2 + 1);
	hexout_it = hexout;

	for (auto it = os.begin(); it != os.end(); ++it)
	{
		/*
		 * Not efficient, but your real app won't be using hex strings,
		 * you'll be sending around binary or using base64 or using
		 * http headers or whatever.
		 */
		snprintf(hexout_it, 3, "%02hhX", (unsigned char)(*it));
		hexout_it += 2;
	}
	*hexout_it = '\0';

	std::cerr << "made context hex str " << hexout << std::endl;

	return hexout;
}

extern "C" CDemoTraceSpanContext*
cdemo_trace_context_from_hex(const char *hexbuf)
{
	std::stringstream ss (std::ios::out | std::ios::binary);
	const char * endhexbuf = strchr(hexbuf, '\0');

	assert(tracer);

	if (hexbuf == NULL)
		return nullptr;

	for ( ; hexbuf != endhexbuf; hexbuf += 2)
	{
		unsigned char val;
		if (sscanf(hexbuf, "%02hhX", &val) != 1)
		{
			std::cerr << "bad parse of " << hexbuf << " as hex" << std::endl;
			return nullptr;
		}
		ss << (signed char)val;
	}

	std::stringstream iss (ss.str(), std::ios::in | std::ios::binary);
	auto ctx = tracer->Extract(iss);
	if (!ctx)
		return nullptr;

	return (*ctx).release();
}

extern "C" CDemoTraceSpanContext*
cdemo_trace_context_from_span(CDemoTraceSpan *span)
{
	/*
	 * This is a bit of an API defect in opentracing: it doesn't require a copy
	 * ctor for SpanContext, and the context returned from
	 * opentracing::Span::context() is returned by-value so it won't be valid
	 * outside this scope.
	 */
	assert(span);
	jaegertracing::Span * const jspan = static_cast<jaegertracing::Span*>(span);
	return new jaegertracing::SpanContext(jspan->context());
}

/*
 * You MUST NOT free() memory allocated from C++. This is true even if there's
 * no dtor, as it can corrupt the C and C++ runtime libraries' states on some
 * platforms. But it'll also fail to fire any dtors. So just don't do it.
 *
 * (In fact, you must also avoid free()ing memory that was malloc()'d in
 * another shared library on some platforms, so this is a good habit even
 * without C/C++ mixing).
 */
extern "C" void
cdemo_trace_context_delete(CDemoTraceSpanContext* ctx)
{
	if (ctx)
		delete ctx;
}

extern "C" void
jaeger_debug_print_context(CDemoTraceSpanContext* ctx, const char *msg)
{
#if DEBUG
	std::cerr << std::setw(50) << msg << ": context @" << std::setw(16) << std::hex << (void*)(ctx) << " is ";
	if (ctx)
	{
		jaegertracing::SpanContext *jctx = static_cast<jaegertracing::SpanContext*>(ctx);
		std::cerr << *jctx << std::endl;	
	}
	else
		std::cerr << "null" << std::endl;
#endif
}
