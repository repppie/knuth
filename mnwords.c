/* 7.2.2.(24) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <err.h>
#include <assert.h>

#define	TRIE_SIZE 50000

struct trie {
	int nodes[26];
};

struct trie trie6[TRIE_SIZE];
struct trie trie5[TRIE_SIZE];

char *words5[6000];
char *words6[20000];

int hist[7];
int nsol;

static void
read_words(char *path, struct trie *trie, char *words[])
{
	struct stat st;
	struct trie *t;
	char *m;
	int fd, i, next, nwords;

	if ((fd = open(path, O_RDONLY)) < 0)
		err(1, "open");
	if (fstat(fd, &st))
		err(1, "stat");
	if ((m = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd,
	    0)) == MAP_FAILED)
		err(1, "mmap");

	t = trie;
	next = 0;
	nwords = 0;
	words[0] = m;
	for (i = 0; i < st.st_size; i++, m++) {
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
		t = trie + t->nodes[*m - 'a'];
	}
}

static void
try(int l, int a[5])
{
	char *word;
	int i, j, new[5];

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

	/* a contains constraints for each letter of word5, based on trie6 */
	for (i = 0; i < 5757; i++) {
		word = words5[i];
		/* Select next word5 w for column based on a */
		for (j = 0; j < 5; j++)
			if ((new[j] = trie6[a[j]].nodes[word[j] - 'a']) == 0)
				goto next;
		try(l + 1, new);
next:
		continue;
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

	read_words(w6, trie6, words6);
	read_words(w5, trie5, words5);

	for (i = 0; i < 5; i++)
		a[i] = 0;
	try(0, a);

	printf("found %d solutions\nhisto: ", nsol);
	for (i = 0; i < 7; i++)
		printf("%d ", hist[i]);
	printf("\n");

	return (0);
}
