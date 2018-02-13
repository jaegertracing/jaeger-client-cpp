#ifndef JAEGERTRACING_CDEMO_H
#define JAEGERTRACING_CDEMO_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
typedef opentracing::v1::Span CDemoTraceSpan;
typedef opentracing::v1::SpanContext CDemoTraceSpanContext;
#else
/*
 * CDemoTraceSpan is an opaque struct for C code to prevent
 * accidental free()ing of the C++ pointers that must be
 * delete'd from C++.
 */
struct CDemoTraceSpan;
typedef struct CDemoTraceSpan CDemoTraceSpan;
struct CDemoTraceSpanContext;
typedef struct CDemoTraceSpanContext CDemoTraceSpanContext;
#endif

void cdemo_tracing_start(void);
void cdemo_tracing_finish(void);

extern CDemoTraceSpan* cdemo_trace_start(CDemoTraceSpanContext *ctx, const char *oprname, const char *something_for_a_tag);

extern void cdemo_trace_done(CDemoTraceSpan *span);

/*
 * For manipulating span contexts from C via null-terminated hex strings.
 */
extern char* cdemo_trace_context_to_hex(CDemoTraceSpanContext *ctx);

extern CDemoTraceSpanContext* cdemo_trace_context_from_hex(const char *hexbuf);

extern CDemoTraceSpanContext* cdemo_trace_context_from_span(CDemoTraceSpan *span);

extern void cdemo_trace_context_delete(CDemoTraceSpanContext* ctx);

extern void jaeger_debug_print_context(CDemoTraceSpanContext* ctx, const char *msg);

#ifdef __cplusplus
}
#endif

#endif
