#!/bin/bash

set -e

function main() {
    local project_dir
    project_dir="$(git rev-parse --show-toplevel)"

    mkdir -p build
    cd build
    export CFLAGS="$CFLAGS -march=x86-64"
    export CXXFLAGS="$CXXFLAGS -march=x86-64"

    cat <<EOF > export.map
{
    global:
        OpenTracingMakeTracerFactory;
    local: *;
};
EOF
    CXXFLAGS="-Wno-error=deprecated-copy"
    cmake -DCMAKE_BUILD_TYPE=Release \
        -DJAEGERTRACING_PLUGIN=ON \
        -DBUILD_TESTING=ON \
        -DHUNTER_CONFIGURATION_TYPES=Release \
        ..
    make -j3
    pwd
    mkdir ./utest
    mv libjaegertracing_plugin.so ./utest/libjaegertracing_plugin.so
    ./DynamicallyLoadTracerTest ./utest/libjaegertracing_plugin.so
}

main
