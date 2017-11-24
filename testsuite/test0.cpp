// test1.cpp
// This file can run on the x86-64 system.
#include "testsuite.h"

struct Regs
{
	std::vector<byte> rq;
	std::vector<byte> rd;
	std::vector<byte> rw;
	std::vector<byte> rb;
	std::vector<byte> prq;
};

std::vector<byte> operator+(const std::vector<byte> &v1, const std::vector<byte> &v2)
{
	std::vector<byte> v(v1.begin(), v1.end());
	v.insert(v.end(), v2.begin(), v2.end());

	return v;
}

void Create_OP_x64_mov(JitFuncCreater &jfc, const Regs &regs)
{
	using namespace OpCode_x64;

	for (byte r1 : regs.rq)
		for (byte r2 : regs.rq)
			mov(jfc, static_cast<Register>(r1), static_cast<Register>(r2));

	for (byte r1 : regs.rd)
		for (byte r2 : regs.prq)
			mov(jfc, static_cast<Register>(r1), static_cast<Register>(r2));

	for (byte r1 : regs.rw)
		for (byte r2 : regs.prq)
			mov(jfc, static_cast<Register>(r1), static_cast<Register>(r2));

	for (byte r1 : regs.rb)
		for (byte r2 : regs.prq)
			mov(jfc, static_cast<Register>(r1), static_cast<Register>(r2));

	for (byte r1 : regs.prq)
		for (byte r2 : regs.rq)
			mov(jfc, static_cast<Register>(r1), static_cast<Register>(r2));
	
	for (byte r1 : regs.rd)
		for (byte r2 : regs.rd)
			mov(jfc, static_cast<Register>(r1), static_cast<Register>(r2));

	for (byte r1 : regs.prq)
		for (byte r2 : regs.rd)
			mov(jfc, static_cast<Register>(r1), static_cast<Register>(r2));

	for (byte r1 : regs.rw)
		for (byte r2 : regs.rw)
			mov(jfc, static_cast<Register>(r1), static_cast<Register>(r2));

	for (byte r1 : regs.prq)
		for (byte r2 : regs.rw)
			mov(jfc, static_cast<Register>(r1), static_cast<Register>(r2));

	for (byte r1 : regs.rb)
		for (byte r2 : regs.rb)
			mov(jfc, static_cast<Register>(r1), static_cast<Register>(r2));

	for (byte r1 : regs.prq)
		for (byte r2 : regs.rb)
			mov(jfc, static_cast<Register>(r1), static_cast<Register>(r2));
}

void Create_OP_x64_push_pop(JitFuncCreater &jfc, const Regs &regs)
{
	using namespace OpCode_x64;

	for (byte r : regs.rq)
		push(jfc, static_cast<Register>(r));

	for (byte r : regs.prq)
		push(jfc, static_cast<Register>(r));

	for (byte r : regs.rq)
		pop(jfc, static_cast<Register>(r));

	for (byte r : regs.prq)
		pop(jfc, static_cast<Register>(r));
}

void Create_OP_x64_add_sub(JitFuncCreater &jfc, const Regs &regs)
{
	using namespace OpCode_x64;

	for (byte r : regs.rq)
		add_rx_byte(jfc, static_cast<Register>(r), 0x1);

	for (byte r : regs.rq)
		add_rx_u32(jfc, static_cast<Register>(r), 0x1);

	for (byte r : regs.rq)
		sub_rx_byte(jfc, static_cast<Register>(r), 0x1);

	for (byte r : regs.rq)
		sub_rx_u32(jfc, static_cast<Register>(r), 0x1);
}

void Create_OP_x64_mov_rx_dat(JitFuncCreater &jfc, const Regs &regs)
{
	using namespace OpCode_x64;

	for (byte r : regs.rq)
		mov_rxq_u64(jfc, static_cast<Register>(r), 0x123456789ABCDEF0);

	for (byte r : regs.rd)
		mov_rxd_u32(jfc, static_cast<Register>(r), 0x12345678);

	for (byte r : regs.rw)
		mov_rxw_u16(jfc, static_cast<Register>(r), 0x1234);

	for (byte r : regs.rb)
		mov_rxb_u8(jfc, static_cast<Register>(r), 0x12);
}

void Create_OP_x64(JitFuncCreater &jfc)
{
	using namespace OpCode_x64;
	Regs regs;
	for (byte r = rax; r <= rdi; r++) regs.rq.push_back(r);
	for (byte r = r8; r <= r15; r++) regs.rq.push_back(r);

	for (byte r = eax; r <= edi; r++) regs.rd.push_back(r);
	for (byte r = r8d; r <= r15d; r++) regs.rd.push_back(r);

	for (byte r = ax; r <= di; r++) regs.rw.push_back(r);
	for (byte r = r8w; r <= r15w; r++) regs.rw.push_back(r);

	for (byte r = al; r <= dil; r++) regs.rb.push_back(r);
	for (byte r = r8b; r <= r15b; r++) regs.rb.push_back(r);

	for (byte r = prax; r <= prdi; r++) regs.prq.push_back(r);
	for (byte r = pr8; r <= pr15; r++) regs.prq.push_back(r);

	Create_OP_x64_mov(jfc, regs);
	//Create_OP_x64_push_pop(jfc, regs);
	//Create_OP_x64_mov_rx_dat(jfc, regs);
	//Create_OP_x64_add_sub(jfc, regs);
}

int main()
{
	Compile(Create_OP_x64, true, 0x10000);

	return 0;
}
