#include "test.h"

#include "diagonal.h"
#include "diagonal/port.h"
#include "diagonal/hash.h"
#include "diagonal/vcdiff.h"

int
main()
{
	diag_vcdiff_t *vcdiff;
	diag_vcdiff_context_t *context;
	diag_vcdiff_vm_t *vm;
	uint8_t *target;

#define HELLO_VCDIFF DIAGONAL_TEST_DIR "/vcdiff/hello.vcdiff"
	context = diag_vcdiff_context_new_path(HELLO_VCDIFF);
	if (!context) {
		return 1;
	}
	context->compatibility = 1;
	vcdiff = diag_vcdiff_read(context);
	if (!vcdiff) {
		return 2;
	}
	assert(vcdiff->num_windows == 1);
#define CDUMP(x) printf("context->" #x ": %d\n", context->x)
	CDUMP(compatibility);
#undef CDUMP
#define VDUMP(x) printf("vcdiff->" #x ": %d\n", vcdiff->x)
	VDUMP(version);
	VDUMP(indicator);
	VDUMP(compressor_id);
	VDUMP(length_code_table_data);
#undef VDUMP
#define WDUMP(x) printf("vcdiff->windows[0]->" #x ": %d\n", vcdiff->windows[0]->x)
	WDUMP(indicator);
	WDUMP(length_source_segment);
	WDUMP(position_source_segment);
#undef WDUMP
#define DDUMP(x) printf("vcdiff->windows[0]->delta->" #x ": %d\n", vcdiff->windows[0]->delta->x)
	DDUMP(length);
	DDUMP(s_window);
	DDUMP(indicator);
	DDUMP(length_data);
	DDUMP(length_inst);
	DDUMP(length_addr);
#undef DDUMP
	vm = diag_vcdiff_vm_new_path(HELLO_VCDIFF);
	target = diag_vcdiff_decode(vm, vcdiff);
	assert(target && vm->s_target == 14);
	assert(strcmp("hello, world.\n", (const char *)target) == 0);
	diag_vcdiff_vm_destroy(vm);
	diag_vcdiff_context_destroy(context);
	return EXIT_SUCCESS;
}
