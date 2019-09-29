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
try test_func_7.c 15
try test_func_8.c 20
try test_func_8.c 20

try test_if.c 100
try test_if_2.c 2
try test_if_3.c 200
try test_if_4.c 100
try test_if_5.c 100
try test_if_6.c 1
try test_if_7.c 100
try test_if_8.c 1
try test_if_9.c 100
try test_if_10.c 1
try test_if_11.c 100
try test_if_12.c 1
try test_if_13.c 100
try test_if_14.c 1

try test_if_else.c 100
try test_if_else_2.c 200
try test_if_else_3.c 200
try test_if_else_4.c 30

try fib.c 89

try test_while.c 55

try test_inc.c 1
try test_inc_2.c 2

try test_dec.c 2
try test_dec_2.c 1

try test_for.c 10
try test_for_2.c 10

try test_break.c 6
try test_break_2.c 6
try test_break_3.c 100

try test_continue.c 7
try test_continue_2.c 7

try test_muleq.c 9
try test_diveq.c 3
try test_modeq.c 1
try test_addeq.c 6
try test_subeq.c 0

try test_array.c 100
try test_array_2.c 10

try test_ptr.c 5
try test_ptr_2.c 100
try test_ptr_3.c 100
try test_ptr_4.c 10

try test_global.c 9

echo -e "\e[36mPassed all tests.\e[0m"
