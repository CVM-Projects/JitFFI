#pragma once
#include <memory>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <list>
#include <algorithm>

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

	namespace MS64
	{
		class JitFuncCallerCreaterPlatform : public JitFuncCallerCreater
		{
		public:
			template <typename _FTy>
			explicit JitFuncCallerCreaterPlatform(JitFuncCreater &jfc, _FTy *func = nullptr)
				: JitFuncCallerCreater(jfc, func) {}

			void init_addarg_count(unsigned int int_c, unsigned int dou_c, unsigned int mem_c = 0) {
				argn = int_c + dou_c + mem_c;
				have_init = true;
			}

			void add_int(uint64_t dat);
			void add_int_uint32(uint32_t dat);
			void add_int_rbx();
			void add_double(uint64_t dat);

			void call();

		private:
			void _add_int(const std::function<OpHandler> &handler);
			void _add_double(const std::function<OpHandler> &handler);

			unsigned int argn;
			unsigned int add_count = 0;
		};
	}

	namespace SysV64
	{
		class JitFuncCallerCreaterPlatform : public JitFuncCallerCreater
		{
		public:
			template <typename _FTy>
			explicit JitFuncCallerCreaterPlatform(JitFuncCreater &jfc, _FTy *func = nullptr)
				: JitFuncCallerCreater(jfc, func) {}

			void init_addarg_count(unsigned int int_c, unsigned int dou_c, unsigned int mem_c = 0) {
				addarg_int_count = int_c;
				addarg_double_count = dou_c;
				have_init = true;
			}

			void add_int(uint64_t dat);
			void add_int_uint32(uint32_t dat);
			void add_int_rbx();
			void add_double(uint64_t dat);

			void call();

		private:
			void _add_int(const std::function<OpHandler> &handler);
			void _add_double(const std::function<OpHandler> &handler);

			unsigned int addarg_int_count = 0;
			unsigned int addarg_double_count = 0;
		};
	}

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
	namespace MS64
	{
		inline bool need_pass_by_pointer(size_t n) {
			return (n != 1 && n != 2 && n != 4 && n != 8);
		}
		inline void push_copy(JitFuncCallerCreater &jfcc, const void *tp, size_t size) {
			unsigned int n = static_cast<unsigned int>(size / 8 + size % 8);
			const uint64_t *p = reinterpret_cast<const uint64_t*>(tp);

			for (const uint64_t *dp = p + n; dp != p; --dp) {
				jfcc.push(*(dp - 1));
			}
		}
	}

	namespace SysV64
	{
		enum PassType
		{
			PT_Integer,
			PT_SSE,
			PT_SSEUP,
			PT_X87,
			PT_Memory,
		};
		inline bool need_pass_by_memory(size_t n) {
			if (n > 16)
				return true;
			else
				return false;
		}
	}
}
#include <vector>

namespace JitFFI
{
	using byte = uint8_t;

	enum ArgType
	{
		AT_Unknown,
		AT_Int,
		AT_Float,
		AT_Memory,
	};

	struct TypeListUnit
	{
		unsigned int post;
		unsigned int size;
		ArgType type;
	};

	struct TypeList
	{
		unsigned int num;
		TypeListUnit *data;
	};

	struct ArgTypeUnit
	{
		struct TypeData { unsigned int post; const ArgTypeUnit *argtype; };
		using TypeDataList = std::vector<TypeData>;

		explicit ArgTypeUnit(size_t size, TypeDataList &&typedata)
			: type(AT_Unknown), size(unsigned(size)), typedata(typedata) {}

		explicit ArgTypeUnit(ArgType type, size_t size)
			: type(type), size(unsigned(size)) {}

		explicit ArgTypeUnit(ArgType type, size_t size, TypeDataList &&typedata)
			: type(type), size(unsigned(size)), typedata(typedata) {}

		ArgType type = AT_Unknown;
		unsigned int size = 0;
		TypeDataList typedata;
	};

	using ArgDataList = std::list<void*>;
	using ArgTypeList = std::list<const ArgTypeUnit*>;

	class NewStruct
	{
	public:
		bool push(byte *dat, unsigned int size);

		void write_uint64(unsigned int i, byte *dat) {
			((uint64_t*)&_data)[i] = *(uint64_t*)dat;
		}
		void write_uint32(unsigned int i, byte *dat) {
			((uint32_t*)&_data)[i] = *(uint32_t*)dat;
		}
		void write_uint16(unsigned int i, byte *dat) {
			((uint16_t*)&_data)[i] = *(uint16_t*)dat;
		}
		void write_byte(unsigned int i, byte *dat) {
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
		unsigned int count = 0;
		uint64_t _data = 0;
	};

	namespace MS64
	{
		class ArgumentList
		{
			struct Data {
				ArgType type;
				uint64_t data;
			};
			using DataList = std::list<Data>;

		public:
			ArgumentList() = default;

			void push(ArgType type, uint64_t v) {
				list.push_front({ type, v });
			}
			unsigned int push_memory(uint64_t v) {
				memlist.push_front(v);
				return 1;
			}
			unsigned int push_memory(void *dat, size_t size) {
				assert(size < UINT32_MAX);
				unsigned int count = static_cast<unsigned int>(size / 8);
				unsigned int remsize = static_cast<unsigned int>(size % 8);

				uint64_t *dp = reinterpret_cast<uint64_t*>(dat);

				for (unsigned int i = 0; i != count; ++i) {
					push_memory(dp[i]);
				}

				if (remsize != 0) {
					uint64_t v = 0;
					byte *p = reinterpret_cast<byte*>(dp + count);
					memcpy(&v, p, remsize);
					push_memory(v);
				}

				return count + ((remsize == 0) ? 0 : 1);
			}

			bool get_next_memory(uint64_t &data) {
				if (memlist.empty()) {
					return false;
				}
				else {
					data = memlist.front();
					memlist.pop_front();
					return true;
				}
			}

			bool get_next(ArgType &type, uint64_t &data) {
				if (list.empty()) {
					return false;
				}
				else {
					auto &e = list.front();
					type = e.type;
					data = e.data;
					list.pop_front();
					return true;
				}
			}

			unsigned int size() const {
				assert(list.size() < UINT32_MAX);
				return static_cast<unsigned int>(list.size());
			}

		private:
			DataList list;
			std::list<uint64_t> memlist;
		};

		void push_data(ArgumentList &list, void *t, const ArgTypeUnit &atu);
		void add_argument(JitFuncCallerCreater &jfcc, ArgumentList &list);

		// Will be deleted.
		void pass_struct(JitFuncCallerCreater &jfcc, void *t, size_t size, const TypeList &typelist);
	}

	namespace SysV64
	{
		class ArgumentList
		{
			struct Data {
				ArgType type;
				uint64_t data;
			};
			using DataList = std::list<Data>;

		public:
			ArgumentList() = default;

			void push(ArgType type, uint64_t v) {
				switch (type) {
				case AT_Int: _int_count++; break;
				case AT_Float: _float_count++; break;
				case AT_Memory: _memory_count++; break;
				default: assert(false);
				}
				list.push_front({ type, v });
			}

			bool get_next(ArgType &type, uint64_t &data) {
				if (list.empty()) {
					return false;
				}
				else {
					auto &e = list.front();
					type = e.type;
					data = e.data;
					list.pop_front();
					return true;
				}
			}

			unsigned int get_int_count() const {
				return _int_count;
			}
			unsigned int get_float_count() const {
				return _float_count;
			}
			unsigned int get_memory_count() const {
				return _memory_count;
			}

		private:
			DataList list;
			unsigned int _int_count = 0;
			unsigned int _float_count = 0;
			unsigned int _memory_count = 0;
		};

		void push_data(ArgumentList &list, void *t, const ArgTypeUnit &atu);
		void add_argument(JitFuncCallerCreater &jfcc, ArgumentList &list);

		// Will be deleted.
		void push_struct_data(ArgumentList &list, void *t, size_t size, const TypeList &typelist);
		void pass_struct(JitFuncCallerCreater &jfcc, void *t, size_t size, const TypeList &typelist);
	}
}

namespace JitFFI
{
	namespace SysV64
	{
		void create_function_caller(JitFuncCreater &jfc, ArgumentList &list, void *func);
		void create_function_caller(JitFuncCreater &jfc, void *func, const ArgDataList &adlist, const ArgTypeList &atlist);

		template <typename _FTy>
		void create_function_caller(JitFuncCreater &jfc, ArgumentList &list, _FTy *func)
		{
			create_function_caller(jfc, list, (void*)func);
		}
		template <typename _FTy>
		void create_function_caller(JitFuncCreater &jfc, _FTy *func, const ArgDataList &adlist, const ArgTypeList &atlist)
		{
			create_function_caller(jfc, (void*)func, adlist, atlist);
		}
	}
	namespace MS64
	{
		void create_function_caller(JitFuncCreater &jfc, ArgumentList &list, void *func);
		void create_function_caller(JitFuncCreater &jfc, void *func, const ArgDataList &adlist, const ArgTypeList &atlist);

		template <typename _FTy>
		void create_function_caller(JitFuncCreater &jfc, ArgumentList &list, _FTy *func)
		{
			create_function_caller(jfc, list, (void*)func);
		}
		template <typename _FTy>
		void create_function_caller(JitFuncCreater &jfc, _FTy *func, const ArgDataList &adlist, const ArgTypeList &atlist)
		{
			create_function_caller(jfc, (void*)func, adlist, atlist);
		}
	}
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


	template <typename _Ty1, typename _Ty2>
	unsigned int get_post(const _Ty1 *t1, const _Ty2 *t2)
	{
		const byte *p1 = (const byte*)t1;
		const byte *p2 = (const byte*)t2;

		std::tie(p1, p2) = std::minmax(p1, p2);

		assert(p2 - p1 < UINT32_MAX);

		return static_cast<unsigned int>(p2 - p1);
	}
}

#if (defined (_WIN64))
#	define CurrABI MS64
#elif (defined (__x86_64__))
#	define CurrABI SysV64
#endif
