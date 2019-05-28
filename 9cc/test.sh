#! /bin/bash

try() {
    expected=$1
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$expected expected, but got $actual"
        exit 1
    fi
}

try 42 "return 42;"
try 2 " a = b = 2;"
try 2 "return a = b = 2;"
try 2 "a = 2;return 2;"
try 4 "a = 2;b= a+2;return b;"
try 3 "a = 4;a = 2+1;return a ;"
echo OK
