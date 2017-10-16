// JitFFI Library
// * jitffi-ms64.h

#pragma once
#ifndef _JITFFI_MS64_H_
#define _JITFFI_MS64_H_
#include "jitffi.h"

namespace JitFFI
{
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
				return JitFFI::push_memory<uint64_t>(dat, size, [&](uint64_t v) { push_memory(v); });
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
	}
}
#endif
