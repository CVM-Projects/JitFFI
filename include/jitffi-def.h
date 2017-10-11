#pragma once
#include "jitffi.h"

namespace JitFFI
{
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
