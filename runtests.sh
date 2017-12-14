#!/bin/sh

set -ex
python3.6 setup.py build
PYTHONPATH=build/lib.linux-x86_64-3.6 python3.6 tests/unit.py
