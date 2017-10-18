// JitFFI Library
// * types.h

#pragma once
#ifndef _JITFFI_TYPES_H_
#define _JITFFI_TYPES_H_
#include <cstdint>
#include "argument.h"

namespace JitFFI
{
	extern const ArgTypeUnit atu_bool;

	extern const ArgTypeUnit atu_char;
	extern const ArgTypeUnit atu_schar;
	extern const ArgTypeUnit atu_uchar;
	extern const ArgTypeUnit atu_wchar;

	extern const ArgTypeUnit atu_int;
	extern const ArgTypeUnit atu_lint;
	extern const ArgTypeUnit atu_llint;
	extern const ArgTypeUnit atu_sint;

	extern const ArgTypeUnit atu_uint;
	extern const ArgTypeUnit atu_ulint;
	extern const ArgTypeUnit atu_ullint;
	extern const ArgTypeUnit atu_usint;

	extern const ArgTypeUnit atu_float;
	extern const ArgTypeUnit atu_double;

	extern const ArgTypeUnit atu_ldouble;

	extern const ArgTypeUnit atu_pointer;

	extern const ArgTypeUnit atu_size;

	extern const ArgTypeUnit atu_int8;
	extern const ArgTypeUnit atu_int16;
	extern const ArgTypeUnit atu_int32;
	extern const ArgTypeUnit atu_int64;

	extern const ArgTypeUnit atu_uint8;
	extern const ArgTypeUnit atu_uint16;
	extern const ArgTypeUnit atu_uint32;
	extern const ArgTypeUnit atu_uint64;
}
#endif
