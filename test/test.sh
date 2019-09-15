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
try test_return_add.c 7
try test_return_add_2.c 12
try test_return_sub.c 6
try test_return_sub_2.c 3
try test_return_add_sub.c 165
try test_return_mul.c 42
try test_return_mul_2.c 60
try test_return_div.c 7
try test_return_div_2.c 4
try test_return_mod.c 2
try test_return_mod_2.c 0
try test_return_mix.c 33

echo OK        
