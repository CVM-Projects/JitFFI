// JitFFI Library
// * jitfunc.h

#pragma once
#ifndef _JITFFI_JITFUNC_H_
#define _JITFFI_JITFUNC_H_
#include <memory>

namespace JitFFI
{
	class JitFuncPool
	{
	public:
		enum Access {
			ReadOnly,
			ReadWrite
		};

	public:
		explicit JitFuncPool(size_t _size, Access access = ReadWrite) :
			_size(_size),
			data(alloc(_size, access == ReadOnly), [=](void *dp) { free(dp, _size); }) {}

		explicit JitFuncPool(void *data, size_t _size = 0) :
			_size(_size),
			data(data, [](void *dp) {}) {}

		explicit JitFuncPool(const JitFuncPool &data, size_t _size) :
			_size(_size),
			data(data.get(), [](void *dp) {}) {}

		JitFuncPool(const JitFuncPool &) = default;

		template <typename T = void>
		T* get() const {
			return static_cast<T*>(data.get());
		}

		void unprotect() {
			unprotect(data.get(), _size);
		}
		void protect() {
			protect(data.get(), _size);
		}

		size_t size() const {
			return _size;
		}

	private:
		const size_t _size;
		std::shared_ptr<void> data;

	public:
		static void* alloc(size_t _size, bool readonly);
		static void protect(void *dp, size_t _size);
		static void unprotect(void *dp, size_t _size);
		static void free(void *dp, size_t _size);
	};

	class JitFunc
	{
		using byte = uint8_t;
	public:
		explicit JitFunc(JitFuncPool &jfp) : jfp(jfp) {}
		explicit JitFunc(JitFuncPool &&jfp) : jfp(jfp) {}
		explicit JitFunc(JitFuncPool &jfp, size_t offset, size_t size) : jfp(jfp.get<byte>() + offset, size) {}

		template <typename T>
		T* func() const {
			return reinterpret_cast<T*>(get());
		}

		template <typename T = void>
		T* get() const {
			return jfp.get<T>();
		}

		void unprotect() {
			jfp.unprotect();
		}
		void protect() {
			jfp.protect();
		}

		size_t size() const {
			return jfp.size();
		}

	private:
		JitFuncPool jfp;
	};
}
#endif
