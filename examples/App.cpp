#include <iostream>

#include <yaml-cpp/yaml.h>

#include <jaegertracing/Tracer.h>
#include <jaegertracing/TracerFactory.h>
using namespace jaegertracing;
using namespace std;
using namespace jaegertracing::propagation;
//using namespace
//namespace {

class HeaderKeyWriter : public opentracing::HTTPHeadersWriter {
  public:
    HeaderKeyWriter(){}

    opentracing::expected<void> Set(
        opentracing::string_view key,
        opentracing::string_view value) const override {
        cout<<"key: "<<key << " value: "<<value<<endl;
        return {};
    }

};
void demo(string& configFilePath){

    auto configYAML = YAML::LoadFile(configFilePath);
    auto config = jaegertracing::Config::parse(configYAML);
    auto tracer = jaegertracing::Tracer::make(
        "example-service", config, jaegertracing::logging::consoleLogger());
    auto span = tracer->StartSpan("test");
    std::ostringstream oss;
    HeaderKeyWriter writer = HeaderKeyWriter();
    tracer->Inject(span->context(), dynamic_cast<opentracing::HTTPHeadersWriter&>(writer));
}
int main(int argc, char* argv[])
{
    string configFilePath  = argv[1];
   demo(configFilePath);
    return 0;
}
