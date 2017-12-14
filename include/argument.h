// JitFFI Library
// * argument.h

#pragma once
#ifndef _JITFFI_ARGUMENT_H_
#define _JITFFI_ARGUMENT_H_
#include <list>
#include <vector>
#include <memory>
#include "platform.h"

namespace JitFFI
{
	enum ArgTypeBase
	{
		AT_Unknown = 0,
		AT_Void = 1,
		AT_Struct = 2,
	};

	using ArgTypeIndex = uint32_t;

	struct ArgTypeUnitBase
	{
		explicit ArgTypeUnitBase() = default;
		explicit ArgTypeUnitBase(ArgTypeIndex type, uint32_t size)
			: type(type), size(size) {}

		ArgTypeIndex type = 0;
		uint32_t size = 0;
	};
	struct ArgTypeUnit : public ArgTypeUnitBase
	{
		using TypeList = std::vector<const ArgTypeUnit*>;

		explicit ArgTypeUnit(ArgTypeIndex type, size_t size)
			: ArgTypeUnitBase(type, uint32_t(size)) {}

		explicit ArgTypeUnit(size_t size, size_t align, const TypeList &typelist)
			: ArgTypeUnitBase(AT_Struct, uint32_t(size)), align(uint32_t(align)), typelist(typelist) {}

		uint32_t align = 0;
		TypeList typelist;
	};

	using ArgData = const void*;
	using ArgType = const ArgTypeUnit*;
	using ArgDataList = std::list<ArgData>;
	using ArgTypeList = std::list<ArgType>;
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
#endif
