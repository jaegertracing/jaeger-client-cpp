#include <iostream>

#include <yaml-cpp/yaml.h>

#include <jaegertracing/Tracer.h>

namespace {

void setUpTracer(const char* configFilePath)
{
    auto configYAML = YAML::LoadFile(configFilePath);
    auto config = jaegertracing::Config::parse(configYAML);
    assert(config.reporter().logSpans());
    auto tracer = jaegertracing::Tracer::make("example-service", config);
    opentracing::Tracer::InitGlobal(
        std::static_pointer_cast<opentracing::Tracer>(tracer));
}

void tracedSubroutine(const std::unique_ptr<opentracing::Span>& parentSpan)
{
    opentracing::Tracer::Global()->StartSpan(
        "tracedSubroutine", { opentracing::ChildOf(&parentSpan->context()) });
}

void tracedFunction()
{
    auto span = opentracing::Tracer::Global()->StartSpan("tracedFunction");
    tracedSubroutine(span);
}

}  // anonymous namespace

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <config-yaml-path>\n";
        return 1;
    }
    try {
        setUpTracer(argv[1]);
        tracedFunction();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
    }
    return 0;
}
