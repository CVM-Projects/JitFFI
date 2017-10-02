# JitFFI
FFI (Foreign-Function Interface) with JIT.

## Support

Architecture|Operating System|Calling convention |Compilers|Support
------------|----------------|-------------------|---------|-------
x86-64|Windows|Microsoft x64|MSVC, MinGW|GCCPartial
x86-64|Linux|System V AMD64 ABI|GCC|Partial

## Run Example

```
g++ -std=c++14 testsuite/test1.cpp source/jitffi.cpp -Iinclude -o test
./test
```
