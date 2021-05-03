
#ifndef HASH_H
#define HASH_H

#include <sys/types.h>

int insert_dev_ino(dev_t, ino_t);
void free_table();

#endif
