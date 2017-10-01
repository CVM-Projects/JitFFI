# JitFFI
FFI (Foreign-Function Interface) wit JIT.

## Support

Architecture|Operating System|Calling convention |Support
------------|----------------|-------------------|-------
x86-64|Windows|Microsoft x64|Partial
x86-64|Linux|System V AMD64 ABI|Partial

## Run Example

```
g++ -std=c++14 testsuite/test1.cpp source/jitffi.cpp -Iinclude -o test
./test
```
