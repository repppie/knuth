/* See TAOCP 7.2.2.(21). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

static void
print_sol(int *x, int n)
{
	int i;

	for (i = 0; i < 2 * n + 1; i++)
		printf("%d ", x[i]);
	printf("\n");
}

int
main(int argc, char **argv)
{
	int a, i, j, k, l, n, n_, nsol, *p, *x, *y;

	if (argc < 2)
		err(1, "Usage: %s <n>\n", argv[0]);

	n = atoi(argv[1]);
	n_ = n - ((n % 2) == 0 ? 1 : 0);
	x = malloc((2 * n + 1) * sizeof(int));
	p = malloc((n + 1) * sizeof(int));
	y = malloc((2 * n + 1) * sizeof(int));

	memset(x, 0, (2 * n + 1) * sizeof(int));
	for (i = 0; i < (n + 1); i++) 
		p[i] = i + 1;
	p[n] = 0;
	l = 1;
	a = nsol = 0;

	while (1) {
		if ((k = p[0]) == 0) {
			//print_sol(x, n);
			nsol++;
			goto l5;
		}
		j = 0;
		while (x[l] < 0) {
			if ((l == n / 2 && !(a & (1 << n_))) ||
			    (l >= n - 1 && !(a & (1 << (2 * n - l - 1)))))
				goto l5;
			l++;
		}
l3:
		if (l + k + 1 > 2 * n)
			goto l5;

		if (l == n / 2 && !(a & (1 << n_))) {
			while (k != n_) {
				j = k;
				k = p[k];
			}
		}
		if (l >= n - 1 && !(a & (1 << (2 * n - l - 1)))) {
			while (l + k + 1 != 2 * n) {
				j = k;
				k = p[k];
			}
		}

		if (x[l + k + 1] == 0) {
			x[l] = k;
			x[l + k + 1] = -k;
			y[l] = j;
			p[j] = p[k]; /* Remove k from linked list */
			a |= (1 << k);
			l++;
			continue;
		}
l4:
		j = k;
		k = p[j];
		/* Try with different k. */
		if (k != 0)
			goto l3;
l5:
		/* Backtrack. */
		l--;
		if (l > 0) {
			while (x[l] < 0)
				l--;
			k = x[l];
			x[l] = 0;
			x[l + k + 1] = 0;
			j = y[l];
			p[j] = k; /* Reinsert k in list */
			a &= ~(1 << k);
			if (l == n/2 && k == n_)
				goto l5;
			else
				goto l4;
		} else
			break;
	}

	printf("Found %d solutions\n", nsol);
	return (0);
}
