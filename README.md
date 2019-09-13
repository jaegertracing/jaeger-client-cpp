[![Build Status][ci-img]][ci] [![Coverage Status][cov-img]][cov] [![Appveyor Build][appveyor]][appveyor] [![OpenTracing 1.0 Enabled][ot-img]][ot-url]

# jaeger-client-cpp
C++ OpenTracing binding for [Jaeger](https://www.jaegertracing.io/)

## Contributing

Please see [CONTRIBUTING.md](CONTRIBUTING.md).

## Building

jaeger-client-cpp is built using CMake. It will automatically download
needed dependencies using [Hunter](https://docs.hunter.sh/en/latest/).

To build:

```bash
    mkdir build
    cd build
    cmake ..
    make
```

After building, the [example](./examples/App.cpp) program can be run
with:

```bash
    ./app ../examples/config.yml
```

To run tests:

```bash
    make test
```

To install the library:

```bash
    make install
```

### Generated files

This project uses Apache Thrift for wire-format protocol support code
generation. It currently requires Thrift 0.11.0.

The code can be re-generated with

```bash
    $ git submodule update --init
    $ find idl/thrift/ -type f -name \*.thrift -exec thrift -gen cpp -out src/jaegertracing/thrift-gen {} \;
    $ git apply scripts/thrift-gen.patch
```

### Updating Agent Host and Port

The default host and port for Jaeger Agent is `127.0.0.1:6831`. When the application and Jaeger Agent are running in different containers on the same host, the application's notion of `localhost`/127.0.0.1 it restriced to its own container, so in this case it's usually necessary to override the Agent's host/port by updating your reporter configuration:

YAML configuration:

```yml
repoter:
  localAgentHostPort: jaeger-agent:6831
```

NOTE: It is not recommended to use a remote host for UDP connections.

### Updating Sampling Server URL

The default sampling collector URL is `http://127.0.0.1:5778/sampling`. Similar to UDP address above, you can use a different URL by updating the sampler configuration.

```yml
sampler:
  samplingServerURL: http://jaeger-agent.local:5778
```

## License

[Apache 2.0 License](./LICENSE).

[ci-img]: https://travis-ci.org/jaegertracing/jaeger-client-cpp.svg?branch=master
[ci]: https://travis-ci.org/jaegertracing/jaeger-client-cpp
[appveyor]: https://ci.appveyor.com/api/projects/status/bu992qd3y9bpwe7u?svg=true
[cov-img]: https://codecov.io/gh/jaegertracing/jaeger-client-cpp/branch/master/graph/badge.svg
[cov]: https://codecov.io/gh/jaegertracing/jaeger-client-cpp
[ot-img]: https://img.shields.io/badge/OpenTracing--1.0-enabled-blue.svg
[ot-url]: http://opentracing.io
