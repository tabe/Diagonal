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
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/cmp.h"
#include "diagonal/rbtree.h"
#include "diagonal/private/memory.h"
#include "diagonal/private/filesystem.h"

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
		if (ent->d_type == DT_DIR) {
			if (scan_directory(tree, i + 1, name) == -1) {
				diag_free(name);
				result = -1;
				break;
			}
			continue;
		}
		node = diag_rbtree_node_new((uintptr_t)i, (uintptr_t)name);
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
		if ( (name = strdup(path)) == NULL) goto fail;
		node = diag_rbtree_node_new((uintptr_t)0, (uintptr_t)name);
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

size_t diag_mmap_file(const char *path, char **p)
{
	int fd;
	struct stat st;
	size_t len;

	if ( (fd = open(path, O_RDONLY)) < 0) diag_fatal("could not open file: %s", path);
	if (fstat(fd, &st) < 0) {
		close(fd);
		diag_fatal("could not stat file: %s", path);
	}
	if ( (len = st.st_size) == 0) {
		close(fd);
		goto done;
	}
	*p = (char *)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (*p == MAP_FAILED) diag_fatal("could not map file: %s", strerror(errno));
 done:
	return len;
}

size_t diag_file_to_lines(const char *path, char **dst, char ***lines)
{
	size_t is, nl;
	char *ip;

	assert(path);
	is = diag_mmap_file(path, &ip);
	if (!is) {
		diag_fatal("could not map file: %s", path);
	}
	nl = diag_memory_to_lines(is, ip, dst, lines);
	munmap(ip, is);
	return nl;
}
