#include "test.h"

#include "diagonal.h"
#include "diagonal/port.h"
#include "diagonal/hash.h"
#include "diagonal/vcdiff.h"

static void
decode3(const char *source, const char *input, const char *output)
{
	diag_vcdiff_context_t *context;
	diag_vcdiff_t *vcdiff;
	diag_vcdiff_vm_t *vm;
	int fd, r;
	struct stat st;
	uint8_t *expected;
	uint32_t i;

	context = diag_vcdiff_context_new_path(input);
	context->compatibility = 1;
	vcdiff = diag_vcdiff_read(context);
	if (!vcdiff) exit(EXIT_FAILURE);
	vm = diag_vcdiff_vm_new_path(source);
	if (!vm) exit(EXIT_FAILURE);
	if (!diag_vcdiff_decode(vm, vcdiff)) exit(EXIT_FAILURE);
	fd = open(output, O_RDONLY);
	if (fd < 0) exit(EXIT_FAILURE);
	r = fstat(fd, &st);
	if (r < 0) exit(EXIT_FAILURE);
	if (vm->s_target != (uint32_t)st.st_size) exit(EXIT_FAILURE);
	expected = (uint8_t *)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (!expected) exit(EXIT_FAILURE);
	for (i = 0; i < vm->s_target; i++) {
		if (vm->target[i] != expected[i]) exit(EXIT_FAILURE);
	}
	munmap(expected, st.st_size);
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
	diag_vcdiff_context_t *context;
	diag_vcdiff_t *vcdiff;
	diag_vcdiff_vm_t *vm;

	context = diag_vcdiff_context_new_path(input);
	context->compatibility = 1;
	vcdiff = diag_vcdiff_read(context);
	if (!vcdiff) exit(EXIT_FAILURE);
	vm = diag_vcdiff_vm_new_path(NULL);
	if (!vm) exit(EXIT_FAILURE);
	vcdiff->windows[0] = NULL; /* damaged */
	if (diag_vcdiff_decode(vm, vcdiff) != NULL) exit(EXIT_FAILURE);
	diag_vcdiff_vm_destroy(vm);
	diag_vcdiff_destroy(vcdiff);
	diag_vcdiff_context_destroy(context);
}

#define FAIL_TO_DECODE(input) \
	fail_to_decode(TEST_VCDIFF(input))

int
main()
{
	DECODE2(hello.vcdiff, hello);
	DECODE3(rfc3284.txt, rfc3284.txt.vcdiff.1, rfc3284.txt);
	DECODE3(rfc3284.txt.head, rfc3284.txt.vcdiff.2, rfc3284.txt);
	FAIL_TO_DECODE(hello.vcdiff);
	return EXIT_SUCCESS;
}
