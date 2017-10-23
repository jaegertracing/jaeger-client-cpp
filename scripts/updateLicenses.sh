#!/bin/bash

set -e

python scripts/updateLicense.py $(git ls-files "*\.cpp" "*\.h" | grep -v thrift-gen | grep -v tracetest)
