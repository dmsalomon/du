/** \file du.c
 *  Author: Dov Salomon (dms833) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "set.h"

// macros to check for "." and ".."
#define ISDOT(p)	((p)[0] == '.' && !(p)[1])
#define ISDOTDOT(p)	((p)[0] == '.' && (p)[1] == '.' && !(p)[2])

// macros to compute the blocksize on each platform
// this is to be consistent with GNU which ignores
// POSIX and uses 1Kb blocks instead of 512b
#ifdef __linux__
#define BLOCKS(nb)	((nb)/2)
#elif __MACH__
#define BLOCKS(nb)	((nb))
#endif

// function prototypes
uintmax_t du(char *);
void perrorf(char *);

// master pointer to (dev, ino) structure
struct di_node *di;

int status;

int main(int argc, char **argv)
{
	if (argc > 2) {
		fprintf(stderr, "usage: du [directory]\n");
		return 1;
	}

	char path[PATH_MAX];

	if (argc == 2)
		strcpy(path, argv[1]);
	else
		strcpy(path, ".");

	du(path);

	free_di_nodes(di);

	return status;
}

/**
 * Get the disk usage in a directory and all
 * its subdirectories.
 *
 * The function uses a recursive strategy.
 */
uintmax_t du(char *dirname)
{
	uintmax_t sz = 0;

	DIR *dp = opendir(dirname);
	char *end = dirname + strlen(dirname);
	struct dirent *ep;
	struct stat info;

	if (!dp) {
		perrorf(dirname);
		if (!lstat(dirname, &info)) {
			sz += BLOCKS(info.st_blocks);
		}
		printf("%lu\t%s\n", sz, dirname);
		status = 1;
		return sz;
	}

	for (; (ep = readdir(dp)); *end = '\0') {
		// create the full path for lstat
		if (*(end-1) != '/')
			strcat(dirname, "/");
		strcat(dirname, ep->d_name);

		if (lstat(dirname, &info)) {
			// report the error, and continue
			perrorf(dirname);
			status = 1;
			continue;
		}

		/**
		 * 3 cases:
		 * ".": count the blocks
		 * directory: recursively get nblocks
		 * else: get number of blocks if not seen already
		 */

		if (S_ISDIR(info.st_mode)) {
			if (ISDOT(ep->d_name)) {
				sz += BLOCKS(info.st_blocks);
			}
			else if (!ISDOTDOT(ep->d_name)) {
				sz += du(dirname);
			}
		}
		else if (info.st_nlink == 1 ||
			    !insert_dev_ino(&di, info.st_dev, info.st_ino)) {
			/* if a file has more than one hardlink, check
			 * and store the inode number
			 */
			sz += BLOCKS(info.st_blocks);
		}
	}

	closedir(dp);
	printf("%lu\t%s\n", sz, dirname);
	return sz;
}

void perrorf(char *str)
{
	fprintf(stderr, "du: ");
	perror(str);
}

