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
