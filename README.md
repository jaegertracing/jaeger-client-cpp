[![Build Status][ci-img]][ci] [![Coverage Status][cov-img]][cov] [![OpenTracing 1.0 Enabled][ot-img]][ot-url]

# cpp-client
C++ OpenTracing binding for Jaeger

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
