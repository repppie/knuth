#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <err.h>

struct node {
	int v;
	int lo;
	int hi;
};

struct template {
	int left;
	int right;
	int l;
	int h;
};

#define MAX_V 1000
#define	MEMSIZE ((4U << 30)-1)
#define	MAX_STACK 1000

char *mem;
int op;
unsigned int f0, g0;
int v_max;
unsigned int hbase, ntop, tbot;
unsigned int lcount[MAX_V], llist[MAX_V], hlist[MAX_V], lstart[MAX_V];
int stack[MAX_STACK];

#define NODE(x) ((struct node *)&mem[(x) * sizeof(struct node)])
#define TPLT(x) ((struct template *)&mem[(x) * sizeof(struct template)])

#define	min(a, b) ((a) < (b) ? (a) : (b))
#define	max(a, b) ((a) > (b) ? (a) : (b))

/* ceil(log2(n)) */
static int
log2_ceil(unsigned int n)
{
	static int tab[32] = { 0, 1, 2, 6, 3, 11, 7, 27, 4, 25, 23, 12, 8, 14,
	    28, 17, 31, 5, 10, 26, 24, 22, 13, 16, 30, 9, 21, 15, 29, 20, 19,
	    18 };
	unsigned int n_;

	if (n == 1)
		return (1);
	if (n == 0x80000000)
		return (31);

	n_ = n | n >> 1;
	n_ |= n_ >> 2;
	n_ |= n_ >> 4;
	n_ |= n_ >> 8;
	n_ |= n_ >> 16;
	if (n_ == 0xffffffff)
		return (32);
	n_++;

	return (tab[(n_ * 0x465bea7) >> 27] - ((n & -n) == n));
}

/*
 * Returns 0 or -1 if the result of melding f and g is constant 0 or 1.
 * Otherwise returns the level of the meld.
 */
static int
find_level(unsigned int f, unsigned int g)
{
	int t;

	if (f <= 1 && g <= 1)
		return (-((op >> (3 - 2 * f - g)) & 1));
	if (f <= 1 && g > 1) {
		t = f ? (op & 3) : (op >> 2);
		if (t == 0)
			return 0;
		if (t == 3)
			return -1;
	}
	if (f > 1 && g <= 1) {
		t = (g ? op : (op >> 1)) & 5;
		if (t == 0)
			return 0;
		if (t == 5)
			return -1;
	}
	return (min(NODE(f0 + f)->v, NODE(g0 + g)->v));
}

static int
make_template(int b, unsigned int f, unsigned int g)
{
	int h, t;

	h = hbase + (((uint32_t)(314159257UL * f + 271828171UL * g)) >>
	    (32 - b));
	t = TPLT(h)->h;
	while (t != 0 && (TPLT(t)->left != f || TPLT(t)->right != g))
		t = TPLT(t)->l;
	if (t == 0) {
		t = --tbot;
		TPLT(t)->left = f;
		TPLT(t)->right = g;
		TPLT(t)->l = TPLT(h)->h;
		TPLT(h)->h = t;
	}
	//printf("T %d left %d right %d l %d\n", t, TPLT(t)->left,
	    //TPLT(t)->right, TPLT(t)->l);
	return (t);
}

static void
synth()
{
	int b, f, g, i, l, q, r, s, t, vf, vg;

	/* root is the last node */
	f = g0 - 1 - f0;
	g = ntop - 1 - g0;
	l = find_level(f, g);
	if (l <= 0)
		errx(1, "XXX %d", l); /* 7.1.4-66 */

	lstart[l - 1] = MEMSIZE / sizeof(struct template);
	for (i = l + 1; i <= v_max; i++) {
		llist[i] = hlist[i] = 0;
		lcount[i] = 0;
	}

	tbot = MEMSIZE / sizeof(struct template) - 1;
	TPLT(tbot)->left = f;
	TPLT(tbot)->right = g;

	/* Start by melding the root and keep melding the resulting children */
	while (1) {
		do {
			lstart[l] = tbot;
			t = lstart[l - 1];
			//printf("starting level %d at t %d (%d)\n", l, t,
			    //tbot);
			while (t > tbot) {
				int ll, lh;

				t--;
				f = TPLT(t)->left;
				g = TPLT(t)->right;
				vf = NODE(f0 + f)->v;
				vg = NODE(g0 + g)->v;
				ll = find_level((vf <= vg ? NODE(f0 + f)->lo :
				    f), (vf >= vg ? NODE(g0 + g)->lo : g));
				lh = find_level((vf <= vg ? NODE(f0 + f)->hi :
				    f), (vf >= vg ? NODE(g0 + g)->hi : g));

				//printf("scanning %d.%d vf %d vg %d ll %d"
				    //" lh %d\n", f, g, vf, vg, ll, lh);
				
				if (ll <= 0)
					TPLT(t)->l = -ll;
				else {
					TPLT(t)->l = llist[ll];
					llist[ll] = t;
					lcount[ll]++;
				}
				if (lh <= 0)
					TPLT(t)->h = -lh;
				else {
					TPLT(t)->h = hlist[lh];
					hlist[lh] = t;
					lcount[lh]++;
				}
			}

			if (l == v_max)
				goto reduce;
			l++;
		} while (lcount[l] == 0);

		b = log2_ceil(lcount[l]);
		//printf("b %d lcount[%d] %d\n", b, l, lcount[l]);
		/* Allocate space for 2^b templates */
		hbase = tbot - (1 << b); 
		for (i = 0; i < (1 << b); i++)
			TPLT(hbase + i)->h = 0;

		/* Make the l level templates */
		t = llist[l];
		while (t != 0) {
			s = TPLT(t)->l;
			f = TPLT(t)->left;
			g = TPLT(t)->right;
			vf = NODE(f0 + f)->v;
			vg = NODE(g0 + g)->v;
			TPLT(t)->l = make_template(b, (vf <= vg ?
			    NODE(f0 + f)->lo : f), (vf >= vg ?
			    NODE(g0 + g)->lo : g));
			t = s;
		}
		t = hlist[l];
		while (t != 0) {
			s = TPLT(t)->h;
			f = TPLT(t)->left;
			g = TPLT(t)->right;
			vf = NODE(f0 + f)->v;
			vg = NODE(g0 + g)->v;
			TPLT(t)->h = make_template(b, (vf <= vg ?
			    NODE(f0 + f)->hi : f), (vf >= vg ?
			    NODE(g0 + g)->hi : g));
			t = s;
		}
	}
reduce:
	ntop = f0 + 2;

	while (1) {
		t = lstart[l - 1];
		/* Bucket sort */
		//printf("l %d\n", l);
		while (t > lstart[l]) {
			t--;
			//printf("before l %d h %d\n", TPLT(t)->l, TPLT(t)->h);
			TPLT(t)->l = TPLT(TPLT(t)->l)->right;
			TPLT(t)->h = TPLT(TPLT(t)->h)->right;
			if (TPLT(t)->l == TPLT(t)->h)
				TPLT(t)->right = TPLT(t)->l;
			else {
				TPLT(t)->right = -1;
				TPLT(t)->left = TPLT(TPLT(t)->l)->left;
				TPLT(TPLT(t)->l)->left = t;
			}
			//printf("b t %d %d.%d l %d h %d\n", t, TPLT(t)->left,
			    //TPLT(t)->right, TPLT(t)->l, TPLT(t)->h);
		}

		/* Restore clone addresses */
		while (1) {
			if (t == lstart[l - 1]) {
				t = lstart[l] - 1;
				break;
			}
			if (TPLT(t)->left < 0)
				TPLT(TPLT(t)->l)->left = TPLT(t)->left;
			t++;
		}

		/* Done with level? */
	s9:
		do {
			t++;
			if (t == lstart[l - 1])
				goto loop;
		} while (TPLT(t)->right >= 0);
		
		/* Examine a bucket */
		s = t;
		while (s > 0) {
			r = TPLT(s)->h;
			//printf("s %d r %d\n", s, r);
			TPLT(s)->right = TPLT(r)->left;
			if (TPLT(r)->left < 0)
				TPLT(r)->left = s;
			s = TPLT(s)->left;
		}
		s = t;

		/* Make clones */
		while (1) {
			if (s < 0)
				goto s9;
			if (TPLT(s)->right >= 0)
				s = TPLT(s)->left;
			else {
				r = TPLT(s)->left;
				TPLT(TPLT(s)->h)->left = TPLT(s)->right;
				TPLT(s)->right = s;
				q = ntop++;
				TPLT(s)->left = ~(q - f0);
				NODE(q)->lo = ~TPLT(TPLT(s)->l)->left;
				NODE(q)->hi = ~TPLT(TPLT(s)->h)->left;
				NODE(q)->v = l;
				//printf("N %d v %d l %d h %d\n", q - f0,
				    //NODE(q)->v, NODE(q)->lo, NODE(q)->hi);
				s = r;
			}
		}

	loop:
		l--;
		if (lstart[l] >= MEMSIZE / sizeof(struct template)) {
			if (TPLT(MEMSIZE / sizeof(struct template) - 1)->right
			    == 0)
				ntop--;
			break;
		}
	}
}

static void
update_v_max(int l, int v)
{
	int i;

	/* Update all 0 1 nodes' v field if v_max changes */
	if (v > v_max) {
		v_max = v;
		for (i = 0; i < l; i++) {
			NODE(stack[i])->v = v_max + 1;
			NODE(stack[i] + 1)->v = v_max + 1;
		}
	}
}

static void
set_node(int i, int v, int lo, int hi)
{
	NODE(i)->v = v;
	NODE(i)->lo = lo;
	NODE(i)->hi = hi;
}

static int
read_num(int *dot)
{
	char buf[256] = { };
	int c, i;

	i = 0;
	while ((c = getchar()) != ' ' && c != '\n' && c != '.') {
		if (c < '0' || c > '9')
			errx(1, "expected number");
		buf[i++] = c;
	}
	if (dot && c == '.')
		*dot = 1;
	return (atoi(buf));
}

int
main(void)
{
	char buf[512];
	int c, i, l;

	mem = malloc(MEMSIZE);

	TPLT(0)->left = ~0;
	TPLT(0)->right = 0;
	TPLT(1)->left = ~1;
	TPLT(1)->right = 1;
	ntop = 4;
	v_max = 0;

	i = l = 0;
	memset(buf, 0, 256);
	while ((c = getchar()) != EOF) {
		if (c != ' ' && c != '\n') {
			buf[i++] = c;
			continue;
		}
		buf[i] = '\0';
		i = 0;
		if (buf[0] == '\0')
			continue;
		if (buf[0] == '~' || (buf[0] >= '1' && buf[0] <= '9')) {
			int v;

			if (buf[0] == '~')
				i = 1;
			v = atoi(buf + i);
			update_v_max(l, v);
			set_node(ntop, v_max + 1, 0, 0);
			set_node(ntop + 1, v_max + 1, 1, 1);
			if (i == 0)
				set_node(ntop + 2, v, 0, 1);
			else
				set_node(ntop + 2, v, 1, 0);
			stack[l++] = ntop;
			//printf("adding v %s%d @ %d %d\n", i == 0 ? "" : "~",
			    //v, ntop, v_max);
			ntop += 3;
			i = 0;
			continue;
		} else if (strcmp(buf, "p") == 0) {
			if (l == 0)
				errx(1, "stack underflow");
			for (i = ntop-1; i >= stack[l-1] + 2; i--)
				printf("%d: ~%d ? %d : %d\n", i - stack[l-1],
				    NODE(i)->v, NODE(i)->lo, NODE(i)->hi);
			printf("l %d ntop %d\n", l, ntop);
			i = 0;
			continue;
		} else if (strcmp(buf, "dup") == 0) {
			int s;

			s = stack[l - 1];
			stack[l++] = ntop;
			for (; s < stack[l - 1]; s++)
				memcpy(NODE(ntop++), NODE(s),
				    sizeof(struct node));
			continue;
		} else if (strcmp(buf, "drop") == 0) {
			if (l == 0)
				errx(1, "stack underflow");
			ntop = stack[--l];
			continue;
		} else if (strcmp(buf, "bdd") == 0) {
			/* Keep reading nodes until number ending in '.' */
			int hi, last, lo, max, v;

			stack[l++] = ntop;
			set_node(ntop++, v_max + 1, 0, 0);
			set_node(ntop++, v_max + 1, 1, 1);
			max = last = 0;
			do {
				v = read_num(NULL);
				if (v > max)
					max = v;
				lo = read_num(NULL);
				hi = read_num(&last);
				set_node(ntop++, v, lo, hi);
			} while(!last);
			update_v_max(l, max);
			//printf("read %d nodes ntop %d\n", ntop-stack[l-1],
			    //ntop);
			continue;
		} else if (strncmp(buf, "chg", strlen("chg")) == 0) {
			/* chg <level> <old_v> <new_v> */
			/* XXX should only change if it doesn't change order */
			int end, lvl, new, old;

			lvl = read_num(NULL);
			old = read_num(NULL);
			new = read_num(NULL);
			update_v_max(l, new);
			if (lvl >= l)
				errx(1, "invalid level %d", lvl);
			if (lvl == 0)
				end = ntop;
			else
				end = stack[l - lvl];
			for (i = stack[l-lvl-1]+2; i < end; i++)
				if (NODE(i)->v == old)
					NODE(i)->v = new;
			i = 0;
			continue;
		} else if (buf[0] == 'c') { /* count */
			unsigned long *c;
			int n, start;

			start = stack[l-1];
			n = ntop - start;
			printf("n %d\n", n);
			c = malloc(n * sizeof(long));
			memset(c, 0, sizeof(n * sizeof(long)));
			c[0] = 0;
			c[1] = 1;

			for (i = 2; i < ntop - start; i++) {
				int lo, hi;

				lo = NODE(start + i)->lo;
				hi = NODE(start + i)->hi;
				c[i] = (1ULL << (NODE(start+lo)->v -
				    NODE(start+i)->v - 1))
				    * c[lo] + (1ULL << (NODE(start+hi)->v -
				    NODE(start+i)->v - 1)) * c[hi];
			}
			printf("nodes %d count %lu\n", ntop - stack[l-1] - 2,
			   c[ntop - start - 1]);
			free(c);
			i = 0;
			continue;
		} else if (buf[0] == '&')
			op = 1;
		else if (buf[0] == '^')
			op = 6;
		else if (buf[0] == '|')
			op = 7;
		else
			errx(1, "unknown op %s", buf);

		if (l < 2)
			errx(1, "stack underflow");
		g0 = stack[l - 1];
		f0 = stack[l - 2];
		//printf("synth l %d f0 %d g0 %d ntop %d op %d\n", l, f0, g0,
		    //ntop, op);
		l--;
		synth();
		//printf("ntop %d l %d cur %d nodes %d\n", ntop, l,
		    //stack[l - 1], l > 1 ? stack[l-1]-stack[l-2] : stack[l]-4);
	}

	return (0);
}
