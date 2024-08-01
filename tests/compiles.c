#include <fp/pointer.h>

void check_stack(void) {
	int* arr = fp_salloc(int, 20);
	arr[10] = 6;

	assert(is_fp(arr));
	assert(fp_is_stack_allocated(arr));
	assert(!fp_is_heap_allocated(arr));
	assert(fp_length(arr) == 20);
	assert(arr[10] == 6);
}

void check_heap(void) {
	int* arr = fp_malloc(int, 20);
	arr = fp_realloc(int, arr, 25);
	arr[20] = 6;

	assert(is_fp(arr));
	assert(!fp_is_stack_allocated(arr));
	assert(fp_is_heap_allocated(arr));
	assert(fp_length(arr) == 25);
	assert(arr[20] == 6);

	fp_free(arr);
}

void check_view(void) {
	int* arr = fp_salloc(int, 20);
	arr[10] = 6;

	fp_view(int) view = fp_view_make(int, arr, 10, 3);
	assert(fp_view_size(view) == 3);
	assert(*fp_view_access(int, view, 0) == 6);
	*fp_view_access(int, view, 1) = 8;
	*fp_view_access(int, view, 2) = 6;

	auto sub = fp_view_subview(int, view, 1, 1);
	assert(*fp_view_access(int, sub, 0) == 8);
	*fp_view_access(int, sub, 0) = 6;

	fp_view_iterate(int, view) assert(*i == 6);
}