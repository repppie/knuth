/* 7.2.2-25 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <err.h>
#include <assert.h>

#define	TRIE_SIZE 50000

struct trie {
	int nodes[26];
	int p;
};

struct trie trie6[TRIE_SIZE];

struct word {
	char *w;
	int next[5];
};

struct word words5[6000];
char *words6[20000];
/* Orthogonal lists for word5s with a..z as their 1..5th letter */
struct word *ortho[5][26];
int len[5][26];


int hist[7];
int nsol;

static int
map(char *path, char **m)
{
	struct stat st;
	int fd;

	if ((fd = open(path, O_RDONLY)) < 0)
		err(1, "open");
	if (fstat(fd, &st))
		err(1, "stat");
	if ((*m = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE,
	    fd, 0)) == MAP_FAILED)
		err(1, "mmap");

	return (st.st_size);
}

static void
read_trie(char *path, struct trie *trie, char *words[])
{
	struct trie *t;
	int size;
	char *m;
	int fd, i, next, nwords;

	size = map(path, &m);
	t = trie;
	next = 0;
	nwords = 0;
	words[0] = m;
	for (i = 0; i < size; i++, m++) {
		assert(next < TRIE_SIZE);
		if (*m == '\n') {
			t = trie;
			*m = '\0';
			words[++nwords] = m + 1;
			continue;
		}
		assert(*m >= 'a' && *m <= 'z');
		/* Use nwords + 1 to avoid conflict with 0 (non-present). */
		if (*(m + 1) == '\n')
			t->nodes[*m - 'a'] = nwords + 1;
		else if (t->nodes[*m - 'a'] == 0)
			t->nodes[*m - 'a'] = ++next;
		t->p |= (1 << (*m - 'a'));
		t = trie + t->nodes[*m - 'a'];
	}
}

static void
read_ortho(char *path, struct word *ortho[5][26], struct word words[])
{
	size_t size;
	char *m;
	int i, j, nwords;
	struct word *cur, *prev[5][26];

	size = map(path, &m);
	for (i = 0; i < 5; i++)
		for (j = 0; j < 26; j++)
			prev[i][j] = NULL;

	words[1].w = m;
	nwords = 1;
	for (i = 0; i < size; i++, m++) {
		if (*m == '\n') {
			*m = '\0';
			words[++nwords].w = m + 1;
			continue;
		}
		assert(*m >= 'a' && *m <= 'z');
		if ((cur = prev[i % 6][*m - 'a']) != NULL)
			cur->next[i % 6] = nwords;
		else
			ortho[i % 6][*m - 'a'] = &words[nwords];
		len[i % 6][*m - 'a']++;
		prev[i % 6][*m - 'a'] = &words[nwords];
	}
}

static void
try(int l, int a[5])
{
	struct word *w;
	int b, i, j, min, min_sum, new[5], p, s;

	hist[l]++;

	if (l == 6) {
#if 0
		for (i = 0; i < 5; i++)
			printf("%s\n", words6[a[i] - 1]);
		printf("\n");
#endif
		nsol++;
		printf("Found: %d\r", nsol);
		fflush(stdout);
		return;
	}

	min = min_sum = INT_MAX;
	for (i = 0; i < 5; i++) {
		p = trie6[a[j]].p;
		s = 0;
		while ((b = (p & -p)) != 0) {
			p &= ~b;
			s += len[i][__builtin_ffs(b) - 1];
		}
		if (s < min_sum) {
			min_sum = s;
			min = i;
		}
	}

	p = trie6[a[min]].p;
	while ((b = (p & -p)) != 0) {
		p &= ~b;
		w = ortho[min][__builtin_ffs(b) - 1];
		while (w && w != &words5[0]) {
			/* Select next word5 w for column based on a */
			for (j = 0; j < 5; j++)
				if ((new[j] = trie6[a[j]].nodes[w->w[j] - 'a'])
				    == 0)
					goto next;
			try(l + 1, new);
next:
			if (w->next[min] == 0)
				break;
			w = &words5[w->next[min]];
		}
	}
}

int
main(int argc, char **argv)
{
	char *w5, *w6;
	char *m6;
	int a[5], i, next, nwords;

	if (argc < 3) {
		w5 = "words/words5-from-sgb";
		w6 = "words/words6-from-OSPD4";
	} else {
		w6 = argv[1];
		w5 = argv[2];
	}

	read_trie(w6, trie6, words6);
	read_ortho(w5, ortho, words5);

	for (i = 0; i < 5; i++)
		a[i] = 0;
	try(0, a);

	printf("found %d solutions\nhisto: ", nsol);
	for (i = 0; i < 7; i++)
		printf("%d ", hist[i]);
	printf("\n");

	return (0);
}
