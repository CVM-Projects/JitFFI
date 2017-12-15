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
	// GetArgInfo  = (ArgTypeUnit ArgTypeList)              -   > ArgumentInfo
	// Compile     = (JFC ArgumentInfo Func)                    -> function<void (void*, ArgDataList)>
	// Compile     = (JFC ArgumentInfo Func ArgDataList)        -> function<void (void*)>
	// Compile     = (JFC ArgumentInfo Func ArgDataListPartial) -> function<void (void*, ArgDataListPartial)>
	// Compile     = (JFC ArgumentInfo)                         -> function<void (Func*, void*, ArgDataList)>
	// Compile     = (JFC ArgumentInfo ArgDataList)             -> function<void (Func*, void*)>
	// Compile     = (JFC ArgumentInfo ArgDataListPartial)      -> function<void (Func*, void*, ArgDataListPartial)>

	using f1 = void(void* dst);
	using f2 = void(void* dst, const void* datalist[]);
	using f3 = void(void* dst, const void *func);
	using f4 = void(void* dst, const void *func, const void* datalist[]);

#define _DECLARE_create_function_caller \
	ArgumentInfo GetArgInfo(const ArgTypeUnit &restype, const ArgTypeList &atlist); \
	void CreateCaller(JitFuncCreater &jfc, const ArgumentInfo &argumentinfo, void *func, const ArgDataList &adlist); \
	void CreateCaller(JitFuncCreater &jfc, const ArgumentInfo &argumentinfo, void *func); \
	template <typename _FTy> \
	f1* Compile(JitFuncCreater &jfc, const ArgumentInfo &argumentinfo, _FTy *func, const ArgDataList &adlist) { \
		CreateCaller(jfc, argumentinfo, (void*)func, adlist); \
		return jfc.get().func<f1>(); \
	} \
	template <typename _FTy> \
	f2* Compile(JitFuncCreater &jfc, const ArgumentInfo &argumentinfo, _FTy *func) { \
		CreateCaller(jfc, argumentinfo, (void*)func); \
		return jfc.get().func<f2>(); \
	}

	namespace SysV64 { _DECLARE_create_function_caller }
	namespace MS64 { _DECLARE_create_function_caller }

#undef _DECLARE_create_function_caller
}
#endif
