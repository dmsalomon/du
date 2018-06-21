/** \file set.c
 *   Author: Dov Salomon (dms833)
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "set.h"

void alloc_die(void *);
struct di_node *alloc_di_node(dev_t, ino_t);
struct inodes *alloc_inodes(ino_t);

/**
 * To keep track of seen inodes, this module stores (dev, ino) pairs.
 *
 * The dev numbers are kept in the di_nodes which form
 * a binary tree. Each di_node then holds a pointer
 * to a separate tree which holds all the inodes kept
 * on that device.
 */


/**
 * Inserts a (dev, ino) pair. If the pair is already
 * found the function returns 1.
 *
 * If the pair is not found, it is added and the function returns 0.
 *
 * Because of the two-level tree structure, the routine is
 * a really two successive insert operations.
 */
int insert_dev_ino(struct di_node **di, dev_t dev, ino_t ino)
{

	if (!*di) {
		*di = alloc_di_node(dev, ino);
		return 0;
	}

	struct di_node *pdi = *di;
	struct di_node *qdi = *di;

	while (pdi) {
		qdi = pdi;
		if (dev < pdi->dev)
			pdi = pdi->left;
		else if (dev > pdi->dev)
			pdi = pdi->right;
		else
			break;
	}

	if (!pdi) {
		pdi = alloc_di_node(dev, ino);
		if (dev < qdi->dev)
			qdi->left = pdi;
		else
			qdi->right = pdi;
	}
	else {
		struct inodes *p = pdi->ino;
		struct inodes *q = pdi->ino;

		while (p) {
			q = p;
			if (ino < p->ino)
				p = p->left;
			else if (ino > p->ino)
				p = p->right;
			else //found!
				return 1;
		}

		p = alloc_inodes(ino);

		// insert inode
		if (ino < q->ino)
			q->left = p;
		else
			q->right = p;
	}
	return 0;
}

/*
 * allocate and initialize and 'dev' node
 */
struct di_node *alloc_di_node(dev_t dev, ino_t ino)
{
	struct di_node *p;

	p = malloc(sizeof(struct di_node));
	alloc_die(p);

	p->dev = dev;
	p->right = NULL;
	p->left = NULL;

	p->ino = alloc_inodes(ino);

	return p;
}

/*
 * allocate and initialize an 'inode' node
 */
struct inodes *alloc_inodes(ino_t ino)
{
	struct inodes *p;

	p = malloc(sizeof(struct inodes));
	alloc_die(p);

	p->ino = ino;
	p->left = NULL;
	p->right = NULL;

	return p;
}

// verify malloc'd memory
void alloc_die(void *p)
{
	if (p)
		return;

	perror("du: alloc_die");
	exit(1);
}

/**
 * Recursively frees all the nodes pointed
 * to by *ip
 */
void free_inodes(struct inodes *ip)
{
	if (!ip)
		return;

	free_inodes(ip->left);
	free_inodes(ip->right);
	free(ip);
}

/**
 * Recursively frees all the nodes pointed
 * to by *dp
 */
void free_di_nodes(struct di_node *dp)
{
	if (!dp)
		return;

	free_di_nodes(dp->left);
	free_di_nodes(dp->right);
	free_inodes(dp->ino);
	free(dp);
}
