// test2.cpp
// This file can run on the x86-64 system.
#include "testsuite.h"

typedef struct {
	uint8_t d0;
} type1;

void print_struct1(type1 t)
{
	print(t.d0);
}

typedef struct {
	uint8_t d0;
	double d1;
} type2;

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

void print_struct6(type6 t)
{
	print(t.d0);
	print(t.d1);
	print(t.d2);
}

using CallFunc = std::function<void(JitFuncCallerCreater &)>;

void Call_SX(JitFuncCallerCreater &jfcc, CallFunc f)
{
	jfcc.push_rbx();

	byte &v = jfcc.sub_rsp_unadjusted();

	f(jfcc);

	jfcc.call();

	jfcc.add_rsp();
	jfcc.adjust_sub_rsp(v);

	jfcc.pop_rbx();
	jfcc.ret();
}

void Call_1(JitFuncCreater &jfc)
{
	using namespace JitFFI::CurrABI;

	JitFuncCallerCreaterPlatform jfcc(jfc, &print_struct1);

	auto f = [](JitFuncCallerCreater &jfcc) {
		auto size = sizeof(type1);

		type1 t{ 1 };

		TypeListUnit typelist[] = {
			{ (uint)((byte*)&t.d0 - (byte*)&t), sizeof(t.d0), AT_Int },
		};

		pass_struct(jfcc, &t, size, TypeList{ 1, typelist });
	};

	Call_SX(jfcc, f);
}

void Call_2(JitFuncCreater &jfc)
{
	using namespace JitFFI::CurrABI;

	JitFuncCallerCreaterPlatform jfcc(jfc, &print_struct2);

	auto f = [](JitFuncCallerCreater &jfcc) {
		auto size = sizeof(type2);

		type2 t{ 1, 2.5789763 };

		TypeListUnit typelist[] = {
			{ (uint)((byte*)&t.d0 - (byte*)&t), sizeof(t.d0), AT_Int },
			{ (uint)((byte*)&t.d1 - (byte*)&t), sizeof(t.d1), AT_Double },
		};

		pass_struct(jfcc, &t, size, TypeList{ 2, typelist });
	};

	Call_SX(jfcc, f);
}

void Call_3(JitFuncCreater &jfc)
{
	using namespace JitFFI::CurrABI;

	JitFuncCallerCreaterPlatform jfcc(jfc, &print_struct3);

	auto f = [](JitFuncCallerCreater &jfcc) {
		size_t size = sizeof(type3);

		type3 t = { 1, 2.5789763, 3 };

		TypeListUnit typelist[] = {
			{ (uint)((byte*)&t.d0 - (byte*)&t), sizeof(t.d0), AT_Int },
			{ (uint)((byte*)&t.d1 - (byte*)&t), sizeof(t.d1), AT_Double },
			{ (uint)((byte*)&t.d2 - (byte*)&t), sizeof(t.d2), AT_Int },
		};


		pass_struct(jfcc, &t, size, TypeList{ 3, typelist });
	};

	Call_SX(jfcc, f);
}

void Call_4(JitFuncCreater &jfc)
{
	using namespace JitFFI::CurrABI;

	JitFuncCallerCreaterPlatform jfcc(jfc, &print_struct4);

	auto f = [](JitFuncCallerCreater &jfcc) {
		size_t size = sizeof(type4);

		type4 t = { 1, 0x22222222, 2.5789763 };

		TypeListUnit typelist[] = {
			{ (uint)((byte*)&t.d0 - (byte*)&t), sizeof(t.d0), AT_Int },
			{ (uint)((byte*)&t.d1 - (byte*)&t), sizeof(t.d1), AT_Int },
			{ (uint)((byte*)&t.d2 - (byte*)&t), sizeof(t.d2), AT_Double },
		};


		pass_struct(jfcc, &t, size, TypeList{ 3, typelist });
	};

	Call_SX(jfcc, f);
}

void Call_5(JitFuncCreater &jfc)
{
	using namespace JitFFI::CurrABI;

	JitFuncCallerCreaterPlatform jfcc(jfc, &print_struct5);

	auto f = [](JitFuncCallerCreater &jfcc) {
		size_t size = sizeof(type5);

		type5 t = { 1, 2 };

		TypeListUnit typelist[] = {
			{ (uint)((byte*)&t.d0 - (byte*)&t), sizeof(t.d0), AT_Int },
			{ (uint)((byte*)&t.d1 - (byte*)&t), sizeof(t.d1), AT_Int },
		};


		pass_struct(jfcc, &t, size, TypeList{ 2, typelist });
	};

	Call_SX(jfcc, f);
}

void Call_6(JitFuncCreater &jfc)
{
	using namespace JitFFI::CurrABI;

	JitFuncCallerCreaterPlatform jfcc(jfc, &print_struct6);

	auto f = [](JitFuncCallerCreater &jfcc) {
		size_t size = sizeof(type6);

		type6 t = { 1, 2, 3 };

		TypeListUnit typelist[] = {
			{ (uint)((byte*)&t.d0 - (byte*)&t), sizeof(t.d0), AT_Int },
			{ (uint)((byte*)&t.d1 - (byte*)&t), sizeof(t.d1), AT_Int },
			{ (uint)((byte*)&t.d2 - (byte*)&t), sizeof(t.d2), AT_Int },
		};


		pass_struct(jfcc, &t, size, TypeList{ 3, typelist });
	};

	Call_SX(jfcc, f);
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
