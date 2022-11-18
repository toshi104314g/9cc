#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s ext_functions.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

#assert 1 ',;'
assert 100 'main(){return 100;}'
assert 100 'main(){x=100; return x;}'
assert 11  'main() {x=1;for(i=1; i<5;i=i+1) {x=i+x;}; return x;}'
assert 10 'add(){return 100;} main(){x= 10; return x;}'
assert 10 'plus(){return 100;} main(){plus(); return 10;}'
assert 100 'plus(){return 100;} main(){return plus();}'
assert 10 'plus(){return 100;} main(){x= plus(); return 10;}'
assert 100 'plus(){return 100;} main(){x= plus(); return x;}'
assert 100 'plus(){y=10; z=90;return z+y;} main(){x= plus(); return x;}'
assert 100 'plus(){x=10; z=90;return z+x;} main(){x= plus(); return x;}'
assert 21 'main(){x= args(1,2,3,4,5,6);return x;}'
assert 100 'plus(x){return x;} main(){return plus(100);}'
assert 100 'plus(x){return x;} main(){y = plus(100);return y;}'
assert 100 'plus(x){return x+5;} main(){return plus(95);}'
assert 100 'add(x,y) {return x+y;}; main(){return add(10,90);};'
assert 1  'fib(n) {if (n == 1) {return 1;} else if (n ==2) {return 2;} else {return fib(n-1) + fib(n-2);}} main() {return fib(1);}'
assert 1 'fib(n) {if (n == 1) {return 1;} else if (n ==2) {return 2;} else {return fib(n-1) + fib(n-2);}} main() {return fib(2);}'
assert 2 'fib(n) {if (n == 1) {return 1;} else if (n ==2) {return 2;} else {return fib(n-1) + fib(n-2);}} main() {return fib(3);}'
assert 21 'fib(n) {if (n == 1) {return 1;} else if (n ==2) {return 2;} else {return fib(n-1) + fib(n-2);}} main() {return fib(8);}'
#assert 42 '42;'
#assert 3 "1+2;"
#assert 21 "5+20-4;"
#assert 41 " 12 + 34 -5 ;"
#assert 47 '5+6*7;'
#assert 15 '5*(9-6);'
#assert 4 '(3+5)/2;'
#assert 10 '-10+20;'
#assert 2 '-(3+5)+2*+5;'
#assert 1 '1 == 1;'
#assert 0 '1 == 0;'
#assert 1 '1<2;'
#assert 0 ' 1 < 1;'
#assert 1 '2 > 1;'
#assert 0 '1 > 1;'
#assert 1 ' 1 <= 1;'
#assert 0 ' 1<= 0;'
#assert 1 ' 3 >=3;'
#assert 0 ' 3 >=4;'
#assert 1 '1!=0;'
#assert 0 '1!=1;'
#assert 5 'a=5;'
#assert 5 'a=1;2+3;'
#assert 1 'a=1;a;'
#assert 2 'a=1;b=2;'
#assert 1 'a=5; a-4;'
#assert 10 'a=30;b=20;c=10;c;'
#assert 10 'a=30;b=20;a=10;a;'
#assert 30 'a=30;b=20;a;'
#assert 20 'a=30;b=20;a;b;'
#assert 20 'a=30;b=20;c=10;b;'
#assert 5 'a=5;b=3;a;'
#assert 4 'a=1;b=3;a+b;'
#assert 4 'a=1;
#          b=3;
#          a+b;'
#assert 5 'A1=2; A2=1; A1 + A2 +2;'
#assert 5 'return 5;'
#assert 10 'a=5; b=2; c=a*b; return c;'
#assert 10 'a=3; b=13; return b-a;'
#assert 5 'if(1 < 2) return 5;'
#assert 5 'if(1 < 2) return 5; else return 10;'
#assert 10 'if(1 > 2) return 5; else return 10;'
#assert 5  'a=1; if(a+2 >= 3) a = a+4; return a;'
#assert 6 'i=0; while(i <=5) i=i+1; return i;'
#assert 6 'for(i=0; i<=5; i=i+1) 1+1; return i;' 
#assert 6 'i=0; for(; i<=5; i=i+1) 1+1; return i;' 
#assert 6 'i=0; for(; i<=5; ) i = i+1; return i;' 
#assert 6 'j=0; for(i=0; i<=5; i=i+1) j=j+1; return j;' 
#assert 10 '{return 10;}'
#assert 20 'j=0; for (i=1; i<=5; i=i+1) {j = j+i; k =i;} return j+k;' 
#assert 0 'foo();return 0;'
#assert 0 'bar(5);return 0;'
#assert 21 'return args(1,2,3,4,5,6);'
#
echo OK

