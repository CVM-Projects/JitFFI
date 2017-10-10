#include "testsuite.h"

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

struct Point
{
	uint64_t x, y;
};

void print_Point(Point p)
{
	print(p.x);
	print(p.y);
}

const ArgTypeUnit& get_atu_Point()
{
	static Point p;
	static ArgTypeUnit atu(sizeof(Point), { { get_post(&p, &p.x), &atu_uint64 },{ get_post(&p, &p.y), &atu_uint64 } });

	return atu;
}

void Call_1(JitFuncCreater &jfc)
{
	long double ld = 2.5;

	CurrABI::create_function_caller(jfc, &function_2x, { &ld }, { &atu_ldouble });
}

void Call_2(JitFuncCreater &jfc)
{
	int i = 5;
	float f = 6.75;
	double d = 8.0;
	long double ld = 2.5;

	CurrABI::create_function_caller(jfc, &floating, { &i, &f, &d, &ld }, { &atu_int, &atu_float, &atu_double, &atu_ldouble });
}

void Call_3(JitFuncCreater &jfc)
{
	Point p{ 5, 6 };

	CurrABI::create_function_caller(jfc, &print_Point, { &p }, { &get_atu_Point() });
}

int main()
{
	Call(Call_1);
	Call(Call_2);
	Call(Call_3);
}
