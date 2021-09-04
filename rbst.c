#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <err.h>
#include <assert.h>
#include <time.h>

struct node {
	int key;
	int size; /* size of left or right subtree based on orientation */
	bool orientation; /* 0 = left, 1 = right */
	struct node *left;
	struct node *right;
};

struct rbst {
	struct node *root;
	int size;
};

static int
rand_max(int max)
{
	int div, ret;

	div = RAND_MAX / (max + 1);
	do {
		ret = rand() / div;
	} while (ret > max);
	return (ret);
}

static int
size(struct node *t)
{
	int n;
	if (!t)
		return (0);
	n = 1;
	if (t->left)
		n += size(t->left);
	if (t->right)
		n += size(t->right);
	return (n);
}

static void
check_size(struct node *t) {
	int l, r;

	if (!t)
		return;

	check_size(t->left);
	check_size(t->right);

	l = size(t->left);
	r = size(t->right);
	if (!t->orientation)
		assert(t->size == l);
	else
		assert(t->size == r);
}

static void
flip(struct node *t, int *n)
{
	int tmp;

	tmp = *n - t->size - 1;
	*n = t->size;
	t->size = tmp;
	assert(t->size >= 0);
	t->orientation ^= 1;
}

static int
split(struct node *t, int x, struct node **lt, struct node **gt, int n)
{
	int nn;

	nn = n;
	if (t == NULL) {
		*lt = *gt = NULL;
		return (0);
	}
	if (x < t->key) {
		*gt = t;
		if (!t->orientation)
			flip(t, &nn);
		else
			nn = n - t->size - 1;
		return (split(t->left, x, lt, &((*gt)->left), nn));
	} else {
		*lt = t;
		if (t->orientation)
			flip(t, &nn);
		else
			nn = n - t->size - 1;
		return (split(t->right, x, &((*lt)->right), gt, nn) +
		    t->size + 1);
	}
}

static struct node *
insert_at_root(struct node *t, int n, int x)
{
	struct node *lt, *gt;

	lt = gt = NULL;
	n = split(t, x, &lt, &gt, n);
	t = malloc(sizeof(struct node));
	t->key = x;
	t->left = lt;
	t->right = gt;
	t->size = n;
	t->orientation = 0;
	//check_size(t);
	return (t);
}

struct node *
_rbst_insert(struct node *t, int n, int x)
{
	int r;

	r = rand_max(n + 1);
	if (r == n || t == NULL)
		t = insert_at_root(t, n, x);
	else if (x < t->key) {
		if (!t->orientation)
			flip(t, &n);
		else
			n = n - t->size - 1;
		t->left = _rbst_insert(t->left, n, x);
	} else {
		if (t->orientation)
			flip(t, &n);
		else
			n = n - t->size - 1;
		t->right = _rbst_insert(t->right, n, x);
	}
	//check_size(t);
	return (t);
}

void
rbst_insert(struct rbst *t, int x)
{
	t->root = _rbst_insert(t->root, t->size, x);
	t->size++;
}

static struct node *
join(struct node *l, struct node *r, int m, int n)
{
	int nn, rr;

	if (m + n == 0)
		return NULL;
	if (m == 0)
		return (r);
	if (n == 0)
		return (l);
	rr = rand_max(m + n);
	if (rr < m) {
		/* l becomes the new root */
		nn = m;
		if (l->orientation)
			flip(l, &nn);
		l->right = join(l->right, r, m - l->size - 1, n);
		//check_size(l);
		return (l);
	} else {
		/* r becomes the new root */
		nn = n;
		if (!r->orientation)
			flip(r, &nn);
		r->left = join(l, r->left, m, n - r->size - 1);
		//check_size(r);
		return (r);
	}
}

static struct node *
_rbst_delete(struct node *t, int n, int x)
{
	struct node *tmp;
	int m;

	if (t == NULL)
		return (NULL);
	if (x < t->key) {
		if (!t->orientation)
			flip(t, &n);
		else
			n = n - t->size - 1;
		t->left = _rbst_delete(t->left, n, x);
	} else if (x > t->key) {
		if (t->orientation)
			flip(t, &n);
		else
			n = n - t->size - 1;
		t->right = _rbst_delete(t->right, n, x);
	} else {
		m = n - t->size - 1;
		if (!t->orientation)
			tmp = join(t->left, t->right, t->size, m);
		else
			tmp = join(t->left, t->right, m, t->size);
		free(t);
		t = tmp;
	}
	//check_size(t);
	return (t);
}

struct node *
rbst_delete(struct rbst *t, int x)
{
	t->root = _rbst_delete(t->root, t->size, x);
	t->size--;
	return (t->root);
}

static void
shuffle(int *a, int n)
{
	int i, j, t;

	for (i = n - 1; i > 1; i--) {
		j = rand_max(i + 1);
		t = a[i];
		a[i] = a[j];
		a[j] = t;
	}
}

int
height(struct node *t)
{
	int l, r;

	if (t == NULL)
		return (0);
	l = height(t->left);
	r = height(t->right);
	return (l > r ? l + 1 : r + 1);
}

int
main(int argc, char **argv)
{
	struct rbst t = { };
	int *a, i, n, s, x;

	if (argc < 2)
		errx(1, "Usage: %s <n> [seed]", argv[0]);
	s = time(NULL);
	if (argc > 2)
		s = atoi(argv[2]);
	n = atoi(argv[1]);
	srand(s);

	a = malloc(sizeof(int) * n);
	for (i = 0; i < n; i++)
		a[i] = i;
	shuffle(a, n);

	for (i = 0; i < n; i++)
		rbst_insert(&t, a[i]);

#if 1
	shuffle(a, n);

	for (i = 0; i < n / 2; i++) {
		x = a[i];
		//printf("deleting %d\n", x);
		rbst_delete(&t, x);
	}
#endif

	printf("%d\n", height(t.root));

	return (0);
}
