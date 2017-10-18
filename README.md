# JitFFI
JIT compiler for FFI (Foreign-Function Interface).

## Support

Architecture|Operating System|Calling convention |Compilers|Support
------------|----------------|-------------------|---------|-------
x86-64|Windows|Microsoft x64|MSVC, MinGW|Most
x86-64|Linux|System V AMD64 ABI|GCC|Most

## Run Example

```
python ./test.py -t n   # n is the num you want to run.
```

## Compile to Static Library

```
python ./test.py -c
```

This command will create a 'libjitffi.a' file.
