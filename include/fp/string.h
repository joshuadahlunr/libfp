#ifndef __LIB_FAT_POINTER_STRING_H__
#define __LIB_FAT_POINTER_STRING_H__

#include "dynarray.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define fp_string char*
typedef fp_view(char) fp_string_view;

#ifndef __cplusplus
#define fp_string_view_literal(data, size) ((fp_string_view) {(data), (size)})
#else
#define fp_string_view_literal(data, size) (fp_string_view{(data), (size)})
#endif
#define fp_string_view_null fp_string_view_literal(NULL, 0)

#define fp_string_capacity fpda_capacity

void fp_string_free(fp_string str) FP_NOEXCEPT
#ifdef FP_IMPLEMENTATION
{
	if(is_fpda(str)) fpda_free(str);
	else if(is_fp(str) && fp_is_heap_allocated(str)) fp_free(str);
	else FP_ALLOCATION_FUNCTION(str, 0);
}
#else
;
#endif

#define fp_string_free_and_null(str) (fp_string_free(str), str = NULL)

#if defined __cplusplus && (defined _MSC_VER || defined __APPLE__)
}
#endif // __cplusplus && _MSC_VER

FP_CONSTEXPR inline static fp_string_view fp_string_to_view(fp_string str) FP_NOEXCEPT {
	if(!str) return fp_string_view_null;
	if(is_fp(str)) return fp_view_make_full(char, (fp_string)str);
	return fp_string_view_literal((fp_string)str, strlen(str));
}
FP_CONSTEXPR inline static const fp_string_view fp_string_to_view_const(const fp_string str) FP_NOEXCEPT {
	if(!str) return fp_string_view_null;
	if(is_fp(str)) return fp_view_make_full(char, (fp_string)str);
	return fp_string_view_literal((fp_string)str, strlen(str));
}

#if defined __cplusplus && (defined _MSC_VER || defined __APPLE__)
extern "C" {
#endif // __cplusplus && _MSC_VER

#define fp_string_view_length(view) fp_view_length(view)
FP_CONSTEXPR inline static size_t fp_string_length(const fp_string str) FP_NOEXCEPT {
	auto view = fp_string_to_view_const(str);
	return fp_string_view_length(view);
}

inline static fp_string fp_string_view_make_dynamic(const fp_string_view view) FP_NOEXCEPT {
	fp_string out = NULL;
	size_t size = fp_view_size(view);
	fpda_resize(out, size);
	memcpy(out, fp_view_data(char, view), size);
	return out;
}
inline static fp_string fp_string_make_dynamic(const fp_string str) FP_NOEXCEPT { return fp_string_view_make_dynamic(fp_string_to_view_const(str)); }

#define fp_string_promote_literal fp_string_make_dynamic
#define fp_string_clone fp_string_make_dynamic

FP_CONSTEXPR inline static int fp_string_view_compare(const fp_string_view a, const fp_string_view b) FP_NOEXCEPT {
	size_t sizeA = fp_view_length(a);
	size_t sizeB = fp_view_length(b);
	if(sizeA != sizeB) return sizeA - sizeB;
	return memcmp(fp_view_data(char, a), fp_view_data(char, b), sizeA);
}
FP_CONSTEXPR inline static int fp_string_compare(const fp_string a, const fp_string b) FP_NOEXCEPT { return fp_string_view_compare(fp_string_to_view_const(a), fp_string_to_view_const(b)); }

#define fp_string_view_equal(a, b) (fp_string_view_compare((a), (b)) == 0)
#define fp_string_equal(a, b) (fp_string_compare((a), (b)) == 0)

inline static fp_string __fp_string_view_concatenate_inplace(fp_string* a, const fp_string_view b) FP_NOEXCEPT {
	assert(is_fpda(*a) || !*a);
	size_t sizeA = fp_string_length(*a);
	size_t sizeB = fp_view_length(b);
	fpda_resize(*a, sizeA + sizeB);

	memcpy(*a + sizeA, fp_view_data(char, b), sizeB);
	return *a;
}
inline static fp_string __fp_string_concatenate_inplace(fp_string* a, const fp_string b) FP_NOEXCEPT { return __fp_string_view_concatenate_inplace(a, fp_string_to_view_const(b)); }

#define fp_string_view_concatenate_inplace(a, b) __fp_string_view_concatenate_inplace(&a, (b))
#define fp_string_concatenate_inplace(a, b) __fp_string_concatenate_inplace(&a, (b))

inline static fp_string fp_string_view_concatenate(const fp_string_view a, const fp_string_view b) FP_NOEXCEPT {
	fp_dynarray(char) out = NULL;
	size_t sizeA = fp_view_length(a);
	size_t sizeB = fp_view_length(b);
	fpda_resize(out, sizeA + sizeB);

	memcpy(out, fp_view_data(char, a), sizeA);
	memcpy(out + sizeA, fp_view_data(char, b), sizeB);
	return out;
}
inline static fp_string fp_string_concatenate(const fp_string a, const fp_string b) FP_NOEXCEPT { return fp_string_view_concatenate(fp_string_to_view_const(a), fp_string_to_view_const(b)); }

inline static fp_string fp_string_view_concatenate_n(size_t count, ...) FP_NOEXCEPT {
	fp_dynarray(char) out = NULL;
	va_list args;
	va_start(args, count);

	for(size_t i = count, sum = 0; --i; ) {
		fp_string_view view = va_arg(args, fp_string_view);
		size_t size = fp_view_size(view);
		fpda_grow(out, size);
		memcpy(out + sum, fp_view_data(char, view), size);
		sum += size;
	}

	va_end(args);
	return out;
}
fp_string fp_string_concatenate_n(size_t count, ...) FP_NOEXCEPT
#ifdef FP_IMPLEMENTATION
{
	fp_dynarray(char) out = NULL;
	va_list args;
	va_start(args, count);

	for(size_t i = count, sum = 0; --i; ) {
		fp_string str = va_arg(args, fp_string);
		size_t size = fp_string_length(str);
		fpda_grow(out, size);
		memcpy(out + sum, str, size);
		sum += size;
	}

	va_end(args);
	return out;
}
#else
;
#endif

inline static fp_string fp_string_append_impl(fp_string* str, char c) FP_NOEXCEPT {
	assert(is_fpda(*str));
	size_t size = fpda_length(*str);

	fpda_push_back(*str, c);
	(*str)[size + 1] = 0;
	return *str;
}
#define fp_string_append(str, c) fp_string_append_impl(&str, (c))

inline static fp_string fp_string_vformat(const fp_string format, va_list args) FP_NOEXCEPT {
	va_list argsSize;
	va_copy(argsSize, args);
	size_t size = vsnprintf(NULL, 0, format, argsSize);
	va_end(argsSize);

	fp_dynarray(char) out = NULL;
	fpda_resize(out, size);

	vsnprintf(out, fpda_size(out) + 1, format, args);
	return out;
}
inline static fp_string fp_string_view_vformat(const fp_string_view format, va_list args) FP_NOEXCEPT {
	return fp_string_vformat(fp_string_view_make_dynamic(format), args);
}

inline static fp_string fp_string_format(const fp_string format, ...) FP_NOEXCEPT {
	va_list args;
	va_start(args, format);
	auto out = fp_string_vformat(format, args);
	va_end(args);
	return out;
}
inline static fp_string fp_string_view_format(const fp_string_view format, ...) FP_NOEXCEPT {
	va_list args;
	va_start(args, format);
	auto out = fp_string_view_vformat(format, args);
	va_end(args);
	return out;
}

#ifdef __cplusplus
}
#endif

#if defined __cplusplus && __cpp_lib_string_view >= 201606L
#include <string_view>

FP_CONSTEXPR inline static std::string_view fp_string_view_to_std(fp_string_view view) {
	return std::string_view{fp_view_data(char, view), fp_view_size(view)};
}
FP_CONSTEXPR inline static std::string_view fp_string_to_std(fp_string str) {
	return fp_string_view_to_std(fp_string_to_view(str));
}

FP_CONSTEXPR inline static fp_string_view fp_std_to_string_view(std::string_view view) {
	return fp_string_view_literal((char*)view.data(), view.size());
}
#endif

#endif // __LIB_FAT_POINTER_STRING_H__