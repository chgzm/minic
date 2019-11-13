#!/bin/bash

function assert_return() {
    file="$1"
    expected="$2"

    ./self/selfminic "./test/${file}" > ./self/tmp.s
    gcc -no-pie -o ./self/tmp ./self/tmp.s
    ./self/tmp
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

    ./self/selfminic "./test/${file}" > ./self/tmp.s
    gcc -static -o ./self/tmp ./self/tmp.s
    actual="$(./self/tmp; printf 'x')" # to reserve last newline, add 'x' to the tail
    actual="${actual%?}"

    printf "\e[1m${file}:\n  \e[0m"
    if [[ "${actual}" = "${expected}" ]]; then
        echo -e "\e[32mExpected: ${expected}, Actual: ${actual} => OK.\e[0m"
    else
        echo -e "\e[31mExpected: ${expected}, Actual: ${actual} => NG.\e[0m"
        exit 1
    fi
}

if [[ -f ./self/all.c ]]; then
    rm ./self/all.c
fi

for file in ./self/def.h util.c tokenizer.c preprocessor.c parser.c generator.c minic.c
do
    cat ${file} >> ./self/all.c
done

./minic "./self/all.c" > ./self/self.s
gcc -no-pie -o ./self/selfminic ./self/self.s

echo -e "\e[36mCompile completed.\e[0m"

assert_return test_return.c 42

