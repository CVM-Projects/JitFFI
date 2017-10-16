// JitFFI Library
// * jitffi-sysv64.h

#pragma once
#ifndef _JITFFI_SYSV64_H_
#define _JITFFI_SYSV64_H_
#include "jitffi.h"

namespace JitFFI
{
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

			void push_memory(uint64_t v) {
				list.push_front({ AT_Memory, v });
				_memory_count++;
			}

			void push_memory(void *dat, size_t size) {
				JitFFI::push_memory<uint64_t>(dat, size, [&](uint64_t v) { push_memory(v); });
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
	}
}
#endif
