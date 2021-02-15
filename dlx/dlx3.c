/* MCC solver */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
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
	int slack;
	int bound;
};

struct progress {
	int tot;
	int cur;
};

#define	MAX_ITEMS 100000
#define	MAX_LEVEL 5000

struct item items[MAX_ITEMS];
char *colors[MAX_ITEMS];
struct node mem[5000000];
int ft[MAX_LEVEL];
struct progress progress[MAX_LEVEL];
struct timespec cur_time, start_time;
int pos = 1;
int nr_spacer;
int nr_items;
int nr_colors;
int last_primary;
int cutoff;
int sol;
int prog;
int minimax;

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
		/* See 7.2.2.1-84's solution. */
		if (minimax && d > cutoff)
			mem[q].down = d = x;
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
	int cnt, l, p, r;

	if (minimax) {
		for (cnt = 0, p = mem[i].up; p >= cutoff; p = mem[p].up)
			cnt++;
		if (cnt) {
			mem[i].up = p;
			mem[p].down = i;
			mem[i].len -= cnt;
		}
	}

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
	int c, cnt, i, pp, q;

	c = mem[p].color;
	i = mem[p].top;
	mem[i].color = 0; /* XXX Knuth doesn't do this (on purpose) */

	if (minimax) {
		for (cnt = 0, pp = mem[i].up; pp >= cutoff; pp = mem[pp].up)
			cnt++;
		if (cnt) {
			mem[i].up = pp;
			mem[pp].down = i;
			mem[i].len -= cnt;
		}
	}
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

static void
untweak(int l, int dd)
{
	int a, k, p, x, y, z;

	a = ft[l];
	p = (a <= nr_items) ? a : mem[a].top;
	x = a;
	y = p;
	z = mem[p].down;
	mem[p].down = x;
	k = 0;
	while (x != z) {
		mem[x].up = y;
		k++;
		if (dd)
			unhide(x);
		y = x;
		x = mem[x].down;
	}
	mem[z].up = y;
	mem[p].len += k;
	if (!dd)
		uncover(p);
}

static void
tweak(int x, int p, int do_hide)
{
	int d;

	if (do_hide)
		hide(x);

	d = mem[x].down;
	mem[p].down = d;
	mem[d].up = p;
	mem[p].len--;
}

static int
max(int a, int b)
{
	return a > b ? a : b;
}

static int
choose(void)
{
	int d, deg, i, min, min_at;

	min_at = items[0].next;
	min = mem[min_at].len;
	deg = INT_MAX;
	for (i = items[0].next; i != 0; i = items[i].next) {
		d = min + 1 - max(items[min_at].bound - items[min_at].slack, 0);
#if 1
		if (mem[i].len <= items[i].bound && items[i].slack >=
		    items[i].bound)
			continue;
		if (d < deg || (d == deg && items[i].slack <
		    items[min_at].slack) || (d == deg && items[i].slack ==
		    items[min_at].slack && mem[i].len < min)) {
#else
		if (mem[i].len < min) {
#endif
			min_at = i;
			min = mem[i].len;
			deg = d;
		}
	}


	if (deg <= 0)
		return -1;
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
		if (j <= nr_items + 1) /* null level */
			continue;
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
found_minimax_sol(int x[], int l)
{
	int cnt, i, j, max, top;

	for (i = 0, max = 0; i < l; i++)
		if (x[i] > max)
			max = x[i];
	while (mem[++max].top > 0);
	if (max != cutoff) {
		cutoff = max;

		/* Remove all nodes > cutoff */
		for (i = 0; i < l; i++) {
			top = mem[x[i]].top;
			for (cnt = 0, j = mem[top].up; j >= cutoff; j =
			    mem[j].up)
				cnt++;
			if (cnt) {
				mem[j].down = top;
				mem[top].up = j;
				mem[top].len -= cnt;
			}
		}
	}
}

static void
dlx(void)
{
	struct timespec last_time;
	unsigned long cnt, found;
	int i, j, l, p, q, x[MAX_LEVEL];

	if (clock_gettime(CLOCK_MONOTONIC, &start_time) != 0)
		err(1, "clock_gettime");
	last_time = start_time;

	cnt = found = l = 0;
	while (1) {
		if (items[0].prev == 0) {
			if (minimax)
				found_minimax_sol(x, l);
			print_sol(x, l);
			found++;
			goto m9;
		}
		if ((i = choose()) < 0)
			goto m9;
	m4:
		x[l] = mem[i].down;
		items[i].bound--;
		if (items[i].bound == 0)
			cover(i);
		if (items[i].bound != 0 || items[i].slack != 0)
			ft[l] = x[l];

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

	m5:
		if (items[i].bound == 0 && items[i].slack == 0) {
			if (x[l] == i)
				goto m8; /* backtrack */
		} else if (mem[i].len <= items[i].bound - items[i].slack)
			goto m8;
		else if (x[l] != i)
			tweak(x[l], i, items[i].bound == 0 ? 0 : 1);
		else if (items[i].bound != 0) {
			/* Remove i from active list */
			p = items[i].prev;
			q = items[i].next;
			items[p].next = q;
			items[q].prev = p;
		}

	m6:
		if (x[l] != i) {
			p = x[l] + 1;
			/*
			 * (Potentially partially) cover items != i that
			 * contain x[l].
			 */
			while (p != x[l]) {
				j = mem[p].top;
				if (j <= 0)
					p = mem[p].up;
				else if (j < last_primary) {
					items[j].bound--;
					if (items[j].bound == 0)
						cover(j);
					p++;
				} else {
					commit(p, j);
					p++;
				}
			}
		}
		l++;
		continue;

	m7:
		p = x[l] - 1;
		/*
		 * Uncover items != i in option that contains x[l], in reverse.
		 */
		while (p != x[l]) {
			j = mem[p].top;
			if (j <= 0)
				p = mem[p].down;
			else if (j < last_primary) {
				items[j].bound++;
				if (items[j].bound == 1)
					uncover(j);
				p--;
			} else {
				uncommit(p, j);
				p--;
			}
		}
		x[l] = mem[x[l]].down;
		progress[l].cur++;
		goto m5;
	m8:
		if (items[i].bound == 0 && items[i].slack == 0)
			uncover(i);
		else
			untweak(l, items[i].bound == 0 ? 0 : 1);
		items[i].bound++;
	m9:
		if (l == 0)
			break;
		l--;
		if (x[l] <= nr_items) {
			i = x[l];
			p = items[i].prev;
			q = items[i].next;
			items[p].next = items[q].prev = i;
			goto m8;
		}
		i = mem[x[l]].top;
		goto m7;
	}
	fprintf(stderr, "found %ld solutions\n", found);
}

int
main(int argc, char **argv)
{
	struct node *spacer;
	struct stat st;
	char *e, *end, *m, *s;
	int ch, fd, i, it, primary_done;

	while ((ch = getopt(argc, argv, "mps")) != -1) {
		switch (ch) {
		case 'm':
			minimax = 1;
			cutoff = INT_MAX;
			break;
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
		int u, v;
		u = v = 1;
		if (strcmp(e, "|") == 0) { 
			last_primary = nr_items + 1;
			primary_done = 1;
			continue;
		} else if (!primary_done && *e >= '0' && *e <= '9' &&
		    strchr(e, ':') != NULL) {
		 	/* multiple items have format u:v|name */
			u = strtol(e, &e, 10);
			if (*e++ != ':')
				errx(1, "expected ':' after lower bound");
			v = strtol(e, &e, 10);
			if (*e++ != '|')
				errx(1, "expected '|' after upper bound");

		}
		it = find_or_add_item(e);
		items[it].slack = v - u;
		items[it].bound = v;
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
		if (*s == '\n' || *s == '|')
			continue;
		*(m - 1) = '\0';
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
