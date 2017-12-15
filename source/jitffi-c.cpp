#include "jitffi.h"
#include "jitffi-c.h"
#include "platform.h"

extern "C"
{
	struct jitffi_jfp : public JitFFI::JitFuncPool {
		jitffi_jfp(size_t size, jitffi_jfpac access)
			: JitFuncPool(size, JitFuncPool::Access(access)) {}
	};
	struct jitffi_jf : public JitFFI::JitFunc {
		jitffi_jf(jitffi_jfp *jfp)
			: JitFunc(*(JitFFI::JitFuncPool*)jfp) {}
	};
	struct jitffi_jfc : public JitFFI::JitFuncCreater {
		jitffi_jfc(jitffi_jf *jf)
			: JitFuncCreater(*(JitFFI::JitFunc*)jf) {}
	};
	struct jitffi_argtype : public JitFFI::ArgTypeUnit {
		jitffi_argtype(const JitFFI::ArgTypeUnit &atu)
			: JitFFI::ArgTypeUnit(atu) {}
	};
	struct jitffi_arginfo : public JitFFI::ArgumentInfo {
		jitffi_arginfo(const JitFFI::ArgumentInfo &ai)
			: JitFFI::ArgumentInfo(ai) {}
	};

	jitffi_jfp* jitffi_create_jfp(size_t size, jitffi_jfpac access) {
		return new jitffi_jfp(size, access);
	}
	jitffi_jf* jitffi_create_jf(jitffi_jfp *jfp) {
		return new jitffi_jf(jfp);
	}
	jitffi_jfc* jitffi_create_jfc(jitffi_jf *jf) {
		return new jitffi_jfc(jf);
	}
	void jitffi_release_jfp(jitffi_jfp *jfp) {
		delete jfp;
	}
	void jitffi_release_jf(jitffi_jf *jf) {
		delete jf;
	}
	void jitffi_release_jfc(jitffi_jfc *jfc) {
		delete jfc;
	}
	void jitffi_release_arginfo(jitffi_arginfo *ai) {
		delete ai;
	}

	jitffi_arginfo* jitffi_create_arginfo(const jitffi_argtype *restype, const jitffi_argtype_ptr typelist[]) {
		JitFFI::ArgTypeList atl;
		for (auto p = typelist; *p != nullptr; ++p) {
			atl.push_back(*p);
		}
		
		return new jitffi_arginfo(JitFFI::CurrABI::GetArgInfo(*restype, atl));
	}

	void* jitffi_compile(jitffi_jfc *jfc, const jitffi_arginfo *info, void *func, const void * datalist[]) {
		if (datalist == nullptr) {
			JitFFI::CurrABI::CreateCaller(*jfc, *info, func);
		}
		else {
			JitFFI::ArgDataList adl;
			for (auto p = datalist; *p != nullptr; ++p) {
				adl.push_back(*p);
			}

			JitFFI::CurrABI::CreateCaller(*jfc, *info, func, adl);
		}

		return jitffi_getfunc(jfc);
	}

	void* jitffi_getfunc(jitffi_jfc *jfc) {
		return jfc->get().get();
	}


	// Types

#define _define_argtype(type) const jitffi_argtype *jitffi_type_##type = (const jitffi_argtype *)&(JitFFI::CurrABI::atu_##type);

	_define_argtype(void);

	_define_argtype(bool);

	_define_argtype(char);
	_define_argtype(schar);
	_define_argtype(uchar);

	_define_argtype(int);
	_define_argtype(short);
	_define_argtype(lint);
	_define_argtype(llint);

	_define_argtype(uint);
	_define_argtype(ushort);
	_define_argtype(ulint);
	_define_argtype(ullint);

	_define_argtype(float);
	_define_argtype(double);

	_define_argtype(ldouble);

	_define_argtype(pointer);

	_define_argtype(size);

	_define_argtype(int8);
	_define_argtype(int16);
	_define_argtype(int32);
	_define_argtype(int64);

	_define_argtype(uint8);
	_define_argtype(uint16);
	_define_argtype(uint32);
	_define_argtype(uint64);

#undef _define_argtype
}
