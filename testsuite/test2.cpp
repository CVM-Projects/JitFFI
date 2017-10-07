// test2.cpp
// This file can run on the x86-64 system
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
	AT_Unknown,
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

class NewStruct
{
public:
	bool push(byte *dat, uint size) {
		assert(size != 0);
		if (count + size > 8)
			return false;
		if (size == 4) {
			if (count == 0) {
				write_uint32(0, dat);
				count = 4;
			}
			else if (count <= 4) {
				write_uint32(1, dat);
				count = 8;
			}
			else {
				return false;
			}
		}
		else if (size == 2) {
			if (count == 0) {
				write_uint16(0, dat);
				count = 2;
			}
			else if (count <= 2) {
				write_uint16(1, dat);
				count = 4;
			}
			else if (count <= 4) {
				write_uint16(2, dat);
				count = 6;
			}
			else if (count <= 6) {
				write_uint16(3, dat);
				count = 8;
			}
			else {
				return false;
			}
		}
		else if (size == 1) {
			write_byte(count, dat);
			count += 1;
		}
		else {
			assert(false);
			return false;
		}
		return true;
	}

	void write_uint32(uint i, byte *dat) {
		((uint32_t*)&_data)[i] = *(uint32_t*)dat;
	}
	void write_uint16(uint i, byte *dat) {
		((uint16_t*)&_data)[i] = *(uint16_t*)dat;
	}
	void write_byte(uint i, byte *dat) {
		((byte*)&_data)[i] = *dat;
	}

	void clear() {
		count = 0;
		_data = 0;
	}

	uint64_t data() const {
		return _data;
	}

private:
	uint count = 0;
	uint64_t _data = 0;
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
		uint memory_count = 0;

		NewStruct ns;

		ArgType c_type = AT_Unknown;

		std::vector<uint64_t> vec;
		std::vector<ArgType> vectype;

		auto add_count = [&](ArgType type) {
			switch (type) {
			case AT_Int:
				int_count++;
				break;
			case AT_Double:
				double_count++;
				break;
			case AT_Memory:
				memory_count++;
				break;
			default:
				assert(false);
			}
		};

		auto push_and_clear = [&]() {
			vec.push_back(ns.data());
			vectype.push_back(c_type);
			ns.clear();
			add_count(c_type);
			c_type = AT_Unknown;
		};

		for (uint i = 0; i != typelist.num; ++i) {
			auto &e = typelist.data[i];
			ArgType type = e.type;
			uint size = e.size;
			byte *p = (byte*)t + e.post;

			assert(size <= 8);

			if (size >= 8) {
				if (c_type != AT_Unknown) {
					push_and_clear();
				}
				vec.push_back(convert_uint64(p, size));
				vectype.push_back(type);
				add_count(type);
			}
			else {
				if (ns.push(p, size)) {
					switch (type) {
					case AT_Int:
						if (c_type == AT_Double || c_type == AT_Unknown) {
							c_type = AT_Int;
						}
						break;
					case AT_Double:
						if (c_type == AT_Unknown) {
							c_type = AT_Double;
						}
						break;
					case AT_Memory:
						c_type = AT_Memory;
					}
				}
				else {
					push_and_clear();
					i--;
					continue;
				}
			}
		}

		if (c_type != AT_Unknown) {
			push_and_clear();
		}

		jfcc.init_addarg_count(int_count, double_count, memory_count);

		for (uint i = vec.size(); i != 0; --i) {
			uint64_t &e = vec[i - 1];
			switch (vectype[i - 1]) {
			case AT_Int:
				jfcc.add_int(convert_uint64(e));
				break;
			case AT_Double:
				jfcc.add_double(convert_uint64(e));
				break;
			case AT_Memory:
				jfcc.push(convert_uint64(e));
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

void Call_4(JitFuncCreater &jfc)
{
	JitFuncCallerCreater jfcc(jfc, &print_struct4);

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
//
//struct typex
//{
//	uint8_t a, b, c, d;
//	uint32_t e;
//	//double x;
//};
//
//void print_structx(typex x)
//{
//	print(x.a);
//	print(x.b);
//	print(x.c);
//	print(x.d);
//	//print(x.f);
//	print(x.e);
//	//print(x.x);
//}
//
//typedef struct {
//	uint8_t d0;
//	uint8_t d1;
//	float f0;
//	float f1;
//	uint8_t d2;
//	//float f2;
//} typeX1;
//void print_structX1(typeX1 t) {
//	print(convert_uint64(t.d0));
//	print(convert_uint64(t.d1));
//	print(convert_uint64(t.f0));
//	print(convert_uint64(t.f1));
//	//print(convert_uint64(t.f2));
//	print(convert_uint64(t.d2));
//}

//void Call_5(JitFuncCreater &jfc)
//{
//	OpCode_x64::sub_rsp_byte(jfc, 0x8);
//	OpCode_x64::mov_rdi_uint64(jfc, 0x123456789abcdef0);
//	OpCode_x64::mov_rsi_uint64(jfc, 0x0fedcba987654321);
//	OpCode_x64::mov_rax(jfc, 0x1122334455667788);
//	OpCode_x64::movq_xmm0_rax(jfc);
//	OpCode_x64::mov_rax(jfc, 0x99aabbccddeeff00);
//	OpCode_x64::movq_xmm1_rax(jfc);
//	OpCode::call_func(jfc, &print_structX1);
//	OpCode_x64::add_rsp_byte(jfc, 0x8);
//	OpCode::ret(jfc);
//}

int main(int argc, char *argv[])
{
	Call(Call_1);
	Call(Call_2);
	Call(Call_3);
	Call(Call_4);
	//Call(Call_5);
}
