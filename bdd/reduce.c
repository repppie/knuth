/*
 * 7.1.4 Algorithm R
 * 7.1.4-54
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <math.h>

struct node {
	int v;
	int lo;
	int hi;
	int aux;
};

struct node *bdd;
int *head;
int avail;
int root;
int v_max;

void
reduce(void)
{
	int i, p, p_, q, r, s, v;

	if (root <= 1)
		return;

	bdd[0].aux = bdd[1].aux = bdd[root].aux = -1;
	for (i = bdd[root].v; i <= v_max; i++)
		head[i] = -1;

	s = root;
	while (s != 0) {
		p = s;
		s = ~bdd[p].aux;
		bdd[p].aux = head[bdd[p].v];
		head[bdd[p].v] = ~p;
		if (bdd[bdd[p].lo].aux >= 0) {
			bdd[bdd[p].lo].aux = ~s;
			s = bdd[p].lo;
		}
		if (bdd[bdd[p].hi].aux >= 0) {
			bdd[bdd[p].hi].aux = ~s;
			s = bdd[p].hi;
		}
	}

	bdd[0].aux = bdd[1].aux = 0;

	v = v_max;

r3:
	p = ~head[v];
	s = 0;
	while (p != 0) {
		int p_;

		p_ = ~bdd[p].aux;
		q = bdd[p].hi;
		/* Fixup pointers to reduced children */
		if (bdd[q].lo < 0)
			bdd[p].hi = ~bdd[q].lo;
		q = bdd[p].lo;
		if (bdd[q].lo < 0) {
			bdd[p].lo = ~bdd[q].lo;
			q = bdd[p].lo;
		}
		if (q == bdd[p].hi) {
			/* LO(p) == HI(p), reduce */
			bdd[p].lo = ~q;
			bdd[p].hi = avail;
			bdd[p].aux = 0;
			avail = p;
		} else if (bdd[q].aux >= 0) {
			/* Add p to bucket q = LO(p) */
			bdd[p].aux = s;
			s = ~q;
			bdd[q].aux = ~p;
		} else {
			bdd[p].aux = bdd[~bdd[q].aux].aux;
			bdd[~bdd[q].aux].aux = p;
		}
		p = p_;
	}

	r = ~s;
	s = 0;
	while (r >= 0) {
		q = ~bdd[r].aux;
		bdd[r].aux = 0;
		if (s == 0)
			s = q;
		else
			bdd[p].aux = q;
		p = q;
		while (bdd[p].aux > 0)
			p = bdd[p].aux;
		r = ~bdd[p].aux;
	}

	p = s;
	if (p == 0)
		goto r9;
	q = p;

r6:
	/* Walk through a bucket (all nodes q with same s = LO) */
	s = bdd[p].lo;
	do {
		/*
		 * Find duplicates by writing ~q < 0 to AUX(HI(q))
		 * for all q with our same LO.
		 */
		r = bdd[q].hi;
		if (bdd[r].aux >= 0)
			bdd[r].aux = ~q;
		else {
			bdd[q].lo = bdd[r].aux;
			bdd[q].hi = avail;
			avail = q;
		}
		q = bdd[q].aux;
	} while (q != 0 && bdd[q].lo == s);

	do {
		if (bdd[p].lo >= 0)
			bdd[bdd[p].hi].aux = 0;
		p = bdd[p].aux;
	} while (p != q);

r9:
	if (p != 0)
		goto r6;
	if (v > bdd[root].v) {
		v--;
		goto r3;
	}
	if (bdd[root].lo < 0)
		root = ~bdd[root].lo;
}

int
main(int argc, char **argv)
{
	char *p;
	int i, j, num, truth;

	if (argc < 2)
		errx(1, "Usage: %s <truth table>\n", argv[0]);

	p = argv[1];
	truth = 0;
	for (i = 0; *p; p++, i++)
		if (*p == '1')
			truth |= 1 << i;
	printf("truth %d\n", truth);

	if (i & (i - 1) || i < 2)
		errx(1, "Truth table must be power of two length");

	v_max = log2(i);
	num = i + 1;

	bdd = malloc(num * sizeof(struct node));
	head = malloc(v_max * sizeof(int));

	for (i = 2; i < num; i++) {
		bdd[i].v = ceil(log2(i)); /* XXX Don't use floating point */
		bdd[i].lo = i * 2 - 1;
		bdd[i].hi = i * 2;
	}
	j = 0;
	for (i = 1 + (1 << (v_max - 1)); i <= (1 << v_max); i++) {
		bdd[i].lo = !!(truth & (1 << j++));
		bdd[i].hi = !!(truth & (1 << j++));
	}

	root = 2;
	reduce();

	for (i = root; i < num; i++)
		if (bdd[i].lo >= 0)
			printf("%d: v %d lo %d hi %d\n", i, bdd[i].v, bdd[i].lo,
			    bdd[i].hi);

	return (0);
}
