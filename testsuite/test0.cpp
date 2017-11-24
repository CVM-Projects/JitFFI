// test1.cpp
// This file can run on the x86-64 system.
#include "testsuite.h"

void Create_OP_x64_mov(JitFuncCreater &jfc, const std::vector<byte> &r_regs, const std::vector<byte> &p_regs)
{
	using namespace OpCode_x64;

	for (byte r1 : r_regs)
		for (byte r2 : r_regs)
			mov(jfc, static_cast<Register>(r1), static_cast<Register>(r2));
	for (byte r1 : r_regs)
		for (byte r2 : p_regs)
			mov(jfc, static_cast<Register>(r1), static_cast<Register>(r2));
	for (byte r1 : p_regs)
		for (byte r2 : r_regs)
			mov(jfc, static_cast<Register>(r1), static_cast<Register>(r2));
}

void Create_OP_x64_push_pop(JitFuncCreater &jfc, const std::vector<byte> &r_regs, const std::vector<byte> &p_regs)
{
	using namespace OpCode_x64;

	for (byte r : r_regs)
		push(jfc, static_cast<Register>(r));

	for (byte r : p_regs)
		push(jfc, static_cast<Register>(r));

	for (byte r : r_regs)
		pop(jfc, static_cast<Register>(r));

	for (byte r : p_regs)
		pop(jfc, static_cast<Register>(r));
}

void Create_OP_x64(JitFuncCreater &jfc)
{
	using namespace OpCode_x64;
	std::vector<byte> r_regs;
	std::vector<byte> p_regs;
	for (byte r = 0; r <= 7; r++) r_regs.push_back(r);
	for (byte r = 10; r <= 14; r++) r_regs.push_back(r);
	for (byte r = 20; r <= 27; r++) p_regs.push_back(r);
	for (byte r = 30; r <= 34; r++) p_regs.push_back(r);

	Create_OP_x64_mov(jfc, r_regs, p_regs);
	Create_OP_x64_push_pop(jfc, r_regs, p_regs);
}

int main()
{
	Compile(Create_OP_x64);

	return 0;
}
