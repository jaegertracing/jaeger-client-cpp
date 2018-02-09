#include <jaegertracing/Tracer.h>

#include <chrono>
#include <thread>
#include <iostream>
#include <sstream>

using std::string;
using std::to_string;
using namespace std::chrono;

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

void usage_die()
{
	std::cerr << "usage: shortlived tag-suffix 0|1 [sleep_s]" << std::endl;
	std::cerr << "  argument 1: suffix to identify this test run" << std::endl;
	std::cerr << "  argument 2: 1 to force flush, or 0 for no explicit flush" << std::endl;
	std::cerr << "  argument 3: seconds to sleep after span close and before exit" << std::endl;
	exit(1);
}

int main(int argc, char *argv[])
{
	bool flush = false;
	int sleep_s = 0;
	const auto config = jaegertracing::Config::parse(YAML::Load(cfgstr));
	auto tracer = jaegertracing::Tracer::make("shortlived", config);

	if (argc < 3)
		usage_die();


	flush = stoi(string(argv[2]));

	if (argc > 3)
		sleep_s = stoi(string(argv[3]));

#ifndef HAVE_EXPLICIT_FLUSH
	if (flush)
	{
		std::cerr << "explicit flush requested but built w/o flush support" << std::endl;
		usage_die();
	}
#endif

	std::ostringstream opname;
	opname << "shortlived-" << argv[1]
		   << "-" << (flush ? "flush" : "noflush")
		   << "-sleep" << to_string(sleep_s) << "s";

	std::cout << "Test with opname \"" << opname.str() << ", flush " << flush << std::endl;
	std::cout << "Buffer flush interval is " << duration_cast<milliseconds>(config.reporter().bufferFlushInterval()).count() << "ms" << std::endl;

	auto span = tracer->StartSpan(opname.str());
	span->SetTag("flush", flush);
	span->SetTag("sleep_s", sleep_s);
	span->SetTag("bufferFlushIntervalMs", duration_cast<milliseconds>(config.reporter().bufferFlushInterval()).count());
	span->Finish();
	span.reset();

	if (flush)
	{
		std::cout << "flushing... " << std::flush;
		auto start = system_clock::now();
#ifdef HAVE_EXPLICIT_FLUSH
		/* This should ensure that all spans reach the server */
		static_cast<jaegertracing::Tracer*>(tracer.get())->flush();
#endif
		auto end = system_clock::now();
		std::cout << " flushed (flush took " << duration_cast<milliseconds>(end-start).count() << "ms)" << std::endl;
	}
	else
		std::cout << "not flushing" << std::endl;

	if (sleep_s)
	{
		std::cout << "sleeping for " << sleep_s << " seconds... " << std::flush;
		std::this_thread::sleep_for(seconds(sleep_s));
		std::cout << "done" << std::endl;
	}
	else
		std::cout << "not sleeping before exit " << std::endl;

	tracer->Close();
}
