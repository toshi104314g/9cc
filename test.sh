#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 '0;'
assert 42 '42;'
assert 3 "1+2;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 -5 ;"
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'
assert 2 '-(3+5)+2*+5;'
assert 1 '1 == 1;'
assert 0 '1 == 0;'
assert 1 '1<2;'
assert 0 ' 1 < 1;'
assert 1 '2 > 1;'
assert 0 '1 > 1;'
assert 1 ' 1 <= 1;'
assert 0 ' 1<= 0;'
assert 1 ' 3 >=3;'
assert 0 ' 3 >=4;'
assert 1 '1!=0;'
assert 0 '1!=1;'
assert 5 'a=5;'
assert 5 'a=1;2+3;'
assert 1 'a=1;a;'
assert 2 'a=1;b=2;'
assert 1 'a=5; a-4;'
assert 10 'a=30;b=20;c=10;c;'
assert 10 'a=30;b=20;a=10;a;'
assert 30 'a=30;b=20;a;'
assert 20 'a=30;b=20;a;b;'
assert 20 'a=30;b=20;c=10;b;'
assert 5 'a=5;b=3;a;'
assert 4 'a=1;b=3;a+b;'
assert 4 'a=1;
          b=3;
          a+b;'

echo OK

