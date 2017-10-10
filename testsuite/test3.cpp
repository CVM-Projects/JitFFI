#include "testsuite.h"
#include <cfloat>

namespace JitFFI
{
	const ArgTypeUnit atu_bool(AT_Int, sizeof(bool));

	const ArgTypeUnit atu_char(AT_Int, sizeof(char));
	const ArgTypeUnit atu_schar(AT_Int, sizeof(signed char));
	const ArgTypeUnit atu_uchar(AT_Int, sizeof(unsigned char));
	const ArgTypeUnit atu_wchar(AT_Int, sizeof(wchar_t));

	const ArgTypeUnit atu_int(AT_Int, sizeof(int));
	const ArgTypeUnit atu_lint(AT_Int, sizeof(long int));
	const ArgTypeUnit atu_llint(AT_Int, sizeof(long long int));
	const ArgTypeUnit atu_sint(AT_Int, sizeof(short int));

	const ArgTypeUnit atu_uint(AT_Int, sizeof(unsigned int));
	const ArgTypeUnit atu_ulint(AT_Int, sizeof(unsigned long int));
	const ArgTypeUnit atu_ullint(AT_Int, sizeof(unsigned long long int));
	const ArgTypeUnit atu_usint(AT_Int, sizeof(unsigned short int));

	const ArgTypeUnit atu_float(AT_Float, sizeof(float));
	const ArgTypeUnit atu_double(AT_Float, sizeof(double));

#if (defined(_WIN64))
	const ArgTypeUnit atu_ldouble = atu_double;
#elif (defined(__x86_64__))
	const ArgTypeUnit atu_ldouble(AT_Memory, sizeof(long double), { { 0, &atu_double }, { 8, &atu_double } });
#endif

	const ArgTypeUnit atu_pointer(AT_Int, sizeof(void*));

	const ArgTypeUnit atu_size(AT_Int, sizeof(size_t));

	const ArgTypeUnit atu_int8(AT_Int, sizeof(int8_t));
	const ArgTypeUnit atu_int16(AT_Int, sizeof(int16_t));
	const ArgTypeUnit atu_int32(AT_Int, sizeof(int32_t));
	const ArgTypeUnit atu_int64(AT_Int, sizeof(int64_t));

	const ArgTypeUnit atu_uint8(AT_Int, sizeof(int8_t));
	const ArgTypeUnit atu_uint16(AT_Int, sizeof(int16_t));
	const ArgTypeUnit atu_uint32(AT_Int, sizeof(int32_t));
	const ArgTypeUnit atu_uint64(AT_Int, sizeof(int64_t));
}

void function_2x(long double v);
int floating(int a, float b, double c, long double d)
{
	int i = 0;

	print(a);
	print(b);
	print(c);
	print(d);

	//i = (int)((float)a / b + ((float)c / 1));

	return i;
}

void print_uint64(uint64_t v)
{
	print_num(v);
}

void function_2x(long double v)
{
	//uint64_t *p = (uint64_t*)&v;
	//uint64_t *p = (uint64_t*)&x;

	//print_uint64(*p);
	//print_uint64(*(p + 1));

	printf("%Lf\n", v);
	//printf("0x%llX, 0x%llX\n", *p, *(p + 1));
}

template <typename _Ty1, typename _Ty2>
unsigned int get_post(const _Ty1 *t1, const _Ty2 *t2)
{
	const byte *p1 = (const byte*)t1;
	const byte *p2 = (const byte*)t2;

	std::tie(p1, p2) = std::minmax(p1, p2);

	assert(p2 - p1 < UINT32_MAX);

	return static_cast<unsigned int>(p2 - p1);
}

template <typename _FTy>
void Call_SX(JitFuncCreater &jfc, JitFFI::CurrABI::ArgumentList &list, _FTy *func)
{
	using namespace JitFFI::CurrABI;

	JitFuncCallerCreaterPlatform jfcc(jfc, func);
	jfcc.push_rbx();
	byte &v = jfcc.sub_rsp_unadjusted();

	add_argument(jfcc, list);

	jfcc.call();

	jfcc.add_rsp();
	jfcc.adjust_sub_rsp(v);
	jfcc.pop_rbx();
	jfcc.ret();
}

void Call_1(JitFuncCreater &jfc)
{
	using namespace JitFFI::CurrABI;

	long double ld = 2.5;

	ArgumentList list;

	push_struct_data(list, &ld, atu_ldouble);

	Call_SX(jfc, list, &function_2x);
}

void Call_2(JitFuncCreater &jfc)
{
	using namespace JitFFI::CurrABI;

	int i = 5;
	float f = 6.75;
	double d = 8.0;
	long double ld = 2.5;


	ArgumentList list;

	list.push(AT_Int, convert_uint64(i));
	list.push(AT_Float, convert_uint64(f));
	list.push(AT_Float, convert_uint64(d));
	push_struct_data(list, &ld, atu_ldouble);

	Call_SX(jfc, list, &floating);
}

struct Point
{
	uint64_t x, y;
};

void print_Point(Point p)
{
	print(p.x);
	print(p.y);
}

void Call_3(JitFuncCreater &jfc)
{

	using namespace JitFFI::CurrABI;

	Point p{ 5, 6 };

	ArgumentList list;

	ArgTypeUnit atu(sizeof(Point), { { get_post(&p, &p.x), &atu_uint64}, { get_post(&p, &p.y), &atu_uint64} });

	push_struct_data(list, &p, atu);

	Call_SX(jfc, list, &print_Point);
}

void Call_4(JitFuncCreater &jfc)
{
	using namespace JitFFI::CurrABI;

	Point p{ 5, 6 };


	ArgTypeUnit atu(sizeof(Point), { { get_post(&p, &p.x), &atu_uint64 }, { get_post(&p, &p.y), &atu_uint64 } });

	ArgumentList list;

	push_struct_data(list, &p, atu);

	Call_SX(jfc, list, &print_Point);


}

int main()
{
	Call(Call_1);
	Call(Call_2);
	Call(Call_3);
	Call(Call_4);
	//printf("%d\n", floating(6, 3.14159, ((double)1.0 / (double)3.0), 1));
}
