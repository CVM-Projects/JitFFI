// JitFFI Library
// * creater.h

#pragma once
#ifndef _JITFFI_CREATER_H_
#define _JITFFI_CREATER_H_
#include "jitfunc.h"
#include <cassert>
#include <cstring>

namespace JitFFI
{
	class JitFuncCreater
	{
		using byte = uint8_t;
	public:
		explicit JitFuncCreater(JitFunc &data) : data(data) {}

		void push(byte dat) {
			assert(count + 1 < data.size());
			byte *code = data.get<byte>();
			code[count++] = dat;
		}
		void push(byte dat1, byte dat2) {
			push(dat1);
			push(dat2);
		}
		void push(byte dat1, byte dat2, byte dat3) {
			push(dat1, dat2);
			push(dat3);
		}
		void push(byte dat1, byte dat2, byte dat3, byte dat4) {
			push(dat1, dat2, dat3);
			push(dat4);
		}
		void push(byte dat1, byte dat2, byte dat3, byte dat4, byte dat5) {
			push(dat1, dat2, dat3, dat4);
			push(dat5);
		}
		void push_uint16(uint16_t dat) {
			write(&dat, 2);
		}
		void push_uint32(uint32_t dat) {
			write(&dat, 4);
		}
		void push_uint64(uint64_t dat) {
			write(&dat, 8);
		}
		void write(const void *src, size_t msize) {
			assert(count + msize < data.size());
			byte *code = data.get<byte>();
			std::memcpy(code + count, src, msize);
			count += msize;
		}

		byte* begin() const {
			return data.get<byte>();
		}
		byte* end() const {
			return begin() + count;
		}

		JitFunc& get() {
			return data;
		}
		const JitFunc& get() const {
			return data;
		}

	private:
		JitFunc &data;
		size_t count = 0;
	};

	class JitFuncCallerCreater
	{
		using byte = uint8_t;
	public:
		template <typename _FTy>
		explicit JitFuncCallerCreater(JitFuncCreater &jfc, _FTy *func = nullptr)
			: jfc(jfc), func(reinterpret_cast<void*>(func)) {}

		virtual ~JitFuncCallerCreater() = default;

		void sub_rsp();
		void add_rsp();

		template <typename _FTy> void init_func(_FTy *fp) { func = fp; }
		virtual void init_addarg_count(unsigned int int_c, unsigned int dou_c, unsigned int mem_c = 0) = 0;

		virtual void add_void() = 0;
		virtual void add_int(uint64_t dat) = 0;
		virtual void add_int_uint32(uint32_t dat) = 0;
		virtual void add_int_rbx() = 0;
		virtual void add_int_prax() = 0;
		virtual void add_double(uint64_t dat) = 0;
		virtual void add_double_prax() = 0;

		void push(uint64_t);

		void sub_rbx(uint32_t dat);
		void add_rbx(uint32_t dat);
		void mov_rbx_rsp();

		virtual void call() = 0;
		void ret();

		JitFuncCreater& data() {
			return jfc;
		}

	protected:
		JitFuncCreater &jfc;
		void* func;
		unsigned int push_count = 0;
		bool have_init = false;
		byte* sub_rsp_ptr = nullptr;

		byte get_offset();
		byte get_sub_offset();
		auto get_add_offset();

		using OpHandler = unsigned int(unsigned int);
	};
}
#endif
