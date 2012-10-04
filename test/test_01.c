#include <stdio.h>
#include <stdlib.h>

#define N		25
#define M		55
#define SIZE	60

#pragma instrumente toto2
#pragma instrumente (toto, main)

void toto2 ()
{
	
}

int toto (int *tab)
{
	int sum = tab[0] + tab[1] + tab[2];
	int i;

	for (i = 0; i < N; i++)
	{
		sum += tab[i] + 3 * tab[i+1];
		tab[i+1] = sum;
	}

	sum += tab[0] + tab[1];

	for (i = 1; i < M; i++)
	{
		sum += tab[i] + 3 * tab[i+1] + 4 * tab[i-1];
		tab[i+1] = sum;
		tab[i+2] = sum * 2;
		tab[i+3] = sum * 3;
	}

	return sum;
}

int main (int argc, char **argv)
{
	int i;
	int *tab;

	(void) argc;
	(void) argv;

//	tab = malloc (SIZE * sizeof (int) );

//	for (i = 0; i < SIZE; ++i)
//	{
//		tab[i] = i + 1;
//	}

	toto2 ();

//	printf ("The result is %d\n", toto (tab) );

	printf ("Test\n");

//	free (tab);

	return 0;
}