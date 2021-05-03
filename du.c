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

#include "hash.h"

// macros to compute the blocksize on each platform
// this is to be consistent with GNU which ignores
// POSIX and uses 1Kb blocks instead of 512b
#ifdef __linux__
	#define BLOCKS(nb)	((nb)/2)
#elif __MACH__
	#define BLOCKS(nb)	((nb))
	#ifndef PATH_MAX
		#include <sys/syslimits.h>
	#endif
#endif

// macros to check for "." and ".."
#define ISDOT(p)	((p)[0] == '.' && !(p)[1])
#define ISDOTDOT(p)	((p)[0] == '.' && (p)[1] == '.' && !(p)[2])

// function prototypes
uintmax_t du(char *);
void perrorf(char *);

int status;

int main(int argc, char **argv)
{
	char path[PATH_MAX];
	uintmax_t total = 0;

	if (argc < 2) {
		strcpy(path, ".");
		total += du(path);
	} else {
		for (argv++; *argv; argv++) {
			strcpy(path, *argv);
			total += du(path);
		}
	}

	if (argc > 2) {
		printf("%lu\t%s\n", total, "total");
	}

	free_table();
	return status;
}

/**
 * Get the disk usage in a directory and all
 * its subdirectories.
 *
 * The function uses a recursive strategy.
 */
uintmax_t du(char *path)
{
	uintmax_t sz = 0;

	DIR *dp = opendir(path);
	char *end = path + strlen(path);
	struct dirent *ep;
	struct stat info;

	if (lstat(path, &info)) {
		perrorf(path);
		status = 1;
		return 0;
	}

	if (!dp) {
		if (S_ISDIR(info.st_mode)) {
			perrorf(path);
			status = 1;
		}
		sz += BLOCKS(info.st_blocks);
		goto out;
	}

	for (; (ep = readdir(dp)); *end = '\0') {
		// create the full path for lstat
		if (*(end-1) != '/')
			strcat(path, "/");
		strcat(path, ep->d_name);

		if (lstat(path, &info)) {
			// report the error, and continue
			perrorf(path);
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
				sz += du(path);
			}
		}
		else if (info.st_nlink == 1 ||
			    !insert_dev_ino(info.st_dev, info.st_ino)) {
			/* if a file has more than one hardlink, check
			 * and store the inode number
			 */
			sz += BLOCKS(info.st_blocks);
		}
	}

	closedir(dp);
out:
	printf("%lu\t%s\n", sz, path);
	return sz;
}

void perrorf(char *str)
{
	fprintf(stderr, "du: ");
	perror(str);
}

