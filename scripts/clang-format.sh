#!/bin/bash

clang-format -i $(git ls-files src | grep -E -v 'thrift\-gen' | grep -E '\.(cpp|h)$')
