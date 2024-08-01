#ifndef __LIB_FAT_POINTER_H__
#define __LIB_FAT_POINTER_H__

#ifdef __cplusplus
#include <type_traits>

// From: https://stackoverflow.com/a/21121104
namespace detail {
	template <typename T>
	struct is_complete_helper {
		template <typename U>
		static auto test(U*)  -> std::integral_constant<bool, sizeof(U) == sizeof(U)>;
		static auto test(...) -> std::false_type;
		using type = decltype(test((T*)0));
	};

	template <typename T>
	struct is_complete : is_complete_helper<T>::type {};
	template <typename T>
	static constexpr bool is_complete_v = is_complete<T>::value;

	template<typename T>
	struct completed_sizeof{};
	template<typename T>
	requires(is_complete_v<T>)
	struct completed_sizeof<T> : std::integral_constant<std::size_t, sizeof(T)> {};
	template<typename T>
	requires(!is_complete_v<T>)
	struct completed_sizeof<T> : std::integral_constant<std::size_t, 0> {};

	template <typename T>
	static constexpr std::size_t completed_sizeof_v = completed_sizeof<T>::value;
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#ifndef FP_ALLOCATION_FUNCTION
	/**
	* @brief Function used by libFP to allocate memory
	* @param p pointer to reallocate
	* @note if \p p is null, then new memory is expected to be allocated, otherwise a reallocation and data copy is expected to occur
	* @param size the size in raw bytes to allocate
	* @note if \p size is 0, then the memory pointed to by \p p should be freed
	* @note if \p size is the same as the size of the current allocation of \p p it is valid to simply return \p p
	* @return a pointer to the new allocation or null if the allocation was freed
	*/
	void* __fp_alloc_default_impl(void* p, size_t size)
	#ifdef FP_IMPLEMENTATION
	{
		if(size == 0) {
			free(p);
			return nullptr;
		}

		return realloc(p, size);
	}
	#else
	;
	#endif
#define FP_ALLOCATION_FUNCTION __fp_alloc_default_impl
#endif

enum FP_MagicNumbers {
	FP_MAGIC_NUMBER = 0xFE00,
	FP_HEAP_MAGIC_NUMBER = 0xFEFE,
	FP_STACK_MAGIC_NUMBER = 0xFEFF
};

struct __FatPointerHeader {
	uint16_t magic;
	size_t size;
#ifndef __cplusplus
	uint8_t data[];
#else
	uint8_t data[8];
#endif
};
#ifndef __cplusplus
	#define FP_HEADER_SIZE sizeof(struct __FatPointerHeader)
#else
	static constexpr size_t FP_HEADER_SIZE = sizeof(__FatPointerHeader) - detail::completed_sizeof_v<decltype(__FatPointerHeader{}.data)>;
#endif

#define FP_CONCAT_(x,y) x##y
#define FP_CONCAT(x,y) FP_CONCAT_(x,y)

#ifndef __cplusplus
#define FP_TYPE_OF(x) typeof(x)
#else
#include <type_traits>
#define FP_TYPE_OF(x) std::remove_reference<decltype(x)>::type
#endif

// Warning: this function may call alloca! This means it should not be used inside a loop if at all possible! Preallocate instead!
#ifndef __cplusplus
#define fp_salloc_void(_typesize, _size) ((void*)(((uint8_t*)&(struct {\
		uint16_t magic;\
		size_t size;\
		uint8_t data[_typesize * (_size) + 1];\
	}) {\
		.magic = FP_STACK_MAGIC_NUMBER,\
		.size = (_size),\
		.data = {0}\
	}) + FP_HEADER_SIZE))
#elif defined _MSC_VER
#include <malloc.h>
#define fp_salloc_void(_typesize, _size) ((void*)(((uint8_t*)&((*(__FatPointerHeader*)_alloca(FP_HEADER_SIZE + _typesize * _size)) = \
    __FatPointerHeader {\
		.magic = FP_STACK_MAGIC_NUMBER,\
		.size = (_size),\
	})) + FP_HEADER_SIZE))
#else
#define fp_salloc_void(_typesize, _size) ((void*)(((uint8_t*)&((*(__FatPointerHeader*)alloca(FP_HEADER_SIZE + _typesize * _size)) = \
    __FatPointerHeader {\
		.magic = FP_STACK_MAGIC_NUMBER,\
		.size = (_size),\
	})) + FP_HEADER_SIZE))
#endif
#define fp_salloc(type, _size) ((type*)fp_salloc_void(sizeof(type), _size))

struct __FatPointerHeader* __fp_header(const void* _p)
#ifdef FP_IMPLEMENTATION
{
	uint8_t* p = (uint8_t*)_p;
	p -= FP_HEADER_SIZE;
	return (struct __FatPointerHeader*)p;
}
#else
;
#endif


void* __fp_alloc(void* _p, size_t _size)
#ifdef FP_IMPLEMENTATION
{
	if(_p == nullptr && _size == 0) return nullptr;
	if(_size == 0)
		return FP_ALLOCATION_FUNCTION(__fp_header(_p), 0);

	_p = _p == nullptr ? _p : __fp_header(_p);
	size_t size = FP_HEADER_SIZE + _size + 1;
	uint8_t* p = (uint8_t*)FP_ALLOCATION_FUNCTION(_p, size);
	p += FP_HEADER_SIZE;
	if(!p) return 0;
	auto h = __fp_header(p);
	h->magic = FP_HEAP_MAGIC_NUMBER;
	h->size = _size;
	h->data[_size] = 0;
	return p;
}
#else
;
#endif

void* __fp_malloc(size_t size)
#ifdef FP_IMPLEMENTATION
{ return __fp_alloc(nullptr, size); }
#else
;
#endif

#ifndef __cplusplus
	#define fp_malloc(type, _size) ((type*)((uint8_t*)(*__fp_header(__fp_malloc(sizeof(type) * (_size))) = (struct __FatPointerHeader){\
		.magic = FP_HEAP_MAGIC_NUMBER,\
		.size = (_size),\
	}).data))

	#define fp_realloc(type, p, _size) ((type*)((uint8_t*)(*__fp_header(__fp_alloc((p), sizeof(type) * (_size))) = (struct __FatPointerHeader){\
		.magic = FP_HEAP_MAGIC_NUMBER,\
		.size = (_size),\
	}).data))
#else
	#define fp_malloc(type, _size) ((type*)((uint8_t*)(*__fp_header(__fp_malloc(sizeof(type) * (_size))) = __FatPointerHeader{\
		.magic = FP_HEAP_MAGIC_NUMBER,\
		.size = (_size),\
	}).data))

	#define fp_realloc(type, p, _size) ((type*)((uint8_t*)(*__fp_header(__fp_alloc((p), sizeof(type) * (_size))) = __FatPointerHeader{\
		.magic = FP_HEAP_MAGIC_NUMBER,\
		.size = (_size),\
	}).data))
#endif

bool is_fp(const void* p)
#ifdef FP_IMPLEMENTATION
{
	if(p == nullptr) return false;
	auto h = __fp_header(p);
	return (h->magic & 0xFF00) == FP_MAGIC_NUMBER && h->size > 0;
}
#else
;
#endif

size_t fp_magic_number(const void* p)
#ifdef FP_IMPLEMENTATION
{
	if(p == nullptr) return false;
	auto h = __fp_header(p);
	return h->magic;
}
#else
;
#endif

bool fp_is_stack_allocated(const void* p)
#ifdef FP_IMPLEMENTATION
{
	return fp_magic_number(p) == FP_STACK_MAGIC_NUMBER;
}
#else
;
#endif

bool fp_is_heap_allocated(const void* p)
#ifdef FP_IMPLEMENTATION
{
	return fp_magic_number(p) == FP_HEAP_MAGIC_NUMBER;
}
#else
;
#endif

void fp_free(void* p)
#ifdef FP_IMPLEMENTATION
{ __fp_alloc(p, 0); }
#else
;
#endif

#define fp_free_and_null(p) (fp_free(p), p = NULL)

size_t fp_length(const void* p)
#ifdef FP_IMPLEMENTATION
{
	if(!is_fp(p)) return 0;
	return __fp_header(p)->size;
}
#else
;
#endif

size_t fp_size(const void* p)
#ifdef FP_IMPLEMENTATION
{ return fp_length(p); }
#else
;
#endif

bool fp_empty(const void* p)
#ifdef FP_IMPLEMENTATION
{ return fp_length(p) == 0; }
#else
;
#endif

#define fp_iterate_named(a, iter) for(auto __fp_start = (a), iter = __fp_start; iter < __fp_start + fp_length(__fp_start); ++iter)
#define fp_iterate(a) fp_iterate_named(a, i)

#ifdef __cplusplus
}

template<typename T>
struct __fp_view {
	T* data;
	size_t size;

	template<typename To>
	explicit operator __fp_view<To>() { return {(To*)data, size}; }
};
#define fp_view(type) __fp_view<type>
typedef fp_view(void) fp_void_view;

#if !(defined _MSC_VER || defined __APPLE__)
extern "C" {
#endif // _MSC_VER || __APPLE__
#else
struct __fp_view {
	void* data;
	size_t size;
};

#define fp_view(type) struct __fp_view
typedef fp_view(void) fp_void_view;
#endif

fp_void_view __fp_make_view(void* p, size_t type_size, size_t start, size_t length)
#ifdef FP_IMPLEMENTATION
{
	assert(start + length <= fp_size(p));
#ifdef __cplusplus
	return fp_void_view
#else
	return (fp_void_view)
#endif
		{((uint8_t*)p) + type_size * start, length};
}
#else
;
#endif
#define fp_view_make(type, p, start, length) ((fp_view(type))__fp_make_view((p), sizeof(*(p)), (start), (length)))
#define fp_view_make_full(type, p) fp_view_make(type, p, 0, fp_size(p))
#define fp_view_make_start_end(type, p, start, end) fp_view_make(type, p, (start), ((end) - (start) + 1))

#define fp_view_length(view) ((view).size)
#define fp_view_size(view) fp_view_length(view)
#define fp_view_data_void(view) ((void*)(view).data)
#define fp_view_data(type, view) ((type*)fp_view_data_void(view))
#define fp_view_access(type, view, index) (assert((index) < fp_view_length(view)), fp_view_data(type, view) + (index))
#define fp_view_subscript(type, view, index) fp_view_access(type, view, index)

fp_void_view __fp_make_subview(fp_void_view view, size_t type_size, size_t start, size_t length)
#ifdef FP_IMPLEMENTATION
{
	assert(start + length <= fp_view_size(view));
#ifdef __cplusplus
	return fp_void_view
#else
	return (fp_void_view)
#endif
		{((uint8_t*)fp_view_data_void(view)) + type_size * start, length};
}
#else
;
#endif
#define fp_view_subview(type, view, start, length) ((fp_view(type))__fp_make_subview((fp_void_view)(view), sizeof(type), (start), (length)))
#define fp_view_subview_start_end(type, view, start, end) fp_view_subview(type, view, (start), ((end) - (start) + 1))

#define fp_view_iterate_named(type, view, iter) for(type* __fp_start = fp_view_data(type, view), *iter = __fp_start; iter < __fp_start + fp_view_length(view); ++iter)
#define fp_view_iterate(type, view) fp_view_iterate_named(type, (view), i)

#if (defined __cplusplus) && !(defined _MSC_VER || defined __APPLE__)
}
#endif // __cplusplus && !_MSC_VER 

#endif

// #include <stdio.h>

// int main() {
// 	// int* arr = fp_salloc(int, 20);
// 	int* arr = fp_malloc(int, 20);
// 	arr = fp_realloc(int, arr, 25);
// 	arr[20] = 6;

// 	printf(is_fp(arr) ? "true\n" : "false\n");
// 	printf("%lu\n", fp_length(arr));
// 	printf("%d\n", arr[20]);
// 	printf("%lu\n", sizeof(struct __FatPointerHeader));

// 	fp_free(arr);
// }