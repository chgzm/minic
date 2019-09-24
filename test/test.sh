#!/bin/bash

function try() {
    file="$1"
    expected="$2"

    ./minic "./test/${file}" > ./test/tmp.s
    gcc -o ./test/tmp ./test/tmp.s
    ./test/tmp
    actual="$?"

    printf "\e[1m${file}:\n  \e[0m"
    if [[ "${actual}" = "${expected}" ]]; then
        echo -e "\e[32mExpected: ${expected}, Actual: ${actual} => OK.\e[0m"
    else
        echo -e "\e[31mExpected: ${expected}, Actual: ${actual} => NG.\e[0m"
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
try test_return_paren.c 50
try test_return_paren_2.c 35
try test_localvar.c 42
try test_localvar_2.c 7
try test_localvar_3.c 17
try test_localvar_4.c 7
try test_localvar_5.c 34
try test_func.c 42
try test_func_2.c 5
try test_func_3.c 10
try test_func_4.c 25
try test_func_5.c 10
try test_func_6.c 215

echo -e "\e[36mPassed all tests.\e[0m"
