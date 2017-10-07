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

void caller()
{
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

void Call_1(JitFuncCreater &jfc) {
	const unsigned int argn = 4;

	JitFuncCallerCreater jfcc(jfc, &callerX4);
	jfcc.init_addarg_count(0, argn);

	byte &v = jfcc.sub_rsp_unadjusted();

	for (double v = argn; v != 0; --v)
		jfcc.add_double(convert_uint64(v));

	jfcc.adjust_sub_rsp(v);

	jfcc.call();
	jfcc.add_rsp();
	jfcc.ret();
}

void Call_2(JitFuncCreater &jfc) {
	const unsigned int argn = 10;

	JitFuncCallerCreater jfcc(jfc, &callerN10);
	jfcc.init_addarg_count(argn, 0);

	byte &v = jfcc.sub_rsp_unadjusted();

	for (int v = argn; v != 0; --v)
		jfcc.add_int(convert_uint64(v));

	jfcc.adjust_sub_rsp(v);

	jfcc.call();
	jfcc.add_rsp();
	jfcc.ret();
}

void Call_3(JitFuncCreater &jfc) {
	JitFuncCallerCreater jfcc(jfc, &callerNX);
	jfcc.init_addarg_count(4, 4);

	byte &v = jfcc.sub_rsp_unadjusted();

	jfcc.add_double(convert_uint64(8.0));
	jfcc.add_int(convert_uint64(7));
	jfcc.add_double(convert_uint64(6.0));
	jfcc.add_int(convert_uint64(5));
	jfcc.add_double(convert_uint64(4.0));
	jfcc.add_int(convert_uint64(3));
	jfcc.add_double(convert_uint64(2.0));
	jfcc.add_int(convert_uint64(1));

	jfcc.adjust_sub_rsp(v);

	jfcc.call();
	jfcc.add_rsp();
	jfcc.ret();
}

struct Point
{
	uint64_t x;
	uint64_t y;
};

Point *global_p;

void print(uint64_t x)
{
	printf("0x%llX ", x);
}

void print_Point(Point p)
{
	//print(p.x);
	//print(p.y);
	printf("(0x%llX, 0x%llX)\n", p.x, p.y);
	p.x++;
	p.y++;
}

void print_int(int64 v)
{
	printf("0x%I64X\n", v);
}

uint64_t new_malloc(uint64_t size)
{
	return (uint64_t)malloc(size);
}

void new_free(uint64_t address)
{
	free((void*)address);
}


void print_PointX(Point *p)
{
	printf("(0x%llX, 0x%llX)\n", p->x, p->y);
	p->x++;
	p->y++;
}

void print_PointN6(Point p0, Point p1, Point p2, Point p3, Point p4, Point p5)
{
	print_Point(p0);
	print_Point(p1);
	print_Point(p2);
	print_Point(p3);
	print_Point(p4);
	print_Point(p5);
}

void Call_4X(JitFuncCreater &jfc) {
	global_p = new Point{ 5, 6 };
	Point *p = global_p;

	OpCode_x64::sub_rsp_byte(jfc, 0x28);

	OpCode_win64::add_int0(jfc, sizeof(Point));
	OpCode::call_func(jfc, &new_malloc);

	OpCode_x64::mov_rbx_rax(jfc);

	OpCode_x64::mov_rcx_rax(jfc);
	OpCode_x64::mov_rdx_uint64(jfc, (uint64_t)global_p);
	OpCode_x64::mov_r8d_uint32(jfc, sizeof(Point));
	OpCode::call_func(jfc, &memcpy);

	OpCode_x64::mov_rcx_rax(jfc);
	OpCode::call_func(jfc, &print_Point);

	OpCode_x64::mov_rcx_rbx(jfc);
	OpCode::call_func(jfc, &new_free);

	OpCode_x64::add_rsp_byte(jfc, 0x28);
	OpCode::ret(jfc);
}

void Call_4(JitFuncCreater &jfc) {

	const size_t argn = 6;

	JitFuncCallerCreater jfcc(jfc, &print_PointN6);

	jfcc.init_addarg_count(argn, 0);

	uint64_t *p = (uint64_t*)global_p;

	unsigned int count = 0;

	unsigned int size = sizeof(Point);
	unsigned int n = size / 8 + size % 8;

	OpCode_x64::push_rbx(jfc);

	byte &v = jfcc.sub_rsp_unadjusted();

#if (defined(_WIN64))
	for (int i = 0; i < argn; ++i) {
		for (uint64_t *dp = p + n; dp != p; --dp) {
			jfcc.push(*(dp - 1));
		}
	}

	OpCode_x64::mov_rbx_rsp(jfc);

	for (int i = argn; i != 0; --i) {
		jfcc.add_int_rbx();
		assert(n * 0x8 <= UINT32_MAX && int32_t(n * 0x8) > 0);
		OpCode_x64::add_rbx_uint32(jfc, n * 0x8);
	}
#elif (defined(__x86_64__))
	for (int i = 0; i < argn; ++i) {
		for (uint64_t *dp = p + n; dp != p; --dp) {
			jfcc.add_int(*(dp - 1));
		}
	}
#endif

	jfcc.call();

	jfcc.add_rsp();

	jfcc.adjust_sub_rsp(v);

	OpCode_x64::pop_rbx(jfc);

	OpCode::ret(jfc);
	}

struct PointX3
{
	uint64_t x;
	uint64_t y;
	uint64_t z;
};

void print_PointX3(PointX3 p)
{
	printf("(%llX, %llX, %llX)\n", p.x, p.y, p.z);
}

void print_PointX3N6(PointX3 p0, PointX3 p1, PointX3 p2, PointX3 p3, PointX3 p4, PointX3 p5)
{
	print_PointX3(p0);
	print_PointX3(p1);
	print_PointX3(p2);
	print_PointX3(p3);
	print_PointX3(p4);
	print_PointX3(p5);
}


PointX3 *global_pX;

void Call_5(JitFuncCreater &jfc) {

	const size_t argn = 6;

	JitFuncCallerCreater jfcc(jfc, &print_PointX3N6);

	jfcc.init_addarg_count(0, 0, argn);

	uint64_t *p = (uint64_t*)global_pX;

	unsigned int count = 0;

	size_t size = sizeof(PointX3);
	size_t n = size / 8 + size % 8;

	jfcc.push_rbx();

	byte &v = jfcc.sub_rsp_unadjusted();

#if (defined(_WIN64))
	for (int i = 0; i < argn; ++i) {
		for (uint64_t *dp = p + n; dp != p; --dp) {
			jfcc.push(*(dp - 1));
		}
	}

	OpCode_x64::mov_rbx_rsp(jfc);

	for (int i = argn; i != 0; --i) {
		jfcc.add_int_rbx();
		OpCode_x64::add_rbx_uint32(jfc, n * 0x8); // NOTICE : may > 4 byte
	}
#elif (defined(__x86_64__))

	uint64_t c = 1;
	for (int i = 0; i < argn; ++i) {
		for (uint64_t *dp = p + n; dp != p; --dp) {
			jfcc.push(c++);
		}
	}
#endif

	jfcc.call();

	jfcc.add_rsp();

	jfcc.adjust_sub_rsp(v);

	jfcc.pop_rbx();

	OpCode::ret(jfc);
}

void Call_X(JitFuncCreater &jfc)
{
	JitFuncCallerCreater jfcc(jfc, &print_int);

	jfcc.init_addarg_count(1, 0, 0);

	byte &v = jfcc.sub_rsp_unadjusted();

	OpCode_x64::mov_rbx_rsp(jfc);

#if (defined(_WIN64))
	OpCode_win64::add_int0_rbx(jfc);
#elif (defined(__x86_64__))
	OpCode_sysv64::add_int0_rbx(jfc);
#endif

	jfcc.call();

	jfcc.add_rsp();

	jfcc.adjust_sub_rsp(v);

	jfcc.ret();
}

void Call_old(JitFuncCreater &jfc) {

	/*{
	JitFuncCreater jfc(jf);

	OpCode_win64::sub_rsp(jfc);

	OpCode_win64::add_intx(jfc, 8, 3);
	OpCode_win64::add_intx(jfc, 7, 2);
	OpCode_win64::add_intx(jfc, 6, 1);
	OpCode_win64::add_intx(jfc, 5, 0);
	OpCode_win64::add_int3(jfc, 4);
	OpCode_win64::add_int2(jfc, 3);
	OpCode_win64::add_int1(jfc, 2);
	OpCode_win64::add_int0(jfc, 1);

	OpCode::call_func(jfc, &caller);

	OpCode_win64::add_rsp(jfc);
	OpCode_win64::ret(jfc);
	}*/

	//{
	//	JitFuncCreater jfc(jf);

	//	//OpCode::push_ebp(jfc);
	//	//OpCode::mov_ebp_esp(jfc);

	//	//OpCode::push_uint64(jfc, 5);
	//	//OpCode::push_uint64(jfc, 6);

	//	//OpCode::push_uint64(jfc, 6);
	//	//OpCode::push_uint64(jfc, 6);
	//	//OpCode::push_uint64(jfc, 0);

	//	OpCode::push_byte(jfc, 0);
	//	OpCode::push_byte(jfc, 6);
	//	OpCode::push_byte(jfc, 5);

	//	//OpCode::mov_erax(jfc, 5);
	//	//OpCode::mov_erax(jfc, 6);

	//	OpCode::call_func(jfc, &print_pointx);


	//	//OpCode::pop_ebp(jfc);
	//	OpCode::leave(jfc);
	//	OpCode::ret(jfc);        // ret
	//}
}

using ii = uint64_t;

void callee(ii a, ii b, ii c, ii d, ii e)
{
	print_num(a);
	print_num(b);
	print_num(c);
	print_num(d);
	print_num(e);
}

void callerX()
{
	Point p{ 5, 6 };
	print_Point(p);
	//callee(1, 2, 3, 4, 0x55555555);
}

int main(int argc, char *argv[])
{
	global_p = new Point{ 0x15, 0x36 };
	global_pX = new PointX3{ 0x15, 0x36, 0x75 };

	Call(Call_1);
	Call(Call_2);
	Call(Call_3);
	Call(Call_4);
	Call(Call_5);

	print_Point(*global_p);

	printf("Done.\n");

	return 0;
}
