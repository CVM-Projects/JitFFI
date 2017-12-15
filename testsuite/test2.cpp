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

typedef struct {
	char d0;
	float d1;
} type11a;

typedef struct {
	float d0;
	char d1;
} type11b;

typedef struct {
	type11a d0;
	type11b d1;
} type11;

const ArgTypeUnit atu_type11a(sizeof(type11a), alignof(type11a), { &atu_char, &atu_float });
const ArgTypeUnit atu_type11b(sizeof(type11b), alignof(type11b), { &atu_float, &atu_char });
const ArgTypeUnit atu_type11(sizeof(type11), alignof(type11), { &atu_type11a, &atu_type11b });

void print_struct11a(type11a t)
{
	print(t.d0);
	printf("0x%llX\n", *((uint16_t*)&t.d1));
}

void print_struct11b(type11b t)
{
	printf("0x%llX\n", *((uint16_t*)&t.d0));
	print(t.d1);
}

void print_struct11(type11 t)
{
	print_struct11a(t.d0);
	print_struct11b(t.d1);
}

typedef struct {
	float d0;
	float d1;
} type12a;

typedef struct {
	float d0;
	float d1;
} type12b;

typedef struct {
	type12a d0;
	type12b d1;
} type12;

const ArgTypeUnit atu_type12a(sizeof(type12a), alignof(type12a), { &atu_float, &atu_float });
const ArgTypeUnit atu_type12b(sizeof(type12b), alignof(type12b), { &atu_float, &atu_float });
const ArgTypeUnit atu_type12(sizeof(type12), alignof(type12), { &atu_type12a, &atu_type12b });

void print_struct12a(type12a t)
{
	printf("0x%llX\n", *((uint16_t*)&t.d0));
	printf("0x%llX\n", *((uint16_t*)&t.d1));
}

void print_struct12b(type12b t)
{
	printf("0x%llX\n", *((uint16_t*)&t.d0));
	printf("0x%llX\n", *((uint16_t*)&t.d1));
}

void print_struct12(type12 t)
{
	print_struct12a(t.d0);
	print_struct12b(t.d1);
}

typedef struct {
	uint16_t d0;
} type13a;

typedef struct {
	float d0;
	float d1;
	float d2;
} type13b;

typedef struct {
	type13a d0;
	type13b d1;
} type13;

const ArgTypeUnit atu_type13a(sizeof(type13a), alignof(type13a), { &atu_uint16 });
const ArgTypeUnit atu_type13b(sizeof(type13b), alignof(type13b), { &atu_float, &atu_float, &atu_float });
const ArgTypeUnit atu_type13(sizeof(type13), alignof(type13), { &atu_type13a, &atu_type13b });

void print_struct13a(type13a t)
{
	printf("0x%llX\n", *((uint16_t*)&t.d0));
}

void print_struct13b(type13b t)
{
	printf("0x%llX\n", *((uint16_t*)&t.d0));
	printf("0x%llX\n", *((uint16_t*)&t.d1));
	printf("0x%llX\n", *((uint16_t*)&t.d2));
}

void print_struct13(type13 t)
{
	print_struct13a(t.d0);
	print_struct13b(t.d1);
}

typedef struct {
	char d0;
	char d1;
	uint32_t d2;
	char d3;
} type14;

const ArgTypeUnit atu_type14(sizeof(type14), alignof(type14), { &atu_char, &atu_char, &atu_uint32, &atu_char });

void print_struct14(type14 t)
{
	printf("(%d %d %d %d)\n", t.d0, t.d1, t.d2, t.d3);
}

template <typename _FTy, typename _Ty>
auto Call_X(JitFuncCreater &jfc, _FTy *func, const ArgTypeUnit &atu, const _Ty &t)
{
	ArgTypeList tl{ &atu };
	ArgDataList dl{ &t };
	return CurrABI::Compile(jfc, CurrABI::GetArgInfo(atu_void, tl), func, dl);
}

auto Call_1(JitFuncCreater &jfc)
{
	type1 t{ 1 };

	return Call_X(jfc, print_struct1, atu_type1, t);
}

auto Call_2(JitFuncCreater &jfc)
{
	type2 t{ 1, 2.5789763 };

	return Call_X(jfc, print_struct2, atu_type2, t);
}

auto Call_3(JitFuncCreater &jfc)
{
	type3 t = { 1, 2.5789763, 3 };

	return Call_X(jfc, print_struct3, atu_type3, t);
}

auto Call_4(JitFuncCreater &jfc)
{
	type4 t = { 1, 0x22, 2.5789763 };

	return Call_X(jfc, print_struct4, atu_type4, t);
}

auto Call_5(JitFuncCreater &jfc)
{
	type5 t = { 1, 2 };

	return Call_X(jfc, print_struct5, atu_type5, t);
}

auto Call_6(JitFuncCreater &jfc)
{
	type6 t = { 1, 2, 3 };

	return Call_X(jfc, print_struct6, atu_type6, t);
}

auto Call_7(JitFuncCreater &jfc)
{
	type7 t = { type1{ 1 }, type1{ 2 } };

	return Call_X(jfc, print_struct7, atu_type7, t);
}

auto Call_8(JitFuncCreater &jfc)
{
	type8 t = { type1{ 1 }, type2{ 2, 3 } };

	return Call_X(jfc, print_struct8, atu_type8, t);
}

auto Call_9(JitFuncCreater &jfc)
{
	type9 t = { 1, 2, 3, 4, 5, 6, 7 };

	return Call_X(jfc, print_struct9, atu_type9, t);
}

auto Call_10(JitFuncCreater &jfc)
{
	type10 t = { type9 { 1, 2, 3, 4, 5, 6, 7 }, type9{ 8, 9, 10, 11, 12, 13, 14 } };

	return Call_X(jfc, print_struct10, atu_type10, t);
}

float convert_f32(int v)
{
	return *(float*)(&v);
}

auto Call_11(JitFuncCreater &jfc)
{
	type11 t = { type11a { 1, convert_f32(2) }, type11b { convert_f32(3), 4 } };

	return Call_X(jfc, print_struct11, atu_type11, t);
}

auto Call_12(JitFuncCreater &jfc)
{
	type12 t = { type12a { convert_f32(1), convert_f32(2) }, type12b { convert_f32(3), convert_f32(4) } };

	return Call_X(jfc, print_struct12, atu_type12, t);
}

auto Call_13(JitFuncCreater &jfc)
{
	type13 t = { type13a { 1 }, type13b { convert_f32(2), convert_f32(3), convert_f32(4) } };

	return Call_X(jfc, print_struct13, atu_type13, t);
}

auto Call_14(JitFuncCreater &jfc)
{
	type14 t = { 1, 2, 3, 4 };

	return Call_X(jfc, print_struct14, atu_type14, t);
}

#include "../source/jitffi-def.h"

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
	Call(Call_11);
	Call(Call_12);
	Call(Call_13);
	Call(Call_14);
}
