// JitFFI Library
// * platform/argtype-ms64.h

#pragma once
#ifndef _JITFFI_PLATFORM_ARGTYPE_MS64_H_
#define _JITFFI_PLATFORM_ARGTYPE_MS64_H_

namespace JitFFI
{
	namespace MS64
	{
		enum ArgType
		{
			AT_Unknown = 0,
			AT_Void = 1,
			AT_Struct = 2,
			AT_Int,
			AT_Float,
			AT_Memory,
			AT_Over,
		};
	}
}
#endif
