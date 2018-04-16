#include "cdemo_tracing.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

/*
 * Fake a RPC call using a subprocess to demonstrate marshalling and unmarshalling
 * of context info across process boundaries.
 */
static void
send_fake_rpc(CDemoTraceSpanContext *trace_ctx, const char *endpoint, const char *tagval, int depth)
{
	CDemoTraceSpan *span = NULL;
	CDemoTraceSpanContext *innerctx = NULL;
	char *ctx_arg_hexstr;
#define CDEMO_MAX_CMDLINE_LENGTH 300
	char cmdline[CDEMO_MAX_CMDLINE_LENGTH];

	jaeger_debug_print_context(trace_ctx, "main_passed");

	span = cdemo_trace_start(trace_ctx, "send_fake_rpc", NULL);
	innerctx = cdemo_trace_context_from_span(span);

	jaeger_debug_print_context(innerctx, "rpc inner");

	/*
	 * Marshal the trace context for sending as a command line argument.
	 *
	 * But since this example doesn't know what your app needs, it just
	 * gets a binary context, then munges it to hex for sending on a command
	 * line.
	 */
	ctx_arg_hexstr = cdemo_trace_context_to_hex(innerctx);

	if (strchr(endpoint, '\"'))
	{
		/* We should escape the endpoint path but meh, we'll bail out */
		fprintf(stderr, "paths containing double quotes not handled here");
		exit(1);
	}

	/*
	 * Make the command line to fake up the "RPC" and
	 * run the 
	 */
	snprintf(cmdline, CDEMO_MAX_CMDLINE_LENGTH,
		"\"%s\" %s-child-%d %d \"%s\"", endpoint, tagval, depth, depth, ctx_arg_hexstr);
	cmdline[CDEMO_MAX_CMDLINE_LENGTH-1] = '\0';
#if DEBUG
	fprintf(stderr, "invoking %s\n", cmdline);
#endif

	/*
	 * And invoke the child proc synchronously. We fire and forget here,
	 * and don't care if it succeeded or not.
	 */
	system(cmdline);

	/*
	 * Hopefully whatever framework/tool you're using has cleanup hooks or
	 * other helpers so you don't have to do this manual cleanup, but this demo
	 * is pure C, so:
	 */
	free(ctx_arg_hexstr);
	cdemo_trace_context_delete(innerctx);
	cdemo_trace_done(span);
}

static void
usage_die(void)
{
	fprintf(stderr, "usage: cdemo tagstring [ncalls [opentracing-context-as-hex]]\n");
	exit(1);
}

int main(int argc, const char* argv[])
{
	const char * tagval = NULL;
	CDemoTraceSpan *span = NULL;
	CDemoTraceSpanContext *imported_ctx = NULL;
	CDemoTraceSpanContext *ctx = NULL;
	int depth;

	/* Bring up the tracer */
	cdemo_tracing_start();

	if (argc < 2)
		usage_die();

	tagval = argv[1];
	fprintf(stderr, "cdemo invoked with tag \"%s\"", tagval);

	if (argc > 2)
	{
		char *endptr;
		depth = strtol(argv[2], &endptr, 10);
		if (endptr == argv[2])
			usage_die();
	}
	else
	{
		/* Default to single level calls */
		depth = 1;
	}
	fprintf(stderr, " at depth %d", depth);

	if (argc > 3)
	{
		/*
		 * We're being called with an exported trace context. Import it
		 * to use as the parent context for our trace.
		 */
		imported_ctx = cdemo_trace_context_from_hex(argv[3]);
		assert(imported_ctx);
		fprintf(stderr, " and with context \"%s\"", argv[3]);
	}
	fprintf(stderr, "\n");

	jaeger_debug_print_context(imported_ctx, "imported");

	span = cdemo_trace_start(imported_ctx, "main", tagval);
	ctx = cdemo_trace_context_from_span(span);

	jaeger_debug_print_context(ctx, "main");

	if (depth > 0)
		send_fake_rpc(ctx, argv[0], tagval, depth - 1);

	cdemo_trace_context_delete(ctx);
	cdemo_trace_done(span);

	/* Shut down the tracer */
	cdemo_tracing_finish();
}
