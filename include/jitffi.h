// JitFFI Library
// * jitffi.h

#pragma once
#ifndef _JITFFI_H_
#define _JITFFI_H_
#include "platform.h"
#include "jitfunc.h"
#include "creater.h"
#include "argument.h"
#include "types.h"

namespace JitFFI
{
	// get_argumentinfo       = (ArgTypeUnit ArgTypeList)              -> ArgumentInfo
	// create_function_caller = (Func ArgumentInfo)                    -> function<void (void*, ArgDataList)>;
	// create_function_caller = (Func ArgumentInfo ArgDataList)        -> function<void (void*)>;
	// create_function_caller = (Func ArgumentInfo ArgDataListPartial) -> function<void (void*, ArgDataListPartial)>;

#define _DECLARE_create_function_caller \
	ArgumentInfo get_argumentinfo(const ArgTypeUnit &restype, const ArgTypeList &atlist); \
	void create_function_caller(JitFuncCreater &jfc, void *func, const ArgumentInfo &argumentinfo, const ArgDataList &adlist); \
	template <typename _FTy> \
	void create_function_caller(JitFuncCreater &jfc, _FTy *func, const ArgumentInfo &argumentinfo, const ArgDataList &adlist) { \
		create_function_caller(jfc, (void*)func, argumentinfo, adlist); \
	}

	namespace SysV64 { _DECLARE_create_function_caller }
	namespace MS64 { _DECLARE_create_function_caller }

#undef _DECLARE_create_function_caller
}
#endif
