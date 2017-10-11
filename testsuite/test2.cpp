// test2.cpp
// This file can run on the x86-64 system.
#include "testsuite.h"

typedef struct {
	uint8_t d0;
} type1;

const ArgTypeUnit& get_atu_type1()
{
	static type1 t;
	static ArgTypeUnit atu(sizeof(type1), {
		{ get_post(&t, &t.d0), &atu_uint8 }
	});

	return atu;
}

void print_struct1(type1 t)
{
	print(t.d0);
}

typedef struct {
	uint8_t d0;
	double d1;
} type2;

const ArgTypeUnit& get_atu_type2()
{
	static type2 t;
	static ArgTypeUnit atu(sizeof(type2), {
		{ get_post(&t, &t.d0), &atu_uint8 },
		{ get_post(&t, &t.d1), &atu_double }
	});

	return atu;
}

void print_struct2(type2 t)
{
	print(t.d0);
	print(t.d1);
}

typedef struct {
	uint8_t d0;
	double d1;
	uint32_t d2;
} type3;

const ArgTypeUnit& get_atu_type3()
{
	static type3 t;
	static ArgTypeUnit atu(sizeof(type3), {
		{ get_post(&t, &t.d0), &atu_uint8 },
		{ get_post(&t, &t.d1), &atu_double },
		{ get_post(&t, &t.d2), &atu_uint32 }
	});

	return atu;
}

void print_struct3(type3 t)
{
	print(t.d0);
	print(t.d1);
	print(t.d2);
}

typedef struct {
	uint8_t d0;
	uint8_t d1;
	double d2;
} type4;

const ArgTypeUnit& get_atu_type4()
{
	static type4 t;
	static ArgTypeUnit atu(sizeof(type4), {
		{ get_post(&t, &t.d0), &atu_uint8 },
		{ get_post(&t, &t.d1), &atu_uint8 },
		{ get_post(&t, &t.d2), &atu_double }
	});

	return atu;
}

void print_struct4(type4 t)
{
	print(t.d0);
	print(t.d1);
	print(t.d2);
}

typedef struct {
	uint64_t d0;
	uint64_t d1;
} type5;

const ArgTypeUnit& get_atu_type5()
{
	static type5 t;
	static ArgTypeUnit atu(sizeof(type5), {
		{ get_post(&t, &t.d0), &atu_uint64 },
		{ get_post(&t, &t.d1), &atu_uint64 }
	});

	return atu;
}

void print_struct5(type5 t)
{
	print(t.d0);
	print(t.d1);
}

typedef struct {
	uint64_t d0;
	uint64_t d1;
	uint64_t d2;
} type6;

const ArgTypeUnit& get_atu_type6()
{
	static type6 t;
	static ArgTypeUnit atu(sizeof(type6), {
		{ get_post(&t, &t.d0), &atu_uint64 },
		{ get_post(&t, &t.d1), &atu_uint64 },
		{ get_post(&t, &t.d2), &atu_uint64 }
	});

	return atu;
}

void print_struct6(type6 t)
{
	print(t.d0);
	print(t.d1);
	print(t.d2);
}

void Call_1(JitFuncCreater &jfc)
{
	type1 t{ 1 };

	CurrABI::create_function_caller(jfc, &print_struct1, { &t }, { &get_atu_type1() });
}

void Call_2(JitFuncCreater &jfc)
{
	type2 t{ 1, 2.5789763 };

	CurrABI::create_function_caller(jfc, &print_struct2, { &t }, { &get_atu_type2() });
}

void Call_3(JitFuncCreater &jfc)
{
	type3 t = { 1, 2.5789763, 3 };

	CurrABI::create_function_caller(jfc, &print_struct3, { &t }, { &get_atu_type3() });
}

void Call_4(JitFuncCreater &jfc)
{
	type4 t = { 1, 0x22222222, 2.5789763 };

	CurrABI::create_function_caller(jfc, &print_struct4, { &t }, { &get_atu_type4() });
}

void Call_5(JitFuncCreater &jfc)
{
	type5 t = { 1, 2 };

	CurrABI::create_function_caller(jfc, &print_struct5, { &t }, { &get_atu_type5() });
}

void Call_6(JitFuncCreater &jfc)
{
	type6 t = { 1, 2, 3 };

	CurrABI::create_function_caller(jfc, &print_struct6, { &t }, { &get_atu_type6() });
}

int main(int argc, char *argv[])
{
	Call(Call_1);
	Call(Call_2);
	Call(Call_3);
	Call(Call_4);
	Call(Call_5);
	Call(Call_6);
}
