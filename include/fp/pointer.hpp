#pragma once

#include <utility>
#include <cstring>

#include "pointer.h"

namespace fp {

	template<typename T>
	struct view: public fp_view(T) {
		static view make(T* p, size_t start, size_t length) {
			assert(fp_size(p) >= start + length);
			return {fp_view_make(T, p, start, length)};
		}
		static view make_full(T* p) {
			assert(is_fp(p));
			return {fp_view_make_full(T, p)};
		}
		static view make_start_end(T* p, size_t start, size_t end) {
			assert(start < end);
			assert(fp_size(p) >= end);
			return {fp_view_make_start_end(T, p, start, end)};
		}

		inline T* data() { return fp_view_data(T, view_()); }
		inline const T* data() const { return fp_view_data(T, view_()); }

		inline size_t length() const { return fp_view_length(view_()); }
		inline size_t size() const { return fp_view_size(view_()); }
		inline bool empty() const { return size() == 0; }

		inline view subview(size_t start, size_t length) {
			assert(start + length <= size());
			return {fp_view_subview(T, view_(), start, length)};
		}
		inline view subview_start_end(size_t start, size_t end) {
			assert(start < end);
			assert(end <= size());
			return {fp_view_subview_start_end(T, view_(), start, end)};
		}

		inline T* begin() { return fp_view_begin(T, view_()); }
		inline T* end() { return fp_view_end(T, void_view()); }

		inline T& operator*() {
			assert(!empty());
			return *data();
		}
		inline const T& operator*() const {
			assert(!empty());
			return *data();
		}
		inline T* operator->() {
			assert(!empty());
			return data();
		}
		inline const T* operator->() const {
			assert(!empty());
			return data();
		}

		inline T& operator[](size_t i) {
			assert(!empty());
			assert(i < size());
			return data()[i];
		}
		inline const T& operator[](size_t i) const {
			assert(!empty());
			assert(i < size());
			return data()[i];
		}

	protected:
		inline fp_view(T) view_() { return *this; }
		inline const fp_view(T) view_() const { return *this; }
		inline fp_void_view void_view() { return *(fp_void_view*)this; }
		inline const fp_void_view void_view() const { return *(fp_void_view*)this; }
	};




	template<typename T, typename Derived>
	struct pointer_crtp {
		using view_t = fp::view<T>;

		inline T* data() { return ptr(); }
		inline const T* data() const { return ptr(); }
		inline operator T*() { return ptr(); }
		inline operator const T*() const { return ptr(); }

		inline bool is_fp() const { return ::is_fp(ptr()); }
		inline bool stack_allocated() const { return fp_is_stack_allocated(ptr()); }
		inline bool heap_allocated() const { return fp_is_heap_allocated(ptr()); }

		inline size_t length() const { return fp_length(ptr()); }
		inline size_t size() const { return fp_length(ptr()); }
		inline bool empty() const { return fp_empty(ptr()); }

		inline view_t view(size_t start, size_t length) { return view_t::make(ptr(), start, length); }
		inline view_t view_full() { return view_t::make_full(ptr()); }
		inline view_t full_view() { return view_t::make_full(ptr()); }
		inline view_t view_start_end(size_t start, size_t end) { return view_t::make_start_end(ptr(), start, end); }

		inline T& front() { return *fp_front(ptr()); }
		inline T& back() { return *fp_back(ptr()); }

		inline T* begin() { return fp_begin(ptr()); }
		inline const T* begin() const { return fp_begin(ptr()); }
		inline T* end() { return size() ? fp_end(ptr()) + 1 : begin(); }
		inline const T* end() const { return size() ? fp_end(ptr()) + 1 : begin(); }

		inline T& operator*() {
			assert(!empty());
			return *ptr();
		}
		inline const T& operator*() const {
			assert(!empty());
			return *ptr();
		}
		inline T* operator->() {
			assert(!empty());
			return ptr();
		}
		inline const T* operator->() const {
			assert(!empty());
			return ptr();
		}

		// inline T& operator[](size_t i) { // NOTE: These are emulated via implicit conversion to pointer
		// 	// assert(!empty());
		// 	assert(i < size());
		// 	return ptr()[i];
		// }
		// inline const T& operator[](size_t i) const {
		// 	// assert(!empty());
		// 	assert(i < size());
		// 	return ptr()[i];
		// }

	protected:
		inline T*& ptr() { return ((Derived*)this)->ptr(); }
		inline const T* const & ptr() const { return (((const Derived*)this)->ptr()); }
	};


	template<typename T>
	struct pointer: public pointer_crtp<T, pointer<T>> {
		T* raw;

		pointer(): raw(nullptr) {}
		pointer(T* ptr): raw(ptr) { assert(this->is_fp() || ptr == nullptr); }
		pointer(const pointer& o) = default;
		pointer(pointer&& o) = default;
		pointer& operator=(const pointer& o) = default;
		pointer& operator=(pointer&& o) = default;

		void free() { fp_free(raw); }
		void free_and_null() { fp_free_and_null(raw); }
		pointer& realloc(size_t new_count) { raw = fp_realloc(T, raw, new_count); return *this; }

		pointer clone() const { return fp_clone(raw); }

	protected:
		friend pointer_crtp<T, pointer<T>>;
		inline T*& ptr() { return raw; }
		inline const T* const & ptr() const { return raw; }
	};

	namespace raii {
		template<typename T>
		struct pointer: public fp::pointer<T> {
			using super = fp::pointer<T>;
			// using super::super;
			pointer(): super(nullptr) {}
			pointer(T* ptr): super(ptr) {}
			pointer(const super& o): super(o.clone()) {}
			pointer(super&& o): super(std::exchange(o.raw, nullptr)) {}
			pointer(const pointer& o): super(o.clone()) {}
			pointer(pointer&& o): super(std::exchange(o.raw, nullptr)) {}
			pointer& operator=(const pointer& o) { if(super::raw) super::free(); super::raw = o.clone(); return *this;}
			pointer& operator=(pointer&& o) { if(super::raw) super::free(); super::raw = std::exchange(o.raw, nullptr); return *this; }
			~pointer() { if(super::raw) super::free_and_null(); }
		};
	}

	template<typename T>
	inline pointer<T> malloc(size_t count = 1) {
		return fp_malloc(T, count);
	}
	template<typename T>
	inline pointer<T> realloc(pointer<T> ptr, size_t new_count) {
		return ptr.realloc(new_count);
	}
	template<typename T>
	inline void free(pointer<T> ptr) {
		ptr.free();
	}


	template<typename T, size_t N>
	struct array: public pointer_crtp<T, array<T, N>> {
		__FatPointerHeaderTruncated __header = {
			.magic = FP_STACK_MAGIC_NUMBER,
			.size = N,
		};
		T raw[N];

	protected:
		friend pointer_crtp<T, array<T, N>>;
		inline T*& ptr() {
			thread_local static auto tmp = &raw[0];
			tmp = &raw[0];
			return tmp;
		}
		inline const T* const & ptr() const { return const_cast<array*>(this)->ptr(); }
	};

}