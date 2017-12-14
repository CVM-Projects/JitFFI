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

## Usage (in C++)

JitFuncPool is a memory pool, which is executable.

JitFunc is a part of a JitFuncPool, which stands for a function.

To create JitFunc, JitFuncCreater is used.

1. Compile to library.
2. Using `#include "jitffi.h"` and import this library.

Example Code:

```c++
#include "jitffi.h"
#include <stdio.h>

double func(double v)
{
	printf("%lf\n", v);
	return v * 2;
}

int main(void)
{
	using namespace JitFFI;
	using namespace JitFFI::CurrABI;

	ArgTypeList tl = { &atu_double };

	ArgumentInfo info = get_argumentinfo(atu_double, tl);

	JitFuncPool jfp(0x1000, JitFuncPool::ReadWrite);
	JitFunc jf(jfp);
	JitFuncCreater jfc(jf);

	create_function_caller(jfc, info, (void*)&func);
	auto f = jfc.get().func<void(void*, void**)>();

	double v = 3.14;
	double r;
	void* dl[] = { &v };
	f(&r, dl);
	printf("%lf\n", r);
}
```

Command:

```
python ./test.py -c                 # Create libjitffi.a
g++ test.cpp -L. -ljitffi -o test   # Create test
./test                              # Run test
```

## Usage (in C)

JitFFI has a header file named `jitffi_c.h` for C program.

All struct/class are packed with `jitffi_XXX`, and passed by pointer.

Only support CurrABI in C header.

`JitFFI::_Platform_::atu_XX` in C is `jitffi_type_XX`.

1. Compile to library. (Add `jitffi-c.cpp` to source.)
2. Using `#include "jitffi-c.h"` and import this library.

Example Code:

```c
#include "jitffi-c.h"
#include <stdio.h>

double func(double v)
{
	printf("%lf\n", v);
	return v * 2;
}

int main(void)
{
	jitffi_argtype_ptr tl[] = { jitffi_type_double, NULL };

	jitffi_arginfo *info = jitffi_create_arginfo(jitffi_type_double, tl);

	jitffi_jfp *jfp = jitffi_create_jfp(0x1000, jitffi_readwrite);
	jitffi_jf *jf = jitffi_create_jf(jfp);
	jitffi_jfc *jfc = jitffi_create_jfc(jf);

	jitffi_compile(jfc, info, (void*)&func, NULL);
	jitffi_f2 f = (jitffi_f2)jitffi_getfunc(jfc);

	double v = 3.14;
	double r;
	const void* dl[] = { &v };
	f(&r, dl);
	printf("%lf\n", r);

	jitffi_release_jfc(jfc);
	jitffi_release_jf(jf);
	jitffi_release_jfp(jfp);
}
```

Command:

```
python ./test.py -c               # Create libjitffi.a
gcc -c test.c                     # Create test.o
g++ test.o -L. -ljitffi -o test   # Create test
./test                            # Run test
```
