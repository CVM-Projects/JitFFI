// test2.cpp
// This file can run on the x86-64 system.
#include "testsuite.h"

typedef struct {
	uint8_t d0;
} type1;

const ArgTypeUnit atu_type1(sizeof(type1), alignof(type1), { &atu_uint8 });

void print_struct1(type1 t)
{
	print(t.d0);
}

typedef struct {
	uint8_t d0;
	double d1;
} type2;

const ArgTypeUnit atu_type2(sizeof(type2), alignof(type2), { &atu_uint8, &atu_double });

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

const ArgTypeUnit atu_type3(sizeof(type3), alignof(type3), { &atu_uint8, &atu_double, &atu_uint32 });

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

const ArgTypeUnit atu_type4(sizeof(type4), alignof(type4), { &atu_uint8, &atu_uint8, &atu_double });

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

const ArgTypeUnit atu_type5(sizeof(type5), alignof(type5), { &atu_uint64, &atu_uint64 });

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

const ArgTypeUnit atu_type6(sizeof(type6), alignof(type6), { &atu_uint64, &atu_uint64, &atu_uint64 });

void print_struct6(type6 t)
{
	print(t.d0);
	print(t.d1);
	print(t.d2);
}


typedef struct {
	type1 d0;
	type1 d1;
} type7;

const ArgTypeUnit atu_type7(sizeof(type7), alignof(type7), { &atu_type1, &atu_type1 });

void print_struct7(type7 t)
{
	print_struct1(t.d0);
	print_struct1(t.d1);
}

typedef struct {
	type1 d0;
	type2 d1;
} type8;

const ArgTypeUnit atu_type8(sizeof(type8), alignof(type8), { &atu_type1, &atu_type2 });

void print_struct8(type8 t)
{
	print_struct1(t.d0);
	print_struct2(t.d1);
}

typedef struct {
	char d[7];
} type9;

const ArgTypeUnit atu_type9(sizeof(type9), alignof(type9), { &atu_char, &atu_char, &atu_char, &atu_char, &atu_char, &atu_char, &atu_char });

void print_struct9(type9 t)
{
	printf("(%d %d %d %d %d %d %d)\n", t.d[0], t.d[1], t.d[2], t.d[3], t.d[4], t.d[5], t.d[6]);
}

typedef struct {
	type9 d0;
	type9 d1;
} type10;

const ArgTypeUnit atu_type10(sizeof(type10), alignof(type10), { &atu_type9, &atu_type9 });

void print_struct10(type10 t)
{
	print_struct9(t.d0);
	print_struct9(t.d1);
}

template <typename _FTy, typename _Ty>
void Call_X(JitFuncCreater &jfc, _FTy &func, const ArgTypeUnit &atu, const _Ty &t)
{
	ArgTypeList tl{ &atu };
	ArgDataList dl{ &t };
	CurrABI::create_function_caller(jfc, &func, CurrABI::get_argumentinfo(tl), dl);
}

void Call_1(JitFuncCreater &jfc)
{
	type1 t{ 1 };

	Call_X(jfc, print_struct1, atu_type1, t);
}

void Call_2(JitFuncCreater &jfc)
{
	type2 t{ 1, 2.5789763 };

	Call_X(jfc, print_struct2, atu_type2, t);
}

void Call_3(JitFuncCreater &jfc)
{
	type3 t = { 1, 2.5789763, 3 };

	Call_X(jfc, print_struct3, atu_type3, t);
}

void Call_4(JitFuncCreater &jfc)
{
	type4 t = { 1, 0x22, 2.5789763 };

	Call_X(jfc, print_struct4, atu_type4, t);
}

void Call_5(JitFuncCreater &jfc)
{
	type5 t = { 1, 2 };

	Call_X(jfc, print_struct5, atu_type5, t);
}

void Call_6(JitFuncCreater &jfc)
{
	type6 t = { 1, 2, 3 };

	Call_X(jfc, print_struct6, atu_type6, t);
}

void Call_7(JitFuncCreater &jfc)
{
	type7 t = { type1{ 1 }, type1{ 2 } };

	Call_X(jfc, print_struct7, atu_type7, t);
}

void Call_8(JitFuncCreater &jfc)
{
	type8 t = { type1{ 1 }, type2{ 2, 3 } };

	Call_X(jfc, print_struct8, atu_type8, t);
}

void Call_9(JitFuncCreater &jfc)
{
	type9 t = { 1, 2, 3, 4, 5, 6, 7 };

	Call_X(jfc, print_struct9, atu_type9, t);
}

void Call_10(JitFuncCreater &jfc)
{
	type10 t = { type9 { 1, 2, 3, 4, 5, 6, 7 }, type9{ 8, 9, 10, 11, 12, 13, 14 } };

	Call_X(jfc, print_struct10, atu_type10, t);
}

int main(int argc, char *argv[])
{
	Call(Call_1);
	Call(Call_2);
	Call(Call_3);
	Call(Call_4);
	Call(Call_5);
	Call(Call_6);
	Call(Call_7);
	Call(Call_8);
	Call(Call_9);
	Call(Call_10);
}
