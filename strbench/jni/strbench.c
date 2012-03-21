/*
 * (c) Petr Baudis <pasky@suse.cz>  2009
 * MIT licence.
 *
 * Simple tool for benchmarking string functions.
 * Compile with: gcc -std=c99 -Wall -O3 -fno-builtin -o strbench strbench.c
 * Run like: ./strbench memcmp 4096 4 8 16 32 128 512
 */

#include<assert.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/times.h>
#include<unistd.h>

#define READSIZE (1024*1024)
const int samplesizes[] = { 4, 8, 16, 128, 512 };

void
strcmp_bench(char *s1, char *s2, int sample_len)
{
#undef strcmp
	volatile int r = strcmp(s1, s2);
	assert(!r || r);
}

void
strncmp_bench(char *s1, char *s2, int sample_len)
{
#undef strncmp
	volatile int r = strncmp(s1, s2, sample_len);
	assert(!r || r);
}

void
memcmp_bench(char *s1, char *s2, int sample_len)
{
#undef memcmp
	volatile int r = memcmp(s1, s2, sample_len);
	assert(!r || r);
}

void
memcpy_bench(char *s1, char *s2, int sample_len)
{
#undef memcpy
        volatile int r = memcpy(s1, s2, sample_len);
        assert(!r || r);
}


void
memset_bench(char *s1, char *s2, int sample_len)
{
#undef memset
        volatile int r = memset(s1, &s2, sample_len);
        assert(!r || r);
}


void
strlen_bench(char *s1, char *s2, int sample_len)
{
#undef strlen
	volatile int r = strlen(s1);
	assert(r <= sample_len - 0);
	r = strlen(s2);
	assert(r <= sample_len - 1);
}

struct bench {
	char *name;
	void (*func)(char *s1, char *s2, int sample_len);
} benches[] = {
	{ "strcmp", strcmp_bench },
	{ "strncmp", strncmp_bench },
	{ "memcmp", memcmp_bench },
	{ "memcpy", memcpy_bench },
	{ "memset", memset_bench },
	{ "strlen", strlen_bench },
	{ NULL, NULL }
};

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Usage: %s {strcmp|strncmp|memcmp|memcpy|memset|strlen} ITERS SAMPLESIZE..\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	char *func = argv[1];
	int iters = atoi(argv[2]);

	struct bench *b;
	for (b = benches; b->name; b++)
		if (!strcmp(func, b->name))
			break;
	if (!b->name) {
		fprintf(stderr, "unknown benchmark name: %s\n", func);
		exit(EXIT_FAILURE);
	}

	/* Load up samples. */
	char *sample = malloc(READSIZE);
	if (!sample) { perror("malloc"); exit(EXIT_FAILURE); }
	FILE *f = fopen("/dev/urandom", "rb");
	if (!f) { perror("fopen"); exit(EXIT_FAILURE); }
	fread(sample, READSIZE, 1, f);
	fclose(f);

	srand(sample[0] | (sample[1] << 8));
	/* Benchmark different sample sizes. */
	int s = 3;
	for (s = 3; s < argc; s++) {
		int size = atoi(argv[s]);
		int samples = READSIZE / size;

		/* Punch zeros at sample ends. */
		int i = 0;
		for (i = 0; i < samples; i++)
			sample[(i + 1) * size - 1] = 0;

		/* Run benchmark! */
		struct tms tstart;
		clock_t start = times(&tstart);
		int k = 0;
		for ( k = 0; k < iters; k++)
			for ( i = 0; i < samples; i += 2)
				b->func(&sample[i * size], &sample[i * size + 1], size);

		/* Print time... */
		struct tms tnow;
		clock_t now = times(&tnow);
		int u = sysconf(_SC_CLK_TCK);
		printf("%d %d\t%f %f %f\t(size, samples, TIMES[s] - user, system, total)\n",
			size, samples,
			(float)(tnow.tms_utime - tstart.tms_utime) / u,
			(float)(tnow.tms_stime - tstart.tms_stime) / u,
			(float)(now - start) / u);

		/* Restore sample ends. */
		for ( i = 0; i < samples; i++)
			sample[(i + 1) * size - 1] = rand();
	}

	return EXIT_SUCCESS;
}
