[![Build Status][ci-img]][ci] [![Coverage Status][cov-img]][cov] [![OpenTracing 1.0 Enabled][ot-img]][ot-url]

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

The default agent host and port is `127.0.0.1:6831`, you can use a different host/port by updating your reporter configuration.

YAML configuration:

```yml
repoter:
  localAgentHostPort: localhost:6831
```

NOTE: It is not recommended to use a remote host for UDP connections.

### Updating Sampling Server URL

The default sampling collector URL is `http://127.0.0.1:5778`, you can use a different URL by updating the sampler configuration.

```yml
sampler:
  samplingServerURL: http://jaeger-collector.local:5778
```

## License

[Apache 2.0 License](./LICENSE).

[ci-img]: https://travis-ci.org/jaegertracing/jaeger-client-cpp.svg?branch=master
[ci]: https://travis-ci.org/jaegertracing/jaeger-client-cpp
[cov-img]: https://codecov.io/gh/jaegertracing/jaeger-client-cpp/branch/master/graph/badge.svg
[cov]: https://codecov.io/gh/jaegertracing/jaeger-client-cpp
[ot-img]: https://img.shields.io/badge/OpenTracing--1.0-enabled-blue.svg
[ot-url]: http://opentracing.io
