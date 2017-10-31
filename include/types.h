// JitFFI Library
// * types.h

#pragma once
#ifndef _JITFFI_TYPES_H_
#define _JITFFI_TYPES_H_
#include <cstdint>
#include "argument.h"

namespace JitFFI
{
	namespace MS64
	{
#include "types.inl"
	}
	namespace SysV64
	{
#include "types.inl"
	}
}
#endif
