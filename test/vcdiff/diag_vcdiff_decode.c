/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/bytevector.h"
#include "diagonal/port.h"
#include "diagonal/hash.h"
#include "diagonal/vcdiff.h"
#include "diagonal/private/filesystem.h"

static void
decode3(const char *source, const char *input, const char *output)
{
	struct diag_vcdiff_context *context;
	struct diag_vcdiff *vcdiff;
	struct diag_vcdiff_vm *vm;
	uint8_t *expected;
	uint32_t i;

	context = diag_vcdiff_context_new_path(input);
	context->compatibility = 1;
	vcdiff = diag_vcdiff_read(context);
	if (!vcdiff) {
		printf("failed to read\n");
		exit(EXIT_FAILURE);
	}
	vm = diag_vcdiff_vm_new_path(source);
	if (!vm) {
		printf("failed to create vm with %s\n", source);
		exit(EXIT_FAILURE);
	}
	if (!diag_vcdiff_decode(vm, vcdiff)) {
		printf("failed to decode\n");
		exit(EXIT_FAILURE);
	}
	struct diag_mmap *mm = diag_mmap_file(output, DIAG_MMAP_RO);
	if (!mm) diag_fatal("could not map file: %s", output);
	if (vm->s_target != (uint32_t)mm->size) {
		printf("mismatch found: expected size %d, but s_target %d\n", (uint32_t)mm->size, vm->s_target);
		exit(EXIT_FAILURE);
	}
	expected = (uint8_t *)mm->addr;
	for (i = 0; i < vm->s_target; i++) {
		if (vm->target[i] != expected[i]) {
			printf("mismatch found: %c expected, but %c\n", expected[i], vm->target[i]);
			exit(EXIT_FAILURE);
		}
	}
	diag_munmap(mm);
	diag_vcdiff_vm_destroy(vm);
	diag_vcdiff_destroy(vcdiff);
	diag_vcdiff_context_destroy(context);
}

static void
decode2(const char *input, const char *output)
{
	decode3(NULL, input, output);
}

#define TEST_VCDIFF(path) \
	DIAGONAL_TEST_DIR "/vcdiff/" #path
#define DECODE3(source, input, output) \
	decode3(TEST_VCDIFF(source), TEST_VCDIFF(input), TEST_VCDIFF(output))
#define DECODE2(input, output) \
	decode2(TEST_VCDIFF(input), TEST_VCDIFF(output))

static void
fail_to_decode(const char *input)
{
	struct diag_vcdiff_context *context;
	struct diag_vcdiff *vcdiff;
	struct diag_vcdiff_vm *vm;

	context = diag_vcdiff_context_new_path(input);
	context->compatibility = 1;
	vcdiff = diag_vcdiff_read(context);
	if (!vcdiff) exit(EXIT_FAILURE);
	vm = diag_vcdiff_vm_new(0, NULL);
	if (!vm) exit(EXIT_FAILURE);
	vcdiff->windows[0] = NULL; /* damaged */
	if (diag_vcdiff_decode(vm, vcdiff) != NULL) exit(EXIT_FAILURE);
	diag_vcdiff_vm_destroy(vm);
	diag_vcdiff_destroy(vcdiff);
	diag_vcdiff_context_destroy(context);
}

#define FAIL_TO_DECODE(input) \
	fail_to_decode(TEST_VCDIFF(input))

int main(void)
{
	DECODE2(empty.vcdiff, empty);
	DECODE2(hello.vcdiff, hello);
	DECODE3(rfc3284.txt, rfc3284.txt.vcdiff.1, rfc3284.txt);
	DECODE3(rfc3284.txt.head, rfc3284.txt.vcdiff.2, rfc3284.txt);
	FAIL_TO_DECODE(hello.vcdiff);
	return EXIT_SUCCESS;
}
