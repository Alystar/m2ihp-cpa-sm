#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N		25
#define M		55
#define SIZE	60

#pragma instrument toto2
#pragma instrument (toto, main)

int toto2 (int m)
{
	int * tab = malloc (m * sizeof (int));
	int i, sum;

	for (i = 0; i < m; ++i)
		tab [i] = rand () / RAND_MAX;

	for (i = 0; sum < 50; ++i)
	{
		sum += i + tab [i];
	}

	free (tab);

	return sum;
}

int main (int argc, char **argv)
{
	int i, j, k;
	float tab [100];
	float tab2 [50];
	int n = 10;
	float sum =0.0f;
	float sum2 = 0.0f;

	int m = atoi (argv [1]);

	for (i = 0; i < 100; ++i)
	{
		tab [i] = rand () / RAND_MAX;
		if (i < 50)
			tab2 [i] = rand () / RAND_MAX;
	}

	for (i = 0; i < m; ++i)
		tab2 [i] = 2 * rand () / RAND_MAX;


	for (i = 0; i < n; ++i)
	{
		for (j = 0; j < 10; ++j)
			sum += tab [i*n + j];
		sum2 = 0.0f;
		for (j = 0; j < 50; ++j)
		{
			sum2 += tab2 [j];
			sum2 += tab2 [j + i];
		}
		sum += sum2;
	}

	printf ("%f\n", sum);

	return 0;
}
