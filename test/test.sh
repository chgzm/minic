#!/bin/bash

function assert_return() {
    file="$1"
    expected="$2"

    ./minic "./test/${file}" > ./test/tmp.s
    gcc -no-pie -o ./test/tmp ./test/tmp.s
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

function assert_output() {
    file="$1"
    expected="$(printf "$2"; printf 'x')" # to reserve last newline, add 'x' to the tail
    expected="${expected%?}"

    ./minic "./test/${file}" > ./test/tmp.s
    gcc -static -o ./test/tmp ./test/tmp.s
    actual="$(./test/tmp; printf 'x')" # to reserve last newline, add 'x' to the tail
    actual="${actual%?}"

    printf "\e[1m${file}:\n  \e[0m"
    if [[ "${actual}" = "${expected}" ]]; then
        echo -e "\e[32mExpected: ${expected}, Actual: ${actual} => OK.\e[0m"
    else
        echo -e "\e[31mExpected: ${expected}, Actual: ${actual} => NG.\e[0m"
        exit 1
    fi
}

assert_return test_return.c 42
assert_return test_return_add.c 7
assert_return test_return_add_2.c 12
assert_return test_return_sub.c 6
assert_return test_return_sub_2.c 3
assert_return test_return_add_sub.c 165
assert_return test_return_mul.c 42
assert_return test_return_mul_2.c 60
assert_return test_return_div.c 7
assert_return test_return_div_2.c 4
assert_return test_return_mod.c 2
assert_return test_return_mod_2.c 0
assert_return test_return_mix.c 33
assert_return test_return_paren.c 50
assert_return test_return_paren_2.c 35
assert_return test_return_minus.c 1

assert_return test_localvar.c 42
assert_return test_localvar_2.c 7
assert_return test_localvar_3.c 17
assert_return test_localvar_4.c 7
assert_return test_localvar_5.c 34
assert_return test_localvar_6.c 34
assert_return test_localvar_7.c 120
assert_return test_localvar_8.c 31

assert_return test_func.c 42
assert_return test_func_2.c 5
assert_return test_func_3.c 10
assert_return test_func_4.c 25
assert_return test_func_5.c 10
assert_return test_func_6.c 215
assert_return test_func_7.c 15
assert_return test_func_8.c 20
assert_return test_func_9.c 15
assert_return test_func_10.c 150
assert_output test_func_11.c "usage"

assert_return test_if.c 100
assert_return test_if_2.c 2
assert_return test_if_3.c 200
assert_return test_if_4.c 100
assert_return test_if_5.c 100
assert_return test_if_6.c 1
assert_return test_if_7.c 100
assert_return test_if_8.c 1
assert_return test_if_9.c 100
assert_return test_if_10.c 1
assert_return test_if_11.c 100
assert_return test_if_12.c 1
assert_return test_if_13.c 100
assert_return test_if_14.c 1

assert_return test_if_else.c 100
assert_return test_if_else_2.c 200
assert_return test_if_else_3.c 200
assert_return test_if_else_4.c 30

assert_return fib.c 89

assert_return test_while.c 55

assert_return test_inc.c 1
assert_return test_inc_2.c 2

assert_return test_dec.c 2
assert_return test_dec_2.c 1

assert_return test_for.c 10
assert_return test_for_2.c 10
assert_return test_for_3.c 10

assert_return test_break.c 6
assert_return test_break_2.c 6
assert_return test_break_3.c 100

assert_return test_continue.c 7
assert_return test_continue_2.c 7

assert_return test_muleq.c 9
assert_return test_diveq.c 3
assert_return test_modeq.c 1
assert_return test_addeq.c 6
assert_return test_subeq.c 0

assert_return test_array.c 100
assert_return test_array_2.c 10
assert_return test_array_3.c 3
assert_return test_array_4.c 100
assert_return test_array_5.c 15
assert_output test_array_6.c "Hello, World!!"

assert_return test_ptr.c 5
assert_return test_ptr_2.c 100
assert_return test_ptr_3.c 100
assert_return test_ptr_4.c 10
assert_return test_ptr_5.c 17
assert_return test_ptr_6.c 1

assert_return test_global.c 9
assert_return test_global_2.c 10
assert_output test_global_3.c "Hello"
assert_return test_global_4.c 3

assert_return test_struct.c 6
assert_return test_struct_2.c 15
assert_return test_struct_3.c 120
assert_return test_struct_4.c 3
assert_return test_struct_5.c 6
assert_return test_struct_6.c 63

assert_return test_comment.c 42
assert_return test_comment_2.c 42

assert_return test_switch.c 3
assert_return test_switch_2.c 14
assert_return test_switch_3.c 70
assert_return test_switch_4.c 6

assert_return test_sizeof.c 17
assert_return test_sizeof_2.c 32
assert_return test_sizeof_3.c 80

assert_return test_typedef.c 3

assert_return test_preprocess.c 42
assert_return test_preprocess_2.c 3
assert_return test_preprocess_3.c 1
assert_return test_preprocess_4.c 9
assert_return test_preprocess_5.c 6

assert_return test_enum.c 4

assert_return test_arrow.c 7
assert_return test_arrow_2.c 7

assert_return test_or.c 28

assert_return test_and.c 16
assert_return test_and_2.c 100

assert_return test_char.c 99
assert_return test_char_2.c 195
assert_output test_char_3.c "hello"
assert_output test_char_4.c "hey"
assert_return test_char_5.c 195
assert_return test_char_6.c 195

assert_return test_conditional_op.c 9

assert_return test_post_inc_dec.c 1
assert_return test_post_inc_dec_2.c 8

assert_return test_not.c 1
assert_return test_not_2.c 0

assert_return test_vector.c 30

assert_return test_stack.c 6

assert_return test_strptrmap.c 10

assert_output test_printf.c "Hello, World."

assert_output test_arg.c "./test/tmp"

assert_return test_tokenizer.c 127
assert_return test_tokenizer_2.c 20
assert_return test_tokenizer_3.c 51
assert_return test_tokenizer_4.c 202

assert_return test_preprocessor.c 202

echo -e "\e[36mPassed all tests.\e[0m"
