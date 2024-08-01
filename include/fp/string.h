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

#define fp_string_capacity fpda_capacity

void fp_string_free(fp_string str)
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

fp_string_view fp_string_to_view(fp_string str)
#ifdef FP_IMPLEMENTATION
{
	if(!str) return {nullptr, 0};
	if(is_fp(str)) return fp_view_make_full(char, str);
	return {str, strlen(str)};
}
#else
;
#endif
const fp_string_view fp_string_to_view_const(const fp_string str)
#ifdef FP_IMPLEMENTATION
{
	if(!str) return {nullptr, 0};
	if(is_fp(str)) return fp_view_make_full(char, (fp_string)str);
	return {(fp_string)str, strlen(str)};
}
#else
;
#endif

#if defined __cplusplus && (defined _MSC_VER || defined __APPLE__)
extern "C" {
#endif // __cplusplus && _MSC_VER 

#define fp_string_length_view(view) fp_view_length(view)
size_t fp_string_length(const fp_string str)
#ifdef FP_IMPLEMENTATION
{
	auto view = fp_string_to_view_const(str);
	return fp_string_length_view(view);
}
#else
;
#endif

fp_string fp_string_make_view_dynamic(const fp_string_view view)
#ifdef FP_IMPLEMENTATION
{
	fp_string out = nullptr;
	size_t size = fp_view_size(view);
	fpda_resize(out, size);
	memcpy(out, fp_view_data(char, view), size);
	return out;
}
#else
;
#endif
fp_string fp_string_make_dynamic(const fp_string str)
#ifdef FP_IMPLEMENTATION
{ return fp_string_make_view_dynamic(fp_string_to_view_const(str)); }
#else
;
#endif

#define fp_string_promote_literal fp_string_make_dynamic
#define fp_string_clone fp_string_make_dynamic

int fp_string_compare_view(const fp_string_view a, const fp_string_view b)
#ifdef FP_IMPLEMENTATION
{
	size_t sizeA = fp_view_length(a);
	size_t sizeB = fp_view_length(b);
	if(sizeA != sizeB) return sizeA - sizeB;
	return memcmp(fp_view_data(char, a), fp_view_data(char, b), sizeA);
}
#else
;
#endif
int fp_string_compare(const fp_string a, const fp_string b)
#ifdef FP_IMPLEMENTATION
{ return fp_string_compare_view(fp_string_to_view_const(a), fp_string_to_view_const(b)); }
#else
;
#endif

fp_string fp_string_concatenate_inplace_view_impl(fp_string* a, const fp_string_view b)
#ifdef FP_IMPLEMENTATION
{
	assert(is_fpda(*a));
	size_t sizeA = fpda_length(*a);
	size_t sizeB = fp_view_length(b);
	fpda_resize(*a, sizeA + sizeB);

	memcpy(*a + sizeA, fp_view_data(char, b), sizeB);
	return *a;
}
#else
;
#endif
fp_string fp_string_concatenate_inplace_impl(fp_string* a, const fp_string b)
#ifdef FP_IMPLEMENTATION
{ return fp_string_concatenate_inplace_view_impl(a, fp_string_to_view_const(b)); }
#else
;
#endif
#define fp_string_concatenate_inplace_view(a, b) fp_string_concatenate_inplace_view_impl(&a, (b))
#define fp_string_concatenate_inplace(a, b) fp_string_concatenate_inplace_impl(&a, (b))

fp_string fp_string_concatenate_view(const fp_string_view a, const fp_string_view b)
#ifdef FP_IMPLEMENTATION
{
	fp_dynarray(char) out = nullptr;
	size_t sizeA = fp_view_length(a);
	size_t sizeB = fp_view_length(b);
	fpda_resize(out, sizeA + sizeB);

	memcpy(out, fp_view_data(char, a), sizeA);
	memcpy(out + sizeA, fp_view_data(char, b), sizeB);
	return out;
}
#else
;
#endif
fp_string fp_string_concatenate(const fp_string a, const fp_string b)
#ifdef FP_IMPLEMENTATION
{ return fp_string_concatenate_view(fp_string_to_view_const(a), fp_string_to_view_const(b)); }
#else
;
#endif

fp_string fp_string_append_impl(fp_string* str, char c) 
#ifdef FP_IMPLEMENTATION
{
	assert(is_fpda(*str));
	size_t size = fpda_length(*str);
	
	fpda_push_back(*str, c);
	(*str)[size + 1] = 0;
	return *str;
}
#else
;
#endif
#define fp_string_append(str, c) fp_string_append_impl(&str, (c))

fp_string fp_string_vformat(const fp_string format, va_list args)
#ifdef FP_IMPLEMENTATION
{
	va_list argsSize;
	va_copy(argsSize, args);
	size_t size = vsnprintf(nullptr, 0, format, argsSize);
	va_end(argsSize);

	fp_dynarray(char) out = nullptr;
	fpda_resize(out, size);

	vsnprintf(out, fpda_size(out) + 1, format, args);
	return out;
}
#else
;
#endif
fp_string fp_string_vformat_view(const fp_string_view format, va_list args)
#ifdef FP_IMPLEMENTATION
{
	return fp_string_vformat(fp_string_make_view_dynamic(format), args);
}
#else
;
#endif

fp_string fp_string_format(const fp_string format, ...)
#ifdef FP_IMPLEMENTATION
{
	va_list args;
	va_start(args, format);
	auto out = fp_string_vformat(format, args);
	va_end(args);
	return out;
}
#else
;
#endif

#ifdef __cplusplus
}
#endif

#endif // __LIB_FAT_POINTER_STRING_H__