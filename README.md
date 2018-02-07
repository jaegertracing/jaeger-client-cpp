[![Build Status][ci-img]][ci] [![Coverage Status][cov-img]][cov] [![OpenTracing 1.0 Enabled][ot-img]][ot-url]

# cpp-client
C++ OpenTracing binding for Jaeger

jaeger-cpp is a client library to emit trace information for the Jaeger trace
collector <http://jaeger.readthedocs.io>.

It's written in C++11 and implements the OpenTracing API; see [ot-url] and
https://github.com/opentracing/opentracing-cpp.

This library is just an agent. It sends UDP packets to a trace collector, which
does all the real work. Agent libraries also exist for other languages like
Java and Go.

## Quick build

To do a test build and install in $HOME/jaeger-cpp-client:

    git clone https://github.com/jaegertracing/cpp-client.git jaeger-cpp-client
    mkdir jaeger-cpp-build
    cd jaeger-cpp-build
    cmake -DCMAKE_INSTALL_PREFIX=$HOME/jaeger-cpp-client ../jaeger-cpp-client
    make
    make install

## Usage

To use jaeger-cpp in your code you mostly use the OpenTracing C++ APIs. Your code
must instantiate and configure the Jaeger tracer, but the rest is the same as for
any other OpenTracing backend.

The simplest way to instantiate and configure a Jaeger tracer is to use the
yaml configuration support to create the tracer. Then set it as the OpenTracing
global tracer. Typically a caller would read the configuration in from a file,
but as an example:

    #include <jaegertracing/Tracer.h>

    void initTracing()
    {
        constexpr auto kConfigYAML = R"cfg(
            disabled: false
            sampler:
                type: const
                param: 1
            reporter:
                queueSize: 100
                bufferFlushInterval: 10
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
            )cfg";

        const auto config = jaegertracing::Config::parse(YAML::Load(kConfigYAML));
        auto tracer = jaegertracing::Tracer::make("postgresql", config);
        opentracing::Tracer::InitGlobal(tracer);
    }

    void teardownTracing()
    {
        opentracing::Tracer::Global()->Close
    }

A C++-language configuration facility is also-available; see
`jaegertracing/Config.h` and the jaeger-cpp tests. A trivial example:

    auto config = jaegertracing::Config(
        false,
        jaegertracing::samplers::Config(jaegertracing::kSamplerTypeConst, 1),
        jaegertracing::reporters::Config(
            jaegertracing::reporters::Config::kDefaultQueueSize,
            std::chrono::seconds(1), true));


Once instantiated, simple traces may be performed with the usual opentracing
facilities e.g.:

    #include <chrono>
    #include <thread>

    int main()
    {
        initTracing();

        auto tracer = opentracing::Tracer::Global();
        auto span = tracer->StartSpan("abc");
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
        span->Finish();
        std::this_thread::sleep_for(std::chrono::seconds{5});

        teardownTracing();
    }

The Jaeger tracing instance usually lives as long as the traced process.

The tracer should be closed before program exit so it can flush its buffers,
so traces aren't lost.

### Usage from C

It's possible to use `jaeger-cpp` from C with appropriate `extern "C"` thunks
and care around resource lifetime management etc. A real world example is the
support for opentracing and jaeger in nginx; see
https://github.com/opentracing-contrib/nginx-opentracing . A simplified example
would be welcomed.

### Usage from C++98

No examples currently exist. It'll at least be possible to do so via an "extern
C" thunking layer and C-style coding.

## Dependencies
---

Compiling jaeger-cpp requires:

* opentracing-cpp <https://github.com/opentracing/opentracing-cpp>
* nlohmann json (2.1.0 or greater) <https://github.com/nlohmann/json>
* yaml-cpp <https://github.com/jbeder/yaml-cpp>
* boost regex <http://www.boost.org>
* Apache thrift <https://thrift.apache.org/>
* Google Test <https://github.com/google/googletest>

### Automatic dependencies - Hunter

By default, Jaeger uses Hunter <http://hunter.sh> to download and build
required dependencies (including opentracing) from the Internet as part of the
CMake build preparation.

Hunter does not install these dependencies as part of `jaeger-cpp`'s `make
install` target, so applications that wish to use a Hunter-built jaeger-cpp
should also use Hunter.

Enable Hunter in your build then

    find_package(jaegertracing)
    target_link_libraries(myapp jaegertracing::jaegertracing)

and CMake will do the rest.

Use `jaegertracing-static` if you want the static lib.

### Locally installed dependencies

To perform a traditional installation to the local system, install all the
above dependencies, then build jaeger-cpp with `-DHUNTER_ENABLED=0` and
`make install` as usual.

Specify the install directory with the `CMAKE_INSTALL_PREFIX` cmake variable.
The default is platform dependent, but `/usr/local` on most UNIXes.

#### Installing Apache Thrift

Because exactly Thrift 0.9.2 or 0.9.3 is required, you'll likely need to
install Thrift manually. To install a minimal C++-only build in /usr/local:

	wget http://archive.apache.org/dist/thrift/0.9.3/thrift-0.9.3.tar.gz \
	     http://archive.apache.org/dist/thrift/0.9.3/thrift-0.9.3.tar.gz.asc

	# now check the gpg signature please! Done? OK:

	tar xf thrift-0.9.3.tar.gz
	cd thrift-0.9.3
	./configure --with-cpp --with-java=no --with-python=no --with-lua=no --with-perl=no --enable-shared=yes --enable-static=yes --enable-tutorial=no --with-qt4=no --prefix=/usr/local
	make
	make install

#### Fedora

These dependencies are all in Fedora 25+ except for nlohmann json 2.1.x
and thrift 0.9.3.

Install with:

    sudo dnf install yaml-cpp-devel boost-devel gtest-devel

There's a nlohmann json package `json-cpp` too, but it's too old so you'll
have to compile it yourself. The Thrift package will be either too old
or too new; cpp-client requires exactly 0.9.2 or 0.9.3.

## Build options

CMake flags available for setting with `-D`:

* `HUNTER_ENABLED`: set to 0 to disable the use of the Hunter dependency
  fetching/building tool to download jaeger-cpp's dependencies. Enabled by
  defualt.

* `BUILD_TESTING`: set to 0 to disable test compilation

* `JAEGERTRACING_WITH_YAML_CPP`: disable yaml configuration support

* `CMAKE_INSTALL_PREFIX`: base directory to install `jaeger-cpp`

* `CMAKE_BUILD_TYPE`: `Debug` or `Release`

Most other cmake standard variables
<https://cmake.org/cmake/help/latest/manual/cmake-variables.7.html> are
supported.

## Running tests

A set of mocked, server-less tests may be run with

    make test

Integration tests using Crossdock <https://github.com/crossdock/crossdock>
may be enabled with the `cmake` option `-DJAEGERTRACING_BUILD_CROSSDOCK=1`
then running `make crossdock-run`.

Coverage tests can be run by building with

    cmake -DCMAKE_BUILD_TYPE=Debug -DJAEGERTRACING_COVERAGE=ON

then running

    make UnitTestCoverage

## Generated files

This project uses Apache Thrift for wire-format protocol support code
generation. It currently requires exactly Thrift 0.9.2 or 0.9.3. Patches have
been applied to the generated code so it cannot be directly re-generated from
the IDL in the `idl` submodule

(see https://github.com/jaegertracing/cpp-client/issues/45)

The code can be re-generated with

    git submodule init
    git submodule update
    find idl/thrift/ -type f -name \*.thrift -exec thrift -gen cpp -out src/jaegertracing/thrift-gen {} \;

but at time of writing (Thrift 0.11.0) the resulting code is invalid due to
https://issues.apache.org/jira/browse/THRIFT-4484

[ci-img]: https://travis-ci.org/jaegertracing/cpp-client.svg?branch=master
[ci]: https://travis-ci.org/jaegertracing/cpp-client
[cov-img]: https://codecov.io/gh/jaegertracing/cpp-client/branch/master/graph/badge.svg
[cov]: https://codecov.io/gh/jaegertracing/cpp-client
[ot-img]: https://img.shields.io/badge/OpenTracing--1.0-enabled-blue.svg
[ot-url]: http://opentracing.io
