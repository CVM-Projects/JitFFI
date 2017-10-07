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

enum ArgType
{
	AT_Int,
	AT_Double,
	AT_Memory,
};

struct TypeListUnit
{
	uint post;
	uint size;
	ArgType type;
};

struct TypeList
{
	uint num;
	TypeListUnit *data;
};

void pass_struct(JitFuncCallerCreater &jfcc, void *t, size_t size, const TypeList &typelist)
{
#if (defined(_WIN64))
	if (need_pass_by_pointer(size)) {
		jfcc.init_addarg_count(0, 0, 1);

		push_copy(jfcc, t, size);

		OpCode_x64::mov_rbx_rsp(jfcc.data());

		jfcc.add_int_rbx();
	}
	else {
		jfcc.init_addarg_count(1, 0, 0);

		jfcc.add_int(convert_uint64(t, size));
	}
#elif (defined(__x86_64__))
	if (need_pass_by_memory(size)) {
		jfcc.init_addarg_count(0, 0, 1);

		for (uint i = typelist.num; i != 0; --i) {
			auto &e = typelist.data[i - 1];
			jfcc.push(convert_uint64(t + e.post, e.size));
		}
	}
	else {
		uint int_count = 0;
		uint double_count = 0;
		for (uint i = 0; i != typelist.num; ++i) {
			switch (typelist.data[i].type) {
			case AT_Int:
				int_count++;
				break;
			case AT_Double:
				double_count++;
				break;
			default:
				assert(false);
			}
		}

		jfcc.init_addarg_count(int_count, double_count);

		for (uint i = typelist.num; i != 0; --i) {
			auto &e = typelist.data[i - 1];
			switch (e.type) {
			case AT_Int:
				jfcc.add_int(convert_uint64(t + e.post, e.size));
				break;
			case AT_Double:
				jfcc.add_double(convert_uint64(t + e.post, e.size));
				break;
			default:
				assert(false);
			}
		}
	}
#endif
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
	JitFuncCallerCreater jfcc(jfc, &print_struct1);

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
	JitFuncCallerCreater jfcc(jfc, &print_struct2);

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
	JitFuncCallerCreater jfcc(jfc, &print_struct3);

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

int main(int argc, char *argv[])
{
	Call(Call_1);
	Call(Call_2);
	Call(Call_3);
}
