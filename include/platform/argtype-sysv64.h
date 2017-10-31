// JitFFI Library
// * platform/argtype-sysv64.h

#pragma once
#ifndef _JITFFI_PLATFORM_ARGTYPE_SYSV64_H_
#define _JITFFI_PLATFORM_ARGTYPE_SYSV64_H_

namespace JitFFI
{
	namespace SysV64
	{
		enum ArgType
		{
			AT_Unknown = 0,
			AT_Void = 1,
			AT_Struct = 2,
			AT_Integer,
			AT_SSE,
			AT_SSEUP,
			AT_X87,
			AT_X87UP,
			AT_ComplexX87,
			AT_NoClass,
			AT_Memory,
			AT_Over,
		};
	}
}
#endif
