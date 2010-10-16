#include "test.h"

#include "diagonal.h"
#include "diagonal/bytevector.h"
#include "diagonal/port.h"
#include "diagonal/hash.h"
#include "diagonal/vcdiff.h"

int
main()
{
	struct diag_vcdiff_pcode *pcodes;
	struct diag_vcdiff_script *script;
	uint8_t *result;
	diag_size_t size, i;

	pcodes = diag_calloc(2, sizeof(struct diag_vcdiff_pcode));
	pcodes[0].inst = DIAG_VCD_RUN;
	pcodes[0].size = 1;
	pcodes[0].attr.byte = 'a';
	pcodes[1].inst = DIAG_VCD_COPY;
	pcodes[1].size = 20;
	pcodes[1].attr.addr = 0;
	script = diag_malloc(sizeof(struct diag_vcdiff_script));
	script->source = NULL;
	script->s_pcodes = 2;
	script->pcodes = pcodes;
	result = diag_vcdiff_expand(script, &size);
	ASSERT_NOT_NULL(result);
	ASSERT_EQ_UINT32(21, size);
	for (i = 0; i < size; i++) {
		ASSERT_EQ_UINT8('a', result[i]);
	}
	diag_free(result);
	diag_free(script);
	diag_free(pcodes);

	return EXIT_SUCCESS;
}
