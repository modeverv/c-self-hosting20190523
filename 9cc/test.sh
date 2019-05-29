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

try 0 "return 0;"
try 42 "42;"
try 21 'return 5+20-4;'
try 41 '  12   + 34 - 5;'
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 4 "return (   3  +  5)  /  2;"
try 5 "-3 + 8;"
try 3 "   +6 + -3;"
try 2 " -1 * -1 + +1   ;"

try 1 " 1 == 1 ;"
try 1 "return 1 == 1 ;"
try 0 " 1 == 0;"
try 1 " 1 != 2;"
try 0 " 1 != 1;"
try 1 " 1 < 2;"
try 0 " 1 < 1;"
try 1 " 1 <= 1;"
try 1 " 1 <= 2;"
try 0 " 1 <= 0;"
try 1 " 2 > 1;"
try 0 " 2 > 2;"
try 1 " 2 >= 1 ;"
try 1 " 2 >= 2 ;"
try 0 " 2 >= 3 ;"
try 42 "return 42;"
try 2 " a = b = 2;"
try 2 "return a = b = 2;"
try 2 "a = 2;return 2;"
try 4 "a = 2;b= a+2;return b;"
try 3 "a = 4;a = 2+1;return a ;"
try 3 "aa = 4;aa = 2+1;return aa ;"
echo OK
