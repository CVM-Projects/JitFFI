#pragma once
#include "jitffi.h"

namespace JitFFI
{
	inline auto get_next_post_f(std::function<unsigned int()> get_next_size, const unsigned int align) {
		bool is_first = true;
		unsigned int count = 0;

		return [=]() mutable {
			unsigned int result;

			unsigned int size = get_next_size();
			if (is_first) {
				count += size;
				result = 0;
				is_first = false;
			}
			else {
				unsigned int pack = std::min<unsigned int>(align, size);
				unsigned int rem = count % pack;

				if (rem != 0) {
					count += pack - rem;
				}
				result = count;
				count += size;
			}
			return result;
		};
	}
}
