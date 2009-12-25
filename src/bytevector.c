#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/bytevector.h"

static void
bytevector_munmap(diag_bytevector_t *bv)
{
	assert(bv);
	(void)munmap((void *)bv->data, (size_t)bv->size);
}

diag_bytevector_t *
diag_bytevector_new_path(const char *path)
{
	diag_bytevector_t *bv;
	int fd, r;
	struct stat st;
	diag_size_t size;
	uint8_t *data;

	assert(path);
	fd = open(path, O_RDONLY);
	if (fd < 0) return NULL;
	r = fstat(fd, &st);
	if (r < 0) return NULL;
	size = (diag_size_t)st.st_size;
	data = (uint8_t *)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (data == MAP_FAILED) return NULL;
	bv = (diag_bytevector_t *)diag_malloc(sizeof(diag_bytevector_t));
	bv->size = size;
	bv->data = data;
	bv->finalize = bytevector_munmap;
	return bv;
}

char *
diag_bytevector_to_asciz(const diag_bytevector_t *bv)
{
	char *s;

	assert(bv);
	s = (char *)diag_malloc((size_t)bv->size + 1);
	(void)memcpy(s, bv->data, (size_t)bv->size);
	s[bv->size] = '\0';
	return s;
}

void
diag_bytevector_destroy(diag_bytevector_t *bv)
{
	if (!bv) return;
	if (bv->finalize) bv->finalize(bv);
	diag_free(bv);
}
