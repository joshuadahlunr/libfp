#include <fp/pointer.h>
#include <fp/dynarray.h>

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

void check_dynarray(void) {
	fp_dynarray(int) arr = nullptr;
	fpda_reserve(arr, 20);
	assert(fpda_capacity(arr) == 0); // NOTE: FPDAs aren't valid until they have had at least one element added!
	assert(fpda_size(arr) == 0);

	fpda_push_back(arr, 5);
	assert(fpda_capacity(arr) == 20);
	assert(fpda_size(arr) == 1);
	assert(arr[0] == 5);

	fpda_push_front(arr, 6);
	assert(fpda_capacity(arr) == 20);
	assert(fpda_size(arr) == 2);
	assert(arr[0] == 6);
	assert(arr[1] == 5);
	assert(*fpda_front(arr) == 6);
	assert(*fpda_back(arr) == 5);

	fpda_push_back(arr, 7);
	assert(fpda_capacity(arr) == 20);
	assert(fpda_size(arr) == 3);
	assert(arr[0] == 6);
	assert(arr[1] == 5);
	assert(arr[2] == 7);
	assert(*fpda_front(arr) == 6);
	assert(*fpda_back(arr) == 7);

	fpda_delete(arr, 1);
	assert(fpda_capacity(arr) == 20);
	assert(fpda_size(arr) == 2);
	assert(arr[0] == 6);
	assert(arr[1] == 7);

	fpda_swap(arr, 0, 1);
	assert(fpda_capacity(arr) == 20);
	assert(fpda_size(arr) == 2);
	assert(arr[0] == 7);
	assert(arr[1] == 6);

	fp_dynarray(int) arr2 = NULL;
	fpda_clone_to(arr2, arr);
	assert(arr2 != arr);
	assert(fpda_capacity(arr) == 20);
	assert(fpda_capacity(arr2) == 20);
	assert(fpda_size(arr) == 2);
	assert(fpda_size(arr2) == 2);
	assert(arr[0] == 7);
	assert(arr2[0] == 7);
	assert(arr[1] == 6);
	assert(arr2[1] == 6);

	fpda_shrink_to_fit(arr);
	assert(fpda_capacity(arr) == 2);
	assert(fpda_size(arr) == 2);
	assert(arr[0] == 7);
	assert(arr[1] == 6);

	fpda_free(arr);
	fpda_free(arr2);
} 