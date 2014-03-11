#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
	if (argc < 3) return EXIT_FAILURE;

	double median = atof(argv[1]);
	int num = atoi(argv[2]);

	unsigned int seed = (unsigned int)time(NULL);
	fprintf(stderr, "seed: %d\n", seed);
	srand(seed);

	int i;
	if (num%2 == 0) {
		for (i = 0; i < num/2; i++) {
			int r = rand();
			printf("%g\n", median-r);
			printf("%g\n", median+r);
		}
	} else {
		for (i = 0; i < num - 1; i++) {
			if (i%2 == 0) {
				printf("%g\n", median-rand());
			} else {
				printf("%g\n", median+rand());
			}
		}
		printf("%g\n", median);
	}

	return EXIT_SUCCESS;
}
