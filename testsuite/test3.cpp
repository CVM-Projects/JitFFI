// test3.cpp
// This file can run on the x86-64 system.
#include "testsuite.h"

typedef struct {
	uint32_t d0;
	uint32_t d1;
} type1;

const ArgTypeUnit atu_type1(sizeof(type1), alignof(type1), { &atu_uint32, &atu_uint32 });

type1 print_struct1(type1 t1, type1 t2)
{
	print(t1.d0);
	print(t1.d1);
	print(t2.d0);
	print(t2.d1);

	return type1{ 5, 6 };
}

typedef struct {
	uint64_t d0;
	uint64_t d1;
} type2;

const ArgTypeUnit atu_type2(sizeof(type2), alignof(type2), { &atu_uint64, &atu_uint64 });

type2 print_struct2(type2 t1, type2 t2)
{
	print(t1.d0);
	print(t1.d1);
	print(t2.d0);
	print(t2.d1);

	return type2{ 5, 6 };
}

typedef struct {
	uint64_t d0;
	uint64_t d1;
	uint64_t d2;
} type3;

const ArgTypeUnit atu_type3(sizeof(type3), alignof(type3), { &atu_uint64, &atu_uint64, &atu_uint64 });

type3 print_struct3(type3 t1, type3 t2)
{
	print(t1.d0);
	print(t1.d1);
	print(t1.d2);
	print(t2.d0);
	print(t2.d1);
	print(t2.d2);

	return type3{ 8, 9, 10 };
}

typedef struct {
	uint64_t d0;
	double d1;
} type4;

const ArgTypeUnit atu_type4(sizeof(type4), alignof(type4), { &atu_uint64, &atu_double });

type4 print_struct4(type4 t1, type4 t2)
{
	print(t1.d0);
	print(t1.d1);
	print(t2.d0);
	print(t2.d1);

	return type4{ 5, 6.0 };
}

typedef struct {
	uint8_t d0;
	uint8_t d0x[7];
	//uint8_t d1;
	//uint8_t d1x[7];
	uint8_t d1;
} type5;

const ArgTypeUnit atu_type5(sizeof(type5), alignof(type5), {
	//&atu_uint64,
	&atu_uint8, &atu_uint8, &atu_uint8, &atu_uint8, &atu_uint8, &atu_uint8, &atu_uint8, &atu_uint8,
	&atu_uint8
});

type5 print_struct5(type5 t1, type5 t2)
{
	print(t1.d0);
	print(t1.d1);
	print(t2.d0);
	print(t2.d1);

	type5 r;
	r.d0 = 5;
	r.d0x[1] = 1;
	r.d1 = 6;

	return r;
}

template <typename _FTy, typename _Ty>
void Call_X(JitFuncCreater &jfc, _FTy &func, const ArgTypeUnit &atu, const _Ty &t1, const _Ty &t2)
{
	ArgTypeList tl{ &atu, &atu };
	ArgDataList dl{ &t1, &t2 };
	ArgumentInfo info = CurrABI::get_argumentinfo(atu, tl);
	CurrABI::create_function_caller(jfc, info, &func, dl);
}

template <typename _FTy>
void Call_Y(JitFuncCreater &jfc, _FTy &func, const ArgTypeUnit &atu)
{
	ArgTypeList tl{ &atu, &atu };
	ArgumentInfo info = CurrABI::get_argumentinfo(atu, tl);
	CurrABI::create_function_caller(jfc, info, &func);
}

void Call_1(JitFuncCreater &jfc)
{
	type1 t1{ 1, 2 };
	type1 t2{ 3, 4 };

	Call_X(jfc, print_struct1, atu_type1, t1, t2);
}

void Call_2(JitFuncCreater &jfc)
{
	type2 t1{ 1, 2 };
	type2 t2{ 3, 4 };

	Call_X(jfc, print_struct2, atu_type2, t1, t2);
}

void Call_3(JitFuncCreater &jfc)
{
	type3 t1{ 1, 2, 3 };
	type3 t2{ 4, 5, 6 };

	Call_X(jfc, print_struct3, atu_type3, t1, t2);
}

void Call_4(JitFuncCreater &jfc)
{
	type4 t1{ 1, 2 };
	type4 t2{ 3, 4 };

	Call_X(jfc, print_struct4, atu_type4, t1, t2);
}

void Call_5(JitFuncCreater &jfc)
{
	type5 t1;
	t1.d0 = 1;
	t1.d1 = 2;
	type5 t2;
	t2.d0 = 3;
	t2.d1 = 4;

	Call_X(jfc, print_struct5, atu_type5, t1, t2);
}

void Call_5y()
{
	auto f = Compile<void(void *, void **)>([](JitFuncCreater &jfc) { Call_Y(jfc, print_struct5, atu_type5); });

	type5 t1;
	t1.d0 = 1;
	t1.d1 = 2;
	type5 t2;
	t2.d0 = 3;
	t2.d1 = 4;

	void *dl[] = { &t1, &t2 };


	uint8_t *np = (uint8_t*)calloc(sizeof(type5) + 5, 1);
	np[sizeof(type5) + 0] = 0x11;
	np[sizeof(type5) + 1] = 0x22;
	np[sizeof(type5) + 2] = 0x33;
	np[sizeof(type5) + 3] = 0x44;
	np[sizeof(type5) + 4] = 0x55;

	f(np, dl);
	printf("<0x%016llX>\n", *(uint64_t*)np);
	print_struct5(*(type5*)np, *(type5*)np);
	for (unsigned int i = 0; i != sizeof(type5) + 5; ++i) {
		printf("%02X ", np[i]);
	}
	printf("\n");
}

double f6(double x1)
{
	printf("%lf\n", x1);
	return x1;
}

void Call_6()
{
	auto f = Compile<void(void *)>([](JitFuncCreater &jfc) {
		double v = 3.14;

		ArgumentInfo info = CurrABI::get_argumentinfo(atu_double, { &atu_double });
		CurrABI::create_function_caller(jfc, info, &f6, { &v });
	});

	double r = 0.5;

	f(&r);

	printf("%lf\n", r);
}

void Call_6y()
{

	auto f = Compile<void(void *, void **)>([](JitFuncCreater &jfc) {
		ArgumentInfo info = CurrABI::get_argumentinfo(atu_double, { &atu_double });
		CurrABI::create_function_caller(jfc, info, &f6);
	});

	double npL[3] = { 3.14, 0.0, 1.57 };
	void* dl[] = { npL + 0 };
	f(npL + 1, dl);
	printf("%lf\n", npL[1]);
}

#include "../source/jitffi-def.h"

int main()
{
	void *p = malloc(0xff);

	printf("===1===\n");
	Call(Call_1, p);
	printf("<0x%016llX>\n", *(uint64_t*)p);
	print_struct1(*(type1*)p, *(type1*)p);

	printf("===2===\n");
	Call(Call_2, p);
	printf("<0x%016llX>\n", *(uint64_t*)p);
	print_struct2(*(type2*)p, *(type2*)p);

	printf("===3===\n");
	Call(Call_3, p);
	printf("<0x%016llX>\n", *(uint64_t*)p);
	print_struct3(*(type3*)p, *(type3*)p);

	printf("===4===\n");
	Call(Call_4, p);
	printf("<0x%016llX>\n", *(uint64_t*)p);
	print_struct4(*(type4*)p, *(type4*)p);

	printf("===5===\n");
	uint8_t *np = (uint8_t*)calloc(sizeof(type5) + 5, 1);
	np[sizeof(type5) + 0] = 0x11;
	np[sizeof(type5) + 1] = 0x22;
	np[sizeof(type5) + 2] = 0x33;
	np[sizeof(type5) + 3] = 0x44;
	np[sizeof(type5) + 4] = 0x55;
	Call(Call_5, np);
	printf("<0x%016llX>\n", *(uint64_t*)np);
	print_struct5(*(type5*)np, *(type5*)np);
	for (unsigned int i = 0; i != sizeof(type5) + 5; ++i) {
		printf("%02X ", np[i]);
	}
	printf("\n");

	printf("===5y===\n");
	Call_5y();

	printf("===6===\n");
	Call_6();

	printf("===6y===\n");
	Call_6y();


	//Call(Call_NNN);

	printf("===E===\n");
	printf("<0x%016llX>\n", (size_t)p);
}
