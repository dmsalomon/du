
#include <stdio.h>
#include <stdlib.h>

struct hentry {
	dev_t dev;
	ino_t ino;
	struct hentry *next;
};

static struct {
	struct hentry **table;
	unsigned int buckets;
	unsigned int size;
} ht = {
	.table = NULL,
	.buckets = 0,
	.size = 0,
};

/* TODO: make this better */
static unsigned int genhash(dev_t dev, ino_t ino)
{
	register unsigned int h = 0;
	int i;
	char *s;

	s = (char *)&dev;
	for (i = 0; i < sizeof(dev); s++, i++) {
		h *= 16777619;
		h ^= *s;
	}

	s = (char *)&ino;
	for (i = 0; i < sizeof(ino); s++, i++) {
		h *= 16777619;
		h ^= *s;
	}
	return h;
}

static void *xcalloc(size_t n, size_t size)
{
	void *mem = calloc(n, size);

	if (!mem) {
		perror("calloc:");
		exit(1);
	}
	return mem;
}

static void *xmalloc(size_t size)
{
	void *mem = malloc(size);

	if (!mem) {
		perror("malloc:");
		exit(1);
	}
	return mem;
}

static struct hentry *alloc_dev_ino(dev_t dev, ino_t ino)
{
	struct hentry *mem;

	mem = xmalloc(sizeof(*mem));

	mem->dev = dev;
	mem->ino = ino;
	mem->next = NULL;
	return mem;
}

static void resize(unsigned int buckets)
{
	int i;
	unsigned int hash;
	struct hentry **table, **p, **q;

	table = xcalloc(buckets, sizeof(*table));

	for (i = 0; i < ht.buckets; i++) {
		for (p = &ht.table[i]; *p; ) {
			hash = genhash((*p)->dev, (*p)->ino) % buckets;
			q = &table[hash];

			for (q = &table[hash]; *q; q = &(*q)->next)
				;

			*q = *p;
			*p = (*p)->next;
			(*q)->next = NULL;
		}
	}

	if (ht.table)
		free(ht.table);

	ht.table = table;
	ht.buckets = buckets;
}

int insert_dev_ino(dev_t dev, ino_t ino)
{
	unsigned int hash;
	struct hentry **pp;

	if (!ht.table) {
		resize(4096);
	}

	hash = genhash(dev, ino) % ht.buckets;

	for (pp = &ht.table[hash]; *pp; pp = &(*pp)->next)
		if ((*pp)->ino == ino && (*pp)->dev == dev)
			return 1;

	*pp = alloc_dev_ino(dev, ino);

	if (++ht.size > 2 * ht.buckets) {
		resize(4 * ht.buckets);
	}

	return 0;
}

void free_table() {
	int i;
	struct hentry *p, *q;

	for (i = 0; i < ht.buckets; i++) {
		for (p = ht.table[i]; p; p = q) {
			q = p->next;
			free(p);
		}
	}

	if (ht.table)
		free(ht.table);

	ht.table = NULL;
	ht.buckets = 0;
	ht.size = 0;
}

#ifdef TEST_HASH
#include <assert.h>

int main()
{
	assert(ht.size == 0);
	assert(insert_dev_ino(5, 2) == 0);
	assert(insert_dev_ino(2, 9) == 0);
	assert(insert_dev_ino(5, 2) == 1);
	assert(insert_dev_ino(2, 9) == 1);
	assert(ht.size == 2);

	assert(genhash(532, 432) == genhash(532, 432));

	free_table();

	assert(!ht.table);
	assert(ht.buckets == 0);
	assert(ht.size == 0);

	return 0;
}

#endif
