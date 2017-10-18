// test1.cpp
// This file can run on the x86-64 system.
#include "testsuite.h"

using int64 = int64_t;

void print(int64 n)
{
	print_num(n);
}

int callerN2(int64 a, int64 b)
{
	print(a);
	print(b);

	return 12345;
}

int callerN4(int64 a, int64 b, int64 c, int64 d)
{
	print(a);
	print(b);
	print(c);
	print(d);

	return 1234;
}
int callerN5(int64 a, int64 b, int64 c, int64 d, int64 e)
{
	print(a);
	print(b);
	print(c);
	print(d);
	print(e);

	return 1234;
}
int callerN6(int64 a, int64 b, int64 c, int64 d, int64 e, int64 f)
{
	print(a);
	print(b);
	print(c);
	print(d);
	print(e);
	print(f);

	return 1234;
}

int callerN(int64 a, int64 b, int64 c, int64 d, int64 e, int64 f, int64 g, int64 h)
{
	print(a);
	print(b);
	print(c);
	print(d);
	print(e);
	print(f);
	print(g);
	print(h);

	return 12345;
}

int callerN9(int64 a, int64 b, int64 c, int64 d, int64 e, int64 f, int64 g, int64 h, int64 i)
{
	print(a);
	print(b);
	print(c);
	print(d);
	print(e);
	print(f);
	print(g);
	print(h);
	print(i);

	return 123456;
}

int callerN10(int64 a, int64 b, int64 c, int64 d, int64 e, int64 f, int64 g, int64 h, int64 i, int64 j)
{
	print(a);
	print(b);
	print(c);
	print(d);
	print(e);
	print(f);
	print(g);
	print(h);
	print(i);
	print(j);

	return 1234567;
}

int callerX(double a, double b, double c, double d, double e, double f, double g, double h)
{
	print_num(a);
	print_num(b);
	print_num(c);
	print_num(d);
	print_num(e);
	print_num(f);
	print_num(g);
	print_num(h);

	return 54321;
}

int callerX4(double a, double b, double c, double d)
{
	print_num(a);
	print_num(b);
	print_num(c);
	print_num(d);

	return 54321;
}

int callerX5(double a, double b, double c, double d, double e)
{
	print_num(a);
	print_num(b);
	print_num(c);
	print_num(d);
	print_num(e);

	return 54321;
}

int callerX7(double a, double b, double c, double d, double e, double f, double g)
{
	print_num(a);
	print_num(b);
	print_num(c);
	print_num(d);
	print_num(e);
	print_num(f);
	print_num(g);

	return 54321;
}
int callerX8(double a, double b, double c, double d, double e, double f, double g, double h)
{
	print_num(a);
	print_num(b);
	print_num(c);
	print_num(d);
	print_num(e);
	print_num(f);
	print_num(g);
	print_num(h);

	return 54321;
}
int callerX9(double a, double b, double c, double d, double e, double f, double g, double h, double i)
{
	print_num(a);
	print_num(b);
	print_num(c);
	print_num(d);
	print_num(e);
	print_num(f);
	print_num(g);
	print_num(h);
	print_num(i);

	return 54321;
}

int callerX10(double a, double b, double c, double d, double e, double f, double g, double h, double i, double j)
{
	print_num(a);
	print_num(b);
	print_num(c);
	print_num(d);
	print_num(e);
	print_num(f);
	print_num(g);
	print_num(h);
	print_num(i);
	print_num(j);

	return 54321;
}

int caller(int64 a, int64 b, int64 c, int64 d)
{
	printf("%lld\n", a);
	printf("%lld\n", b);
	printf("%lld\n", c);
	printf("%lld\n", d);

	return 12345;
}

void caller(int64 a, int64 b)
{
	printf("%lld\n", a);
	printf("%lld\n", b);
}

void callerNX(int64 a0, double b0, int64 a1, double b1, int64 a2, double b2, int64 a3, double b3) {
	print_num(a0);
	print_num(b0);
	print_num(a1);
	print_num(b1);
	print_num(a2);
	print_num(b2);
	print_num(a3);
	print_num(b3);
}

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
	double v0 = 1.0;
	double v1 = 2.0;
	double v2 = 3.0;
	double v3 = 4.0;

	ArgDataList dl = { &v0, &v1, &v2, &v3 };
	ArgTypeList tl = { &atu_double, &atu_double, &atu_double, &atu_double };

	ArgumentInfo arginfo = CurrABI::get_argumentinfo(tl);
	CurrABI::create_function_caller(jfc, &callerX4, arginfo, dl);
}

void Call_2(JitFuncCreater &jfc)
{
	const unsigned int argn = 10;

	ArgDataList dl;
	ArgTypeList tl;
	uint64_t arr[argn];

	for (uint64_t i = 0; i < argn; i++) {
		arr[i] = i + 1;
		dl.push_back(&arr[i]);
		tl.push_back(&atu_uint64);
	}

	ArgumentInfo arginfo = CurrABI::get_argumentinfo(tl);
	CurrABI::create_function_caller(jfc, &callerN10, arginfo, dl);
}

void Call_3(JitFuncCreater &jfc)
{
	const unsigned int argn = 8;

	ArgDataList dl;
	ArgTypeList tl;
	uint64_t arr[argn];

	for (uint64_t i = 0; i < argn; i++) {
		if (i % 2 == 0) {
			arr[i] = i + 1;
			tl.push_back(&atu_uint64);
		}
		else {
			arr[i] = convert_uint64((i + 1) * 1.0);
			tl.push_back(&atu_double);
		}
		dl.push_back(&arr[i]);
	}

	ArgumentInfo arginfo = CurrABI::get_argumentinfo(tl);
	CurrABI::create_function_caller(jfc, &callerNX, arginfo, dl);
}


void Call_4(JitFuncCreater &jfc)
{
	long double ld = 2.5;

	ArgumentInfo arginfo = CurrABI::get_argumentinfo({ &atu_ldouble });
	CurrABI::create_function_caller(jfc, &function_1, arginfo, { &ld });
}

void Call_5(JitFuncCreater &jfc)
{
	int i = 5;
	float f = 6.75;
	double d = 8.0;
	long double ld = 2.5;

	ArgumentInfo arginfo = CurrABI::get_argumentinfo({ &atu_int, &atu_float, &atu_double, &atu_ldouble });
	CurrABI::create_function_caller(jfc, &function_2, arginfo, { &i, &f, &d, &ld });
}

int main(int argc, char *argv[])
{
	Call(Call_1);
	Call(Call_2);
	Call(Call_3);
	Call(Call_4);
	Call(Call_5);

	printf("Done.\n");

	return 0;
}
