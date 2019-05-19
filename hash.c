
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

static unsigned int hashf(dev_t dev, ino_t ino)
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
	struct hentry *mem = xmalloc(sizeof(struct hentry));
	mem->dev = dev;
	mem->ino = ino;
	mem->next = NULL;
	return mem;
}

static void resize(unsigned int buckets)
{
	struct hentry **table = xcalloc(buckets, sizeof(struct hentry*));
	struct hentry *cur;
	int i;
	unsigned int hash;

	for (i = 0; i < ht.buckets; i++) {
		for (cur = ht.table[i]; cur; ) {
			hash = hashf(cur->dev, cur->ino) % buckets;
			struct hentry *p = table[hash];

			if (!p) {
				table[hash] = cur;
			}
			else {
				for (; p->next; p = p->next)
					;
				p->next = cur;
			}

			p = cur->next;
			cur->next = NULL;
			cur = p;
		}
	}

	if (ht.table)
		free(ht.table);

	ht.table = table;
	ht.buckets = buckets;
}

int insert_dev_ino(dev_t dev, ino_t ino)
{
	if (!ht.table) {
		resize(4096);
	}

	unsigned int hash = hashf(dev, ino) % ht.buckets;
	struct hentry *q, *p = ht.table[hash];

	if (!p) {
		ht.table[hash] = alloc_dev_ino(dev, ino);
		ht.size++;
		return 0;
	}

	for (; p; p = p->next) {
		if (p->ino == ino && p->dev == dev)
			return 1;
		q = p;
	}

	q->next = alloc_dev_ino(dev, ino);
	ht.size++;

	if (ht.size > 2 * ht.buckets) {
		resize(4 * ht.buckets);
	}

	return 0;
}

void free_table() {
	int i;
	struct hentry *p, *q;

	for (i = 0; i < ht.buckets; i++) {
		for (p = ht.table[i]; p; ) {
			q = p->next;
			free(p);
			p = q;
		}
	}

	if (ht.table)
		free(ht.table);

	ht.buckets = 0;
	ht.size = 0;
}

#ifdef TEST

#include <assert.h>

int main()
{
	assert(insert_dev_ino(5, 2) == 0);
	assert(insert_dev_ino(2, 9) == 0);
	assert(insert_dev_ino(5, 2) == 1);
	assert(insert_dev_ino(2, 9) == 1);

	free_table();

	return 0;
}

#endif
