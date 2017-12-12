#include "jitffi-def.h"
#include "opcode.h"

namespace JitFFI
{
	// JitFuncCallerCreaterX64

	byte JitFuncCallerCreaterX64::get_sub_offset() {
		return (push_count % 2 == 0) ? 0 : 0x8;
	}

	auto JitFuncCallerCreaterX64::get_add_offset() {
		auto push_offset = push_count * 0x8;
		assert(push_offset <= UINT32_MAX - 0x10);
		return 0x8 + get_sub_offset() + push_offset;
	}

	void JitFuncCallerCreaterX64::sub_rsp() {
		OpCode_x64::sub_rx_byte(data, OpCode_x64::rsp, 0x8);
		sub_rsp_ptr = data.end() - 1;
	}

	void JitFuncCallerCreaterX64::add_rsp() {
		OpCode_x64::add_rx_u32(data, OpCode_x64::rsp, get_add_offset());  // !NOTICE! this num may > 1 byte.
		assert(sub_rsp_ptr);
		*sub_rsp_ptr += get_sub_offset();
	}

	void JitFuncCallerCreaterX64::push(uint64_t dat) {
		OpCode_x64::movq_u64(data, OpCode_x64::rax, dat);
		OpCode_x64::push(data, OpCode_x64::rax);
		push_count += 1;
	}

	void JitFuncCallerCreaterX64::push_reg(OpCode_x64::Register r) {
		OpCode_x64::push(data, r);
		push_count += 1;
	}

	void JitFuncCallerCreaterX64::call_func(void *func) {
		OpCode_x64::movq_u64(data, OpCode_x64::rax, reinterpret_cast<uint64_t>(func));
		OpCode_x64::call_rax(data);
	}
}
