// JitFFI Library
// * jitffi-c.h

#pragma once
#ifndef _JITFFI_C_H_
#define _JITFFI_C_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>

	enum jitffi_jfpac { jitffi_readonly, jitffi_readwrite };

	struct jitffi_jfp;     // JitFuncPool
	struct jitffi_jf;      // JitFunc
	struct jitffi_jfc;     // JitFuncCreater
	struct jitffi_argtype; // ArgTypeUnit
	struct jitffi_arginfo; // ArgumentInfo

#ifndef __cplusplus
	typedef struct jitffi_jfp * jitffi_jfp;
	typedef struct jitffi_jf * jitffi_jf;
	typedef struct jitffi_jfc * jitffi_jfc;
	typedef struct jitffi_argtype * jitffi_argtype;
	typedef struct jitffi_arginfo * jitffi_arginfo;
	typedef enum jitffi_jfpac jitffi_jfpac;
#endif

	typedef const jitffi_argtype * jitffi_argtype_ptr;

	typedef void(jitffi_f1)(void* dst);
	typedef void(jitffi_f2)(void* dst, const void* datalist[]);
	typedef void(jitffi_f3)(void* dst, const void *func, const void* datalist[]);

	// C++                    -> C
	// JitFuncCreater &       -> jitffi_jfc *
	// const ArgumentInfo &   -> const jitffi_arginfo *

	jitffi_jfp* jitffi_create_jfp(size_t size, jitffi_jfpac access);
	jitffi_jf* jitffi_create_jf(jitffi_jfp *jfp);
	jitffi_jfc* jitffi_create_jfc(jitffi_jf *jf);
	void jitffi_release_jfp(jitffi_jfp *jfc);
	void jitffi_release_jf(jitffi_jf *jfc);
	void jitffi_release_jfc(jitffi_jfc *jfc);
	void jitffi_release_arginfo(jitffi_arginfo *ai);

	jitffi_arginfo* jitffi_create_arginfo(const jitffi_argtype *restype, const jitffi_argtype_ptr typelist[]);

	void* jitffi_compile(jitffi_jfc *jfc, const jitffi_arginfo *info, void *func, const void * datalist[]);
	void* jitffi_getfunc(jitffi_jfc *jfc);

	// Types

	extern const jitffi_argtype* jitffi_type_void;

	extern const jitffi_argtype* jitffi_type_bool;

	extern const jitffi_argtype* jitffi_type_char;
	extern const jitffi_argtype* jitffi_type_schar;
	extern const jitffi_argtype* jitffi_type_uchar;

	extern const jitffi_argtype* jitffi_type_int;
	extern const jitffi_argtype* jitffi_type_short;
	extern const jitffi_argtype* jitffi_type_lint;
	extern const jitffi_argtype* jitffi_type_llint;

	extern const jitffi_argtype* jitffi_type_uint;
	extern const jitffi_argtype* jitffi_type_ushort;
	extern const jitffi_argtype* jitffi_type_ulint;
	extern const jitffi_argtype* jitffi_type_ullint;

	extern const jitffi_argtype* jitffi_type_float;
	extern const jitffi_argtype* jitffi_type_double;

	extern const jitffi_argtype* jitffi_type_ldouble;

	extern const jitffi_argtype* jitffi_type_pointer;

	extern const jitffi_argtype* jitffi_type_size;

	extern const jitffi_argtype* jitffi_type_int8;
	extern const jitffi_argtype* jitffi_type_int16;
	extern const jitffi_argtype* jitffi_type_int32;
	extern const jitffi_argtype* jitffi_type_int64;

	extern const jitffi_argtype* jitffi_type_uint8;
	extern const jitffi_argtype* jitffi_type_uint16;
	extern const jitffi_argtype* jitffi_type_uint32;
	extern const jitffi_argtype* jitffi_type_uint64;

#ifdef __cplusplus
}
#endif

#endif
