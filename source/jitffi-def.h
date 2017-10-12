#pragma once
#include "jitffi.h"

namespace JitFFI
{
	inline auto get_next_post_f(std::function<const ArgTypeUnit *()> get_next_data, const unsigned int pack = 8) {
		bool is_first = true;
		unsigned int count = 0;

		return [=]() mutable {
			unsigned int result;

			const ArgTypeUnit &type = *get_next_data();
			if (is_first) {
				count += type.size;
				result = 0;
				is_first = false;
			}
			else {
				unsigned int npack = std::min(pack, type.size);

				unsigned int rem = count % npack;
				if (rem != 0) {
					count += npack - rem;
				}

				result = count;

				count += type.size;
			}
			return result;
		};
	}

	template <typename ArgumentList, typename JitFuncCallerCreaterPlatform>
	void create_function_caller_base(JitFuncCreater &jfc, ArgumentList &list, void *func)
	{
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

	template <typename ArgumentList, typename JitFuncCallerCreaterPlatform>
	void create_function_caller_base(JitFuncCreater &jfc, void *func, const ArgDataList &adlist, const ArgTypeList &atlist)
	{
		ArgumentList list;

		assert(adlist.size() == atlist.size());
		assert(adlist.size() < UINT32_MAX);

		size_t count = adlist.size();

		auto ad_iter = adlist.begin();
		auto at_iter = atlist.begin();

		while (count--) {
			push_data(list, *ad_iter, **at_iter);

			++ad_iter;
			++at_iter;
		}

		create_function_caller_base<ArgumentList, JitFuncCallerCreaterPlatform>(jfc, list, func);
	}
}
