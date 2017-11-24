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
	// create_function_caller = (ArgumentInfo Func)                    -> function<void (void*, ArgDataList)>
	// create_function_caller = (ArgumentInfo Func ArgDataList)        -> function<void (void*)>
	// create_function_caller = (ArgumentInfo Func ArgDataListPartial) -> function<void (void*, ArgDataListPartial)>
	// create_function_caller = (ArgumentInfo)                         -> function<void (Func*, void*, ArgDataList)>
	// create_function_caller = (ArgumentInfo ArgDataList)             -> function<void (Func*, void*)>
	// create_function_caller = (ArgumentInfo ArgDataListPartial)      -> function<void (Func*, void*, ArgDataListPartial)>

#define _DECLARE_create_function_caller \
	ArgumentInfo get_argumentinfo(const ArgTypeUnit &restype, const ArgTypeList &atlist); \
	void create_function_caller(JitFuncCreater &jfc, const ArgumentInfo &argumentinfo, void *func, const ArgDataList &adlist); \
	void create_function_caller(JitFuncCreater &jfc, const ArgumentInfo &argumentinfo, void *func); \
	template <typename _FTy> \
	void create_function_caller(JitFuncCreater &jfc, const ArgumentInfo &argumentinfo, _FTy *func, const ArgDataList &adlist) { \
		create_function_caller(jfc, argumentinfo, (void*)func, adlist); \
	} \
	template <typename _FTy> \
	void create_function_caller(JitFuncCreater &jfc, const ArgumentInfo &argumentinfo, _FTy *func) { \
		create_function_caller(jfc, argumentinfo, (void*)func); \
	}

	namespace SysV64 { _DECLARE_create_function_caller }
	namespace MS64 { _DECLARE_create_function_caller }

#undef _DECLARE_create_function_caller
}
#endif
