// test1.cpp
// This file can run on the x64 system.

#include <cstdio>
#include <algorithm>
#include "jitffi.h"
#include "opcode.h"
using namespace JitFFI;

using int64 = int64_t;

int callerN2(int64 a, int64 b)
{
	printf("%lld\n", a);
	printf("%lld\n", b);

	return 12345;
}

int callerN6(int64 a, int64 b, int64 c, int64 d, int64 e, int64 f)
{
	printf("%lld\n", a);
	printf("%lld\n", b);
	printf("%lld\n", c);
	printf("%lld\n", d);
	printf("%lld\n", e);
	printf("%lld\n", f);

	return 12345;
}

int callerN(int64 a, int64 b, int64 c, int64 d, int64 e, int64 f, int64 g, int64 h)
{
	printf("%lld\n", a);
	printf("%lld\n", b);
	printf("%lld\n", c);
	printf("%lld\n", d);
	printf("%lld\n", e);
	printf("%lld\n", f);
	printf("%lld\n", g);
	printf("%lld\n", h);

	return 12345;
}

int callerN9(int64 a, int64 b, int64 c, int64 d, int64 e, int64 f, int64 g, int64 h, int64 i)
{
	printf("%lld\n", a);
	printf("%lld\n", b);
	printf("%lld\n", c);
	printf("%lld\n", d);
	printf("%lld\n", e);
	printf("%lld\n", f);
	printf("%lld\n", g);
	printf("%lld\n", h);
	printf("%lld\n", i);

	return 123456;
}

int callerN10(int64 a, int64 b, int64 c, int64 d, int64 e, int64 f, int64 g, int64 h, int64 i, int64 j)
{
	printf("%lld\n", a);
	printf("%lld\n", b);
	printf("%lld\n", c);
	printf("%lld\n", d);
	printf("%lld\n", e);
	printf("%lld\n", f);
	printf("%lld\n", g);
	printf("%lld\n", h);
	printf("%lld\n", i);
	printf("%lld\n", j);

	return 1234567;
}

int callerX(double a, double b, double c, double d, double e, double f, double g, double h)
{
	printf("%lf\n", a);
	printf("%lf\n", b);
	printf("%lf\n", c);
	printf("%lf\n", d);
	printf("%lf\n", e);
	printf("%lf\n", f);
	printf("%lf\n", g);
	printf("%lf\n", h);

	return 54321;
}

int callerX7(double a, double b, double c, double d, double e, double f, double g)
{
	printf("%lf\n", a);
	printf("%lf\n", b);
	printf("%lf\n", c);
	printf("%lf\n", d);
	printf("%lf\n", e);
	printf("%lf\n", f);
	printf("%lf\n", g);

	return 54321;
}
int callerX8(double a, double b, double c, double d, double e, double f, double g, double h)
{
	printf("%lf\n", a);
	printf("%lf\n", b);
	printf("%lf\n", c);
	printf("%lf\n", d);
	printf("%lf\n", e);
	printf("%lf\n", f);
	printf("%lf\n", g);
	printf("%lf\n", h);

	return 54321;
}
int callerX9(double a, double b, double c, double d, double e, double f, double g, double h, double i)
{
	printf("%lf\n", a);
	printf("%lf\n", b);
	printf("%lf\n", c);
	printf("%lf\n", d);
	printf("%lf\n", e);
	printf("%lf\n", f);
	printf("%lf\n", g);
	printf("%lf\n", h);
	printf("%lf\n", i);

	return 54321;
}

int callerX10(double a, double b, double c, double d, double e, double f, double g, double h, double i, double j)
{
	printf("%lf\n", a);
	printf("%lf\n", b);
	printf("%lf\n", c);
	printf("%lf\n", d);
	printf("%lf\n", e);
	printf("%lf\n", f);
	printf("%lf\n", g);
	printf("%lf\n", h);
	printf("%lf\n", i);
	printf("%lf\n", j);

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
	printf("%lld\n", a0);
	printf("%lf\n", b0);
	printf("%lld\n", a1);
	printf("%lf\n", b1);
	printf("%lld\n", a2);
	printf("%lf\n", b2);
	printf("%lld\n", a3);
	printf("%lf\n", b3);
}

using CallerProcess = void(JitFuncCreater &jfc);

void Call(CallerProcess *handler)
{
	JitFuncPool pool(0x1000, JitFuncPool::ReadOnly);
	JitFunc jf(pool);

	byte *begin;
	byte *end;

	{
		JitFuncCreater jfc(jf);
		handler(jfc);

		begin = jfc.begin();
		end = jfc.end();
	}

	FILE *file = fopen("tmp.bin", "wb+");

	for (byte *p = begin; p != end; ++p) {
		fwrite(p, sizeof(byte), 1, file);
	}

	fclose(file);

	system("objdump -D -b binary -m i386:x86-64 tmp.bin");

	printf("\n");

	auto f = jf.func<int(void)>();

	printf("%d\n", f());
}

void Call_1(JitFuncCreater &jfc) {
	JitFuncCallerCreater jfcc(jfc, &callerX9, 9);
	jfcc.init_addarg_count(0, 9);

	byte &v = jfcc.sub_rsp_unadjusted();

	for (double v = 9; v != 0; --v)
		jfcc.addarg_double(convert_uint64(v));

	jfcc.adjust_sub_rsp(v);

	jfcc.call();
	jfcc.add_rsp();
	jfcc.ret();
}

void Call_2(JitFuncCreater &jfc) {
	JitFuncCallerCreater jfcc(jfc, &callerN10, 10);
	jfcc.init_addarg_count(10, 0);

	byte &v = jfcc.sub_rsp_unadjusted();

	for (int v = 10; v != 0; --v)
		jfcc.addarg_int(convert_uint64(v));

	jfcc.adjust_sub_rsp(v);

	jfcc.call();
	jfcc.add_rsp();
	jfcc.ret();
}

void Call_3(JitFuncCreater &jfc) {
	JitFuncCallerCreater jfcc(jfc, &callerNX, 8);
	jfcc.init_addarg_count(4, 4);

	byte &v = jfcc.sub_rsp_unadjusted();

	jfcc.addarg_double(convert_uint64(8.0));
	jfcc.addarg_int(convert_uint64(7));
	jfcc.addarg_double(convert_uint64(6.0));
	jfcc.addarg_int(convert_uint64(5));
	jfcc.addarg_double(convert_uint64(4.0));
	jfcc.addarg_int(convert_uint64(3));
	jfcc.addarg_double(convert_uint64(2.0));
	jfcc.addarg_int(convert_uint64(1));

	jfcc.adjust_sub_rsp(v);

	jfcc.call();
	jfcc.add_rsp();
	jfcc.ret();
}

void print_int(int64 v)
{
	printf("%I64X\n", v);
}

void Call_X(JitFuncCreater &jfc)
{
	JitFuncCallerCreater jfcc(jfc, &print_int, 1);

	jfcc.sub_rsp();

	OpCode_x64::mov_rax_rsp(jfc);

#if (defined(_WIN64))
	OpCode_x64::mov_rcx_rax(jfc);
#elif (defined(__x86_64__))
	OpCode_x64::mov_rdi_rax(jfc);
#endif

	jfcc.call();

	jfcc.add_rsp();

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

int main(int argc, char *argv[])
{

	printf("-----------\n");
	Call(Call_1);
	printf("-----------\n");
	Call(Call_2);
	printf("-----------\n");
	Call(Call_3);
	printf("-----------\n");

	printf("Done.\n");

	return 0;
}
