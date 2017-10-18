// JitFFI Library
// * platform.h

#pragma once
#ifndef _JITFFI_PLATFORM_H_
#define _JITFFI_PLATFORM_H_
#include <cstdint>

namespace JitFFI
{
	using byte = uint8_t;

	enum Platform
	{
		P_Unknown,
		P_MS64,
		P_SysV64,
	};
}

#	if (defined (_WIN64))
#		define CurrABI MS64
#	elif (defined (__x86_64__))
#		define CurrABI SysV64
#	endif
#endif
