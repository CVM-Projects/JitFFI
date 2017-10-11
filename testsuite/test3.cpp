// test3.cpp
// This file can run on the x86-64 system.
#include "testsuite.h"

void function_1(long double v)
{
	print(v);
}

int function_2(int a, float b, double c, long double d)
{
	print(a);
	print(b);
	print(c);
	print(d);

	return 0;
}

void Call_1(JitFuncCreater &jfc)
{
	long double ld = 2.5;

	CurrABI::create_function_caller(jfc, &function_1, { &ld }, { &atu_ldouble });
}

void Call_2(JitFuncCreater &jfc)
{
	int i = 5;
	float f = 6.75;
	double d = 8.0;
	long double ld = 2.5;

	CurrABI::create_function_caller(jfc, &function_2, { &i, &f, &d, &ld }, { &atu_int, &atu_float, &atu_double, &atu_ldouble });
}

int main()
{
	Call(Call_1);
	Call(Call_2);
}
