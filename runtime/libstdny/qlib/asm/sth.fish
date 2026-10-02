#!/bin/fish

sudo ranlib /usr/local/lib/libstdny.a
g++ test_qlib.cpp /usr/local/lib/libstdny.a -o out/qtest -mavx -lm
./out/qtest
