// JitFFI Library
// * jitffi.h

#pragma once
#ifndef _JITFFI_H_
#define _JITFFI_H_
#include <memory>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <list>
#include <vector>

namespace JitFFI
{
	class JitFuncPool
	{
	public:
		enum Access {
			ReadOnly,
			ReadWrite
		};

	public:
		explicit JitFuncPool(size_t _size, Access access = ReadWrite) :
			_size(_size),
			data(alloc(_size, access == ReadOnly), [=](void *dp) { free(dp, _size); }) {}

		explicit JitFuncPool(void *data, size_t _size = 0) :
			_size(_size),
			data(data, [](void *dp) {}) {}

		explicit JitFuncPool(const JitFuncPool &data, size_t _size) :
			_size(_size),
			data(data.get(), [](void *dp) {}) {}

		JitFuncPool(const JitFuncPool &) = default;

		template <typename T = void>
		T* get() const {
			return static_cast<T*>(data.get());
		}

		void unprotect() {
			unprotect(data.get(), _size);
		}
		void protect() {
			protect(data.get(), _size);
		}

		size_t size() const {
			return _size;
		}

	private:
		const size_t _size;
		std::shared_ptr<void> data;

	public:
		static void* alloc(size_t _size, bool readonly);
		static void protect(void *dp, size_t _size);
		static void unprotect(void *dp, size_t _size);
		static void free(void *dp, size_t _size);
	};

	class JitFunc
	{
		using byte = uint8_t;
	public:
		explicit JitFunc(JitFuncPool &jfp) : jfp(jfp) {}
		explicit JitFunc(JitFuncPool &&jfp) : jfp(jfp) {}
		explicit JitFunc(JitFuncPool &jfp, size_t offset, size_t size) : jfp(jfp.get<byte>() + offset, size) {}

		template <typename T>
		T* func() const {
			return reinterpret_cast<T*>(get());
		}

		template <typename T = void>
		T* get() const {
			return jfp.get<T>();
		}

		void unprotect() {
			jfp.unprotect();
		}
		void protect() {
			jfp.protect();
		}

		size_t size() const {
			return jfp.size();
		}

	private:
		JitFuncPool jfp;
	};

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
		byte& sub_rsp_unadjusted();
		void adjust_sub_rsp(byte &d);

		template <typename _FTy> void init_func(_FTy *fp) { func = fp; }
		virtual void init_addarg_count(unsigned int int_c, unsigned int dou_c, unsigned int mem_c = 0) = 0;

		virtual void add_int(uint64_t dat) = 0;
		virtual void add_int_uint32(uint32_t dat) = 0;
		virtual void add_int_rbx() = 0;
		virtual void add_double(uint64_t dat) = 0;

		void push(uint64_t);

		void sub_rbx(uint32_t dat);
		void mov_rbx_rsp();
		void push_rbx();
		void pop_rbx();

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

		byte get_offset();
		byte get_sub_offset();
		auto get_add_offset();

		using OpHandler = unsigned int(unsigned int);
	};
}

namespace JitFFI
{
	using byte = uint8_t;

	enum ArgType
	{
		AT_Unknown,
		AT_Int,
		AT_Float,
		AT_Memory,
		AT_Struct,
	};

	struct ArgTypeUnit
	{
		using TypeData = const ArgTypeUnit*;
		using TypeDataList = std::vector<TypeData>;

		explicit ArgTypeUnit(ArgType type, size_t size, size_t align)
			: type(type), size(unsigned(size)), align(unsigned(align)) {}

		explicit ArgTypeUnit(size_t size, size_t align, const TypeDataList &typedata)
			: ArgTypeUnit(AT_Struct, size, align, typedata) {}

		explicit ArgTypeUnit(ArgType type, size_t size, size_t align, const TypeDataList &typedata)
			: type(type), size(unsigned(size)), align(unsigned(align)), typedata(typedata) {}

		ArgType type = AT_Unknown;
		unsigned int size = 0;
		unsigned int align = 0;
		TypeDataList typedata;
	};

	using ArgDataList = std::list<const void*>;
	using ArgTypeList = std::list<const ArgTypeUnit*>;
}

namespace JitFFI
{
	enum Platform
	{
		P_MS64,
		P_SysV64,
	};
}

namespace JitFFI
{
	class ArgumentInfo
	{
	public:
		template <typename T>
		explicit ArgumentInfo(Platform platform, T *data) :
#ifndef NDEBUG
			_platform(platform),
#endif
			_data(data, [](void *p) { delete (T*)p; }) {}

		template <Platform P, typename T>
		const T& data() const {
			assert(P == _platform);
			return *(T*)_data.get();
		}

	private:
#ifndef NDEBUG
		Platform _platform;
#endif
		std::shared_ptr<void> _data;
	};
}

namespace JitFFI
{
	// convert_uintxx:
	//    This function can convert to a match size integer of value.

	inline uint32_t convert_uint32(const void *dat, size_t n)
	{
		if (n == 4) {
			return *reinterpret_cast<const uint32_t*>(dat);
		}
		else if (n == 2) {
			return *reinterpret_cast<const uint16_t*>(dat);
		}
		else if (n == 1) {
			return *reinterpret_cast<const uint8_t*>(dat);
		}
		else {
			assert(false);
			return 0xcccccccc;
		}
	}

	inline uint64_t convert_uint64(const void *dat, size_t n)
	{
		if (n == 8) {
			return *reinterpret_cast<const uint64_t*>(dat);
		}
		else {
			return convert_uint32(dat, n);
		}
	}
	template <typename T>
	inline uint64_t convert_uint32(const T &dat)
	{
		return convert_uint32(&dat, sizeof(T));
	}
	template <typename T>
	inline uint64_t convert_uint64(const T &dat)
	{
		return convert_uint64(&dat, sizeof(T));
	}
}

namespace JitFFI
{
	inline unsigned int get_value_count(size_t size, size_t psize) {
		assert(size < UINT32_MAX);
		unsigned int count = static_cast<unsigned int>(size / psize);
		unsigned int remsize = static_cast<unsigned int>(size % psize);
		return count + ((remsize == 0) ? 0 : 1);
	}

	template <typename T>
	inline unsigned int push_memory(const void *dat, size_t size, std::function<void(T)> func_push) {
		assert(size < UINT32_MAX);
		unsigned int count = static_cast<unsigned int>(size / sizeof(T));
		unsigned int remsize = static_cast<unsigned int>(size % sizeof(T));

		const T *dp = reinterpret_cast<const T*>(dat);

		for (unsigned int i = 0; i != count; ++i) {
			func_push(dp[i]);
		}

		if (remsize != 0) {
			T v = 0;
			const byte *p = reinterpret_cast<const byte*>(dp + count);
			memcpy(&v, p, remsize);
			func_push(v);
		}

		return get_value_count(size, sizeof(T));
	}
}

namespace JitFFI
{
	// get_argumentinfo       = ArgTypeList                            -> ArgumentInfo
	// create_function_caller = (Func ArgumentInfo)                    -> function<void (void*, ArgDataList)>;
	// create_function_caller = (Func ArgumentInfo ArgDataList)        -> function<void (void*)>;
	// create_function_caller = (Func ArgumentInfo ArgDataListPartial) -> function<void (void*, ArgDataListPartial)>;

#define _DECLARE_create_function_caller \
	ArgumentInfo get_argumentinfo(const ArgTypeList &atlist); \
	void create_function_caller(JitFuncCreater &jfc, void *func, const ArgumentInfo &argumentinfo, const ArgDataList &adlist); \
	template <typename _FTy> \
	void create_function_caller(JitFuncCreater &jfc, _FTy *func, const ArgumentInfo &argumentinfo, const ArgDataList &adlist) { \
		create_function_caller(jfc, (void*)func, argumentinfo, adlist); \
	}

	namespace SysV64 { _DECLARE_create_function_caller }
	namespace MS64 { _DECLARE_create_function_caller }

#undef _DECLARE_create_function_caller
}

namespace JitFFI
{
	extern const ArgTypeUnit atu_bool;

	extern const ArgTypeUnit atu_char;
	extern const ArgTypeUnit atu_schar;
	extern const ArgTypeUnit atu_uchar;
	extern const ArgTypeUnit atu_wchar;

	extern const ArgTypeUnit atu_int;
	extern const ArgTypeUnit atu_lint;
	extern const ArgTypeUnit atu_llint;
	extern const ArgTypeUnit atu_sint;

	extern const ArgTypeUnit atu_uint;
	extern const ArgTypeUnit atu_ulint;
	extern const ArgTypeUnit atu_ullint;
	extern const ArgTypeUnit atu_usint;

	extern const ArgTypeUnit atu_float;
	extern const ArgTypeUnit atu_double;

	extern const ArgTypeUnit atu_ldouble;

	extern const ArgTypeUnit atu_pointer;

	extern const ArgTypeUnit atu_size;

	extern const ArgTypeUnit atu_int8;
	extern const ArgTypeUnit atu_int16;
	extern const ArgTypeUnit atu_int32;
	extern const ArgTypeUnit atu_int64;

	extern const ArgTypeUnit atu_uint8;
	extern const ArgTypeUnit atu_uint16;
	extern const ArgTypeUnit atu_uint32;
	extern const ArgTypeUnit atu_uint64;
}

#	if (defined (_WIN64))
#		define CurrABI MS64
#	elif (defined (__x86_64__))
#		define CurrABI SysV64
#	endif
#endif
