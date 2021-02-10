/* 7.2.2-75 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <err.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define	MAX_VERTICES 5000
#define	MAX_ARCS 5000

struct vertex {
	char *name;
	int arcs;
	int tag;
	int tried;
};

struct arc {
	int tip;
	int next;
};

struct vertex v[MAX_VERTICES];
struct arc a[MAX_ARCS];

int num_v = 1, num_a = 1;

static void
read_graph(char *_m, size_t len)
{
	char *e, *l, *m;
	int line;

	line = 0;
	for (m = _m; m < _m + len && ((l = strsep(&m, "\n")) != NULL); line++) {
		if (*l == '*')
			continue;
		/* Vertex */
		if (*l == '"') {
			if ((e = strsep(&l, ",")) == NULL)
				errx(1, "expected , at line %d", line);
			e[strlen(e) - 1] = '\0';
			v[num_v].name = e + 1;
			if (*l != 'A')
				errx(1, "expected first arc at line %d", line);
			v[num_v++].arcs = strtol(l + 1, NULL, 0) + 1;
		} else {
			if (*l != 'V')
				errx(1, "expected tip at line %d", line);
			if ((e = strsep(&l, ",")) == NULL)
				errx(1, "expected , at line %d", line);
			a[num_a].tip = strtol(e + 1, NULL, 0) + 1;
			if (*l == 'A')
				a[num_a].next = strtol(l + 1, NULL, 0) + 1;
			num_a++;
		}
	}
}

int vs[500];
int as[500];
int is[500];
int tried[500];
int n;
int found;

static void
print_one(void)
{
	int i;

	found++;
#if 0
	for (i = 0; i < n; i++)
		printf("%s ", v[vs[i]].name);
	printf("\n");
#endif
}

static void
sub(int vv)
{
	int aa, i, j, l, u;

	l = 1;
	aa = as[0] = v[vv].arcs;
	vs[0] = vv;
	for (i = 1; i < num_v; i++)
		v[i].tag = 0;
	for (i = 1; i < vv; i++)
		v[i].tag++;
	v[vv].tag++;
	i = 0;
	goto r4;
	while (1) {
		if (l == n) {
			print_one();
			l--;
		}
	r3:
		aa = a[aa].next;
	r4:
		if (aa != 0)
			goto r5;
		else if (i == l - 1)
			goto r6;
		else {
			i++;
			aa = v[vs[i]].arcs;
		}
	r5:
		u = a[aa].tip;
		v[u].tag++;
		if (v[u].tag > 1)
			goto r3;
		is[l] = i;
		as[l] = aa;
		vs[l] = u;
		l++;
		continue;
	r6:
		if (--l == 0)
			break;
		i = is[l];
		for (j = l; j > i; j--)
			for (aa = v[vs[j]].arcs; aa; aa = a[aa].next)
				v[a[aa].tip].tag--;
		for (aa = a[as[l]].next; aa; aa = a[aa].next)
			v[a[aa].tip].tag--;
		aa = as[l];
		goto r3;
	}
}

int
main(int argc, char **argv)
{
	struct stat st;
	char *m;
	int fd, i;

	if (argc < 3)
		errx(1, "Usage: %s <file> <n>\n", argv[0]);

	if ((fd = open(argv[1], O_RDONLY)) < 0)
		err(1, "open");

	if (fstat(fd, &st) != 0)
		err(1, "stat");
	if ((m = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE,
	    fd, 0)) == MAP_FAILED)
		err(1, "mmap");

	read_graph(m, st.st_size);

	fprintf(stderr, "%d vertices %d arcs\n", num_v - 1, num_a - 1);

	n = atoi(argv[2]);

	for (i = 1; i < num_v; i++)
		sub(i);

	fprintf(stderr, "found %d\n", found);

	return (0);
}
