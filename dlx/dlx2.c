#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <err.h>
#include <time.h>

#define	NSEC_PER_SEC 1000000000UL

struct node {
	union {
		int top;
		int len;
	};
	int up;
	int down;
	int color;
};

struct item {
	char *name;
	int next;
	int prev;
};

struct progress {
	int tot;
	int cur;
};

#define	MAX_ITEMS 10000
#define	MAX_LEVEL 50000

struct item items[MAX_ITEMS];
char *colors[MAX_ITEMS];
struct node mem[500000];
struct progress progress[MAX_LEVEL];
struct timespec cur_time, start_time;
int pos = 1;
int nr_spacer;
int nr_items;
int nr_colors;
int sol;
int prog;

extern int optind;

static void
add_item_for_option(struct node *item, int c)
{
	struct node *new;

	new = &mem[pos];

	new->top = new->down = item - mem;
	new->up = item->up;
	new->color = c;
	mem[item->up].down = pos;
	item->up = pos;
	item->len++;
	pos++;
}

static int
find_or_add_item(char *name)
{
	struct node *n;
	int i;

	for (i = 1; i < nr_items + 1; i++)
		if (strcmp(items[i].name, name) == 0)
			return (i);
	items[pos].name = name;
	items[pos].prev = items[pos].next = -1;
	n = &mem[pos];
	n->up = n->down = pos;
	pos++;
	nr_items++;

	return (pos - 1);
}

static int
find_or_add_color(char *name)
{
	int i;

	for (i = 0; i < nr_colors; i++)
		if (strcmp(colors[i], name) == 0)
			return (i + 1);
	colors[nr_colors++] = name;
	return (nr_colors);
}

static struct node *
add_spacer(struct node *prev)
{
	struct node *n;

	n = &mem[pos++];
	n->top = nr_spacer--;
	if (prev) {
		prev->down = pos - 2;
		n->up = prev - mem + 1;
	}
	return (n);
}

static void
unhide(int p)
{
	int d, q, u, x;

	q = p - 1;
	while (q != p) {
		x = mem[q].top;
		u = mem[q].up;
		d = mem[q].down;
		if (x <= 0)
			q = d;
		else {
			/* Negative color means it's known to be correct. */
			if (mem[q].color >= 0) {
				mem[u].down = q;
				mem[d].up = q;
				mem[x].len++;
			}
			q--;
		}
	}
}

static void
hide(int p)
{
	int d, q, u, x;

	q = p + 1;
	/* Hide all the items that this option covers. */
	while (q != p) {
		x = mem[q].top;
		u = mem[q].up;
		d = mem[q].down;
		if (x <= 0)
			q = u;
		else {
			/* Negative color means it's known to be correct. */
			if (mem[q].color >= 0) {
				mem[u].down = d;
				mem[d].up = u;
				mem[x].len--;
			}
			q++;
		}
	}
}

static void
uncover(int i)
{
	int l, p, r;

	l = items[i].prev;
	r = items[i].next;
	if (l != -1) {
		items[l].next = i;
		items[r].prev = i;
	}
	for (p = mem[i].up; p != i; p = mem[p].up)
		unhide(p);
}

static void
cover(int i)
{
	int l, p, r;

	/* Hide all the options that have this item. */
	for (p = mem[i].down; p != i; p = mem[p].down)
		hide(p);
	l = items[i].prev;
	r = items[i].next;
	if (l == -1) /* Secondary item */
		return;
	items[l].next = r;
	items[r].prev = l;
}

static void
unpurify(int p)
{
	int c, i, q;

	c = mem[p].color;
	i = mem[p].top;
	mem[i].color = 0; /* XXX Knuth doesn't do this (on purpose) */
	for (q = mem[i].up; q != i; q = mem[q].up) {
		if (mem[q].color < 0)
			mem[q].color = c;
		else
			unhide(q);
	}
}

static void
purify(int p)
{
	int c, i, q;

	c = mem[p].color;
	i = mem[p].top;
	mem[i].color = c; /* From errata. Only for debugging? */
	for (q = mem[i].down; q != i; q = mem[q].down) {
		if (mem[q].color == c)
			mem[q].color = -1;
		else
			hide(q);
	}
}

static void
uncommit(int p, int j)
{
	if (mem[p].color == 0)
		uncover(j);
	else if (mem[p].color > 0)
		unpurify(p);
}

static void
commit(int p, int j)
{
	if (mem[p].color == 0)
		cover(j);
	else if (mem[p].color > 0)
		purify(p);
}

static int
choose(void)
{
	int i, min, min_at;

	min_at = items[0].next;
	min = mem[min_at].len;
	for (i = items[0].next; i != 0; i = items[i].next) {
		if (mem[i].len < min) {
			min_at = i;
			min = mem[i].len;
		}
	}
	return (min_at);
}

static void
print_sol(int x[], int l)
{
	int i, j;

	if (!sol)
		return;

	for (i = 0; i < l; i++) {
		j = x[i];
		for (j = x[i]; mem[j].top > 0; j--);
		j++;
		printf("'");
		for (; mem[j].top > 0; j++) {
			printf("%s", items[mem[j].top].name);
			if (mem[j].color != 0)
				printf(":%s", colors[mem[mem[j].top].color -
				    1]);
			printf("%s",  mem[j + 1].top <= 0 ? "' ": " ");
		}
	}
	printf("\n\n");
	fflush(stdout);
}

static void
show_progress(unsigned long found, int l)
{
	double d, pct;
	int eta, i, secs;

	pct = 0;
	d = 1;
	for (i = 0; i < l; i++) {
		d *= progress[i].tot;
		pct += (double)progress[i].cur / d;
	}
	pct += 1.0f / (2 * d);
#if 1
	secs = (cur_time.tv_sec * NSEC_PER_SEC + cur_time.tv_nsec -
	    start_time.tv_sec * NSEC_PER_SEC - start_time.tv_nsec) /
	    NSEC_PER_SEC;
#endif
	eta = secs * (1 - pct) / pct;
	fprintf(stderr, "%d:%02d: sols %lu l %d pct %lf eta %d:%02d\n", secs /
	    60, secs % 60, found, l, pct, eta / 60, eta % 60);
}

static void
dlx(void)
{
	struct timespec last_time;
	unsigned long cnt, found;
	int i, j, l, p, x[MAX_LEVEL];

	if (clock_gettime(CLOCK_MONOTONIC, &start_time) != 0)
		err(1, "clock_gettime");
	last_time = start_time;

	cnt = found = l = 0;
	while (1) {
		if (items[0].prev == 0) {
			print_sol(x, l);
			found++;
			goto c8;
		}
		i = choose();
		cover(i);
		x[l] = mem[i].down;
		progress[l].tot = mem[i].len;
		progress[l].cur = 0;
		if (prog && (++cnt % 10000) == 0) {
			if (clock_gettime(CLOCK_MONOTONIC, &cur_time) != 0)
				err(1, "clock_gettime");
			if (cur_time.tv_sec * NSEC_PER_SEC + cur_time.tv_nsec >=
			    last_time.tv_sec * NSEC_PER_SEC + last_time.tv_nsec
			    + 60 * NSEC_PER_SEC) {
			   	last_time = cur_time;
				show_progress(found, l);
			}
		}

	c5:
		if (x[l] == i)
			goto c7; /* backtrack */
		p = x[l] + 1;
		while (p != x[l]) {
			j = mem[p].top;
			if (j <= 0)
				p = mem[p].up;
			else {
				commit(p, j);
				p++;
			}
		}
		l++;
		continue;
	c6:
		p = x[l] - 1;
		while (p != x[l]) {
			j = mem[p].top;
			if (j <= 0)
				p = mem[p].down;
			else {
				uncommit(p, j);
				p--;
			}
		}
		i = mem[x[l]].top;
		x[l] = mem[x[l]].down;
		progress[l].cur++;
		goto c5;
	c7:
		uncover(i);
	c8:
		if (l == 0)
			break;
		l--;
		goto c6;
	}
	fprintf(stderr, "found %ld solutions\n", found);
}

int
main(int argc, char **argv)
{
	struct node *spacer;
	struct stat st;
	char *e, *end, *m, *s;
	int ch, fd, i, it, last_primary, primary_done;

	while ((ch = getopt(argc, argv, "ps")) != -1) {
		switch (ch) {
		case 'p':
			prog = 1;
			break;
		case 's':
			sol = 1;
			break;
		default:
			errx(1, "Usage: %s [-s] <path>", argv[0]);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 1)
		errx(1, "Usage: %s [-ps] <path>", argv[0]);

	if ((fd = open(argv[0], O_RDONLY)) < 0)
		err(1, "open");
	if (fstat(fd, &st) != 0)
		err(1, "fstat");
	if ((m = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd,
	    0)) == MAP_FAILED)
		err(1, "mmap");

	end = m + st.st_size;
	s = m;
	while (*m++ != '\n');
	*(m - 1) = '\0';
	last_primary = primary_done = 0;
	while ((e = strsep(&s, " ")) != NULL) {
		if (strcmp(e, "|") == 0) { 
			last_primary = nr_items + 1;
			primary_done = 1;
			continue;
		}
		find_or_add_item(e);
	}

	if (!last_primary)
		last_primary = nr_items + 1;

	for (i = 1; i < last_primary; i++) {
		items[i].prev = i - 1;
		items[i].next = (i + 1) % last_primary;
	}
	items[0].next = 1;
	items[0].prev = last_primary - 1;

	spacer = add_spacer(NULL);
	do {
		s = m;
		while (*m++ != '\n');
		*(m - 1) = '\0';
		/* XXX detect duplicate options? */
		while ((e = strsep(&s, " ")) != NULL) {
			char *cc;
			int c;

			cc = e;
			c = 0;
			if (strchr(e, ':')) {
				strsep(&cc, ":");
				c = find_or_add_color(cc);
			}
			it = find_or_add_item(e);
			add_item_for_option(&mem[it], c);
		}
		spacer = add_spacer(spacer);
	} while (m < end);

	dlx();

	return (0);
}
