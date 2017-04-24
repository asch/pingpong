#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <xmmintrin.h>
#include <time.h>
#include <stdatomic.h>

#define limit 100000000

static inline uint64_t rdtsc()
{
	uint32_t lo, hi;

	__asm__ __volatile__ (
			"rdtsc\n"
			: "=a" (lo), "=d" (hi)
			);

	return (uint64_t) hi << 32 | lo;
}

int main(void) {
    atomic_int * shared = mmap(NULL, sizeof(atomic_int), PROT_READ|PROT_WRITE,
            MAP_SHARED|MAP_ANONYMOUS, -1, 0); 

    atomic_store(shared, 0);
    int childstate;
    pid_t client = fork();

    if (client < 0)
        printf("Fork error\n");

    if (client == 0) {
        uint64_t n = rdtsc();
        clock_t t = clock();
        for (unsigned int i = 0; i < limit; i++) {
            atomic_store(shared, 1);
            while (atomic_load(shared) == 1) {
                _mm_pause();
            }
        }
        n = rdtsc() - n;
        t = clock() - t;
        double time_taken = ((double)t)/CLOCKS_PER_SEC;
        printf("%lu\n", n/limit);
        printf("%fs\n", time_taken);
        exit(0);
    }

    for (unsigned int i = 0; i < limit; i++) {
        while (atomic_load(shared) == 0) {
            _mm_pause();
        }
        atomic_store(shared, 0);
    }

    waitpid(client, &childstate, 0); 
    return 0;
}
