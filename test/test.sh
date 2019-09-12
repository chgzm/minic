#!/bin/bash

function try() {
    file="$1"
    expected="$2"

    ./minic "./test/${file}" > ./test/tmp.s
    gcc -o ./test/tmp ./test/tmp.s
    ./test/tmp
    actual="$?"

    if [[ "${actual}" = "${expected}" ]]; then
        echo "Got $actual => OK."
    else
        echo "${expected} expected, but got ${actual}"
        exit 1
    fi
}

try test_return.c 42

echo OK        
