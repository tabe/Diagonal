/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/bytevector.h"
#include "diagonal/port.h"
#include "diagonal/hash.h"
#include "diagonal/vcdiff.h"

static void
dump_pcodes(struct diag_vcdiff_script *script)
{
	diag_size_t i;

	for (i = 0; i < script->s_pcodes; i++) {
		printf("%d: ", i);
		switch (script->pcodes[i].inst) {
		case DIAG_VCD_ADD:
			printf("ADD\t(");
			break;
		case DIAG_VCD_COPY
:
			printf("COPY\t(");
			break;
		default:
			printf("%d\t(", script->pcodes[i].inst);
			break;
		}
		printf("size: %d ", script->pcodes[i].size);
		switch (script->pcodes[i].inst) {
		case DIAG_VCD_ADD:
			printf("data: %s)\n", script->pcodes[i].attr.data);
			break;
		case DIAG_VCD_COPY:
			printf("addr: %d)\n", script->pcodes[i].attr.addr);
			break;
		default:
			printf(")\n");
			break;
		}
	}
}

static void
contract_and_expand(const char *data, diag_size_t s_window, uint32_t base, diag_size_t s_pcodes)
{
	struct diag_vcdiff_script *script;
	struct diag_rolling_hash32 *rh;
	diag_size_t size, s;
	uint8_t *result;
	char *result0;

	size = strlen(data);
	rh = (struct diag_rolling_hash32 *)diag_rolling_hash32_new_rabin_karp((const uint8_t *)data, size, s_window, base);
	ASSERT_NOT_NULL(rh);
	script = diag_vcdiff_contract(rh);
	ASSERT_NOT_NULL(script);
	dump_pcodes(script);
	ASSERT_EQ_UINT(s_pcodes, script->s_pcodes);
	result = diag_vcdiff_expand(script, &s);
	ASSERT_EQ_UINT(size, s);
	result0 = diag_malloc(s + 1);
	(void)memcpy(result0, result, s);
	result0[s] = '\0';
	ASSERT_EQ_STRING(data, result0);
	diag_free(result0);
	diag_free(result);
	diag_vcdiff_script_destroy(script);
	diag_rolling_hash32_destroy(rh);
}

int
main(void)
{
	contract_and_expand("abcdabcd", 8, 89, 1);
	contract_and_expand("abcdabcd", 6, 97, 1);
	contract_and_expand("abcdabcd", 4, 101, 2);
	contract_and_expand("abcdabcd", 3, 103, 2);
	contract_and_expand("aacdaacd", 2, 107, 2);
	contract_and_expand("aacdaacde", 2, 109, 3);
	contract_and_expand("aacdaacde", 1, 113, 5);
	contract_and_expand("aacdaacde", 1, 113, 5);
	contract_and_expand("aaceaacdde", 1, 127, 7);
	contract_and_expand("aaceaacdde", 3, 131, 3);
	contract_and_expand("aacdaacde", 2, 137, 3);

	return EXIT_SUCCESS;
}
