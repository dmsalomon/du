
#ifndef SET_H
#define SET_H

struct di_node {
	dev_t dev;
	struct inodes *ino;
	struct di_node *right;
	struct di_node *left;
};

struct inodes {
	ino_t ino;
	struct inodes *right;
	struct inodes *left;
};

int insert_dev_ino(struct di_node**, dev_t, ino_t);
void free_di_nodes(struct di_node*);

#endif
