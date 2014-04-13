/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#elif defined(_WIN32) && defined(__MINGW32__)
#include <windows.h>
#else
#error "no memory-mapped file"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/cmp.h"
#include "diagonal/rbtree.h"
#include "diagonal/private/memory.h"
#include "diagonal/private/filesystem.h"

#if defined(_WIN32) && defined(__MINGW32__)
struct diag_mmap_ex {
	DIAG_MMAP_HEAD;
	/* extensions */
	HANDLE fh;
};
#endif

static size_t trim_trailing_slashes(char *path)
{
	size_t len = strlen(path);
	while (len > 1 && path[len-1] == '/') {
		path[--len] = '\0';
	}
	return len;
}

static int scan_directory(struct diag_rbtree *tree, int i, char *path)
{
	struct diag_rbtree_node *node;
	int result, slen;
	DIR *dir;
	char *name;
	size_t len;

	assert(tree && path);
	if ( (dir = opendir(path)) == NULL) return -1;
	size_t path_len;
	if (i == 1) { /* top */
		path_len = trim_trailing_slashes(path);
	} else {
		path_len = strlen(path);
	}
	for (;;) {
		struct dirent *ent = readdir(dir);
		if (!ent) {
			result = (errno) ? -1 : 0;
			break;
		}
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
			continue;
		}
#ifdef NAME_MAX
		len = path_len + NAME_MAX + 2;
#elif defined(MAX_PATH) /* Win32 */
		len = path_len + MAX_PATH + 2;
#else
		len = path_len + MAXNAMLEN + 2;
#endif
		name = diag_calloc(len, sizeof(*name));
		slen = snprintf(name, len, "%s/%s", path, ent->d_name);
		if (slen < 0 || (int)len <= slen)  {
			diag_free(name);
			result = -1;
			break;
		}
#ifdef _BSD_SOURCE
		if (ent->d_type == DT_DIR) {
			if (scan_directory(tree, i + 1, name) == -1) {
				diag_free(name);
				result = -1;
				break;
			}
			continue;
		}
#elif defined(_WIN32) && defined(__MINGW32__)
		/* TODO */
#endif
		node = diag_rbtree_node_new((intptr_t)i, (intptr_t)name);
		diag_rbtree_insert(tree, node);
	}
	closedir(dir);
	return result;
}

static struct diag_rbtree *map_paths(char **paths)
{
	struct diag_rbtree *tree;
	struct diag_rbtree_node *node;
	char *path;
	struct stat st;
	char *name;
	size_t i = 0;

	assert(paths);
	tree = diag_rbtree_create(DIAG_CMP_IMMEDIATE);
	while ( (path = paths[i++]) ) {
		if (stat(path, &st) == -1) goto fail;
		if (S_ISDIR(st.st_mode)) {
			if (scan_directory(tree, 1, path) == -1) goto fail;
			continue;
		}
		if ( (name = diag_strdup(path)) == NULL) goto fail;
		node = diag_rbtree_node_new((intptr_t)0, (intptr_t)name);
		diag_rbtree_insert(tree, node);
	}
	return tree;
 fail:
	diag_rbtree_destroy(tree);
	return NULL;
}

static char **serialize_entries(const struct diag_rbtree *tree,
				size_t *num_entries)
{
	struct diag_rbtree_node *node;
	char **e = NULL;
	size_t i = 0;

	assert(tree);
	if (tree->num_nodes == 0) goto done;
	e = diag_calloc(tree->num_nodes, sizeof(*e));
	node = diag_rbtree_minimum(tree);
	assert(node);
	do {
		e[i++] = (char *)node->attr;
	} while ( (node = diag_rbtree_successor(node)) );
	assert(i == tree->num_nodes);
 done:
	*num_entries = i;
	return e;
}

/* API */

void diag_assert_directory(const char *path)
{
	int r = diag_is_directory(path);
	if (r == 1) return;
	else if (r == 0) diag_fatal("%s is not a directory", path);
	else diag_fatal("could not stat %s", path);
}

int diag_is_directory(const char *path)
{
	assert(path);

	struct stat st;
	int r = stat(path, &st);
	if (r == 0) {
		return S_ISDIR(st.st_mode) ? 1 : 0;
	}
	perror(path);
	return r;
}

char **diag_paths(char **paths, size_t *num_entries)
{
	struct diag_rbtree *tree;
	char **entries;

	tree = map_paths(paths);
	if (!tree) return NULL;
	entries = serialize_entries(tree, num_entries);
	diag_rbtree_destroy(tree);
	return entries;
}

struct diag_mmap *diag_mmap_file(const char *path, enum diag_mmap_mode mode)
{
#ifdef HAVE_SYS_MMAN_H
	int fd;
	if ( (fd = open(path, O_RDONLY)) < 0) {
		perror(path);
		return NULL;
	}
	struct stat st;
	if (fstat(fd, &st) < 0) {
		perror(path);
		close(fd);
		return NULL;
	}
	void *addr = NULL;
	size_t size;
	if ( (size = (size_t)st.st_size) == 0) {
		goto done;
	}
	int prot = PROT_READ;
	if (mode == DIAG_MMAP_COW) prot |= PROT_WRITE;
	addr = mmap(NULL, size, prot, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED) {
		perror(path);
		close(fd);
		return NULL;
	}

 done:
	close(fd);
	struct diag_mmap *mm = diag_malloc(sizeof(*mm));
	mm->addr = addr;
	mm->size = size;
	return mm;
#elif defined(_WIN32) && defined(__MINGW32__)
	HANDLE fh = CreateFile(path,
			       GENERIC_READ,
			       FILE_SHARE_READ,
			       NULL,
			       OPEN_EXISTING,
			       0,
			       NULL);
	if (fh == INVALID_HANDLE_VALUE) return NULL;
	DWORD ls = GetFileSize(fh, NULL);
	if (ls == INVALID_FILE_SIZE) {
		(void)CloseHandle(fh);
		return NULL;
	}
	if (ls == 0) {
		(void)CloseHandle(fh);
		struct diag_mmap *mm = diag_malloc(sizeof(*mm));
		mm->addr = NULL;
		mm->size = 0;
		return mm;
	}
	HANDLE fmo = CreateFileMapping(fh,
				       NULL,
				       PAGE_READONLY,
				       0,
				       0,
				       NULL);
	if (!fmo) {
		(void)CloseHandle(fh);
		return NULL;
	}
	DWORD access = (mode == DIAG_MMAP_RO) ? FILE_MAP_READ : FILE_MAP_COPY;
	void *vof = MapViewOfFile(fmo,
				  access,
				  0,
				  0,
				  0);
	/* Note that CreateHandle() with a return value of CreateFileMapping()
	   can be called before UnmapViewOfFile() with the one of
	   CreateFileMapping(). */
	(void)CloseHandle(fmo);
	if (!vof) {
		(void)CloseHandle(fh);
		return NULL;
	}
	struct diag_mmap_ex *mme = diag_malloc(sizeof(*mme));
	mme->addr = vof;
	mme->size = ls;
	mme->fh = fh;
	return (struct diag_mmap *)mme;
#endif
}

void diag_munmap(struct diag_mmap *mm)
{
	if (mm->size > 0) {
#ifdef HAVE_SYS_MMAN_H
		(void)munmap(mm->addr, mm->size);
#elif defined(_WIN32) && defined(__MINGW32__)
		struct diag_mmap_ex *mme = (struct diag_mmap_ex *)mm;
		(void)UnmapViewOfFile(mme->addr);
		(void)CloseHandle(mme->fh);
#endif
	}
	diag_free(mm);
}

size_t diag_file_to_lines(const char *path, char **dst, char ***lines)
{
	assert(path);
	struct diag_mmap *mm = diag_mmap_file(path, DIAG_MMAP_RO);
	if (!mm) diag_fatal("could not map file: %s", path);
	if (mm->size == 0) {
		diag_munmap(mm);
		return 0;
	}
	size_t nl = diag_memory_to_lines(mm->size, (char *)mm->addr, dst, lines);
	diag_munmap(mm);
	return nl;
}
