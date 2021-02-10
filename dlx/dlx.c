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

#define	MAX_ITEMS 1000
#define	MAX_LEVEL 5000

struct item items[MAX_ITEMS];
struct node mem[50000];
struct progress progress[MAX_LEVEL];
struct timespec cur_time, start_time;
int pos = 1;
int nr_spacer;
int nr_items;
int sol;
int prog;


extern int optind;

static void
add_item_for_option(struct node *item)
{
	struct node *new;

	new = &mem[pos];

	new->top = new->down = item - mem;
	new->up = item->up;
	mem[item->up].down = pos;
	item->up = pos;
	item->len++;
	pos++;
}

static int
find_or_add_item(char *name) {
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
			mem[u].down = q;
			mem[d].up = q;
			mem[x].len++;
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
			mem[u].down = d;
			mem[d].up = u;
			mem[x].len--;
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
	int i, in, j;

	if (!sol)
		return;

	for (i = 0; i < l; i++) {
		j = x[i];
		for (j = x[i]; mem[j].top > 0; j--);
		j++;
		printf("'");
		for (in = j; mem[j].top > 0; j++) {
			printf("%s%s", items[mem[j].top].name,
			    mem[j + 1].top <= 0 ? "' ": " ");
		}
	}
	printf("\n\n");
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
	x2:
		if (items[0].prev == 0) {
			print_sol(x, l);
			found++;
			goto x8;
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

	x5:
		if (x[l] == i)
			goto x7; /* backtrack */
		p = x[l] + 1;
		while (p != x[l]) {
			j = mem[p].top;
			if (j <= 0)
				p = mem[p].up;
			else {
				cover(j);
				p++;
			}
		}
		l++;
		continue;
	x6:
		p = x[l] - 1;
		while (p != x[l]) {
			j = mem[p].top;
			if (j <= 0)
				p = mem[p].down;
			else {
				uncover(j);
				p--;
			}
		}
		i = mem[x[l]].top;
		x[l] = mem[x[l]].down;
		progress[l].cur++;
		goto x5;
	x7:
		uncover(i);
	x8:
		if (l == 0)
			break;
		l--;
		goto x6;
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
		while ((e = strsep(&s, " ")) != NULL) {
			it = find_or_add_item(e);
			add_item_for_option(&mem[it]);
		}
		spacer = add_spacer(spacer);
	} while (m < end);

	dlx();

	return (0);
}
