#pragma once
#include "jitffi.h"

namespace JitFFI
{
	class GetTypePost
	{
		using Unit = const ArgTypeUnit* const;
	public:
		explicit GetTypePost(const ArgTypeUnit::TypeDataList &list, unsigned int pack = 8)
			: begin(&*list.begin()), end(&*list.end()), pack(pack), ptr(begin) {}

		explicit GetTypePost(Unit *begin, Unit *end, unsigned int pack = 8)
			: begin(begin), end(end), pack(pack), ptr(begin) {}

		unsigned int get_next_post() {
			unsigned int result;

			const ArgTypeUnit &type = **ptr;
			if (ptr == begin) {
				count += type.size;
				result = 0;
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
			ptr++;
			return result;
		}

	private:
		Unit * const begin;
		Unit * const end;
		const unsigned int pack;
		const ArgTypeUnit *const * ptr;
		unsigned int count = 0;
	};

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
