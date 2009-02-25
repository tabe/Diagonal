#include "test.h"

#include "diagonal.h"
#include "diagonal/metric.h"
#include "diagonal/imf.h"

int
main()
{
	diag_imf_t *imf;
	char *s;
	int r;

	s = strdup("\r\n");
	r = diag_imf_parse(s, &imf, 0);
	assert(r == 0);
	assert(imf->header_fields);
	assert(!imf->header_fields[0]);
	assert(strcmp("", imf->body) == 0);
	diag_imf_destroy(imf);
	free(s);

	s = strdup("content-type: text/html\r\n"
			   "foo:bar\r\n"
			   "\r\n");
	r = diag_imf_parse(s, &imf, 0);
	assert(imf->header_fields);
	assert(imf->header_fields[0]);
	assert(strcmp("content-type", imf->header_fields[0]->name) == 0);
	assert(strcmp("text/html", imf->header_fields[0]->body) == 0);
	assert(imf->header_fields[1]);
	assert(strcmp("foo", imf->header_fields[1]->name) == 0);
	assert(strcmp("bar", imf->header_fields[1]->body) == 0);
	assert(!imf->header_fields[2]);
	assert(strcmp("", imf->body) == 0);
	diag_imf_destroy(imf);
	free(s);

	return 0;
}
