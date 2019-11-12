#!/bin/bash

if [[ -f ./self/all.c ]]; then
    rm ./self/all.c
fi

for file in ./self/def.h util.c tokenizer.c preprocessor.c parser.c generator.c minic.c
do
    cat ${file} >> ./self/all.c
done

./minic "./self/all.c" > ./self/self.s
gcc -no-pie -o ./test/selfminic ./self/self.s

echo -e "\e[36mCompile completed.\e[0m"
