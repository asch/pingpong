#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdatomic.h>
#include <xmmintrin.h>
#include <sched.h>

#define limit 1000000000ULL // 1e9

atomic_int aint;
atomic_int fire;

static inline uint64_t rdtsc()
{
	uint32_t lo, hi;

	__asm__ __volatile__ (
			"rdtsc\n"
			: "=a" (lo), "=d" (hi)
			);

	return (uint64_t) hi << 32 | lo;
}

void* fun1(void *not_used)
{
    cpu_set_t cpuset;
    pthread_t thread;
    thread = pthread_self();
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    int s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (s != 0)
        printf("pthread_setaffinity_np");
    while (!atomic_load(&fire));

A:  while (atomic_load(&aint) % 2 == 0) {
        //_mm_pause();
    }
    int i = atomic_fetch_add(&aint, 1);
    if (i >= limit-2)
        return 0;
    goto A;
}

void* fun2(void *not_used)
{
    cpu_set_t cpuset;
    pthread_t thread;
    thread = pthread_self();
    CPU_ZERO(&cpuset);
    CPU_SET(2, &cpuset);
    int s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (s != 0)
        printf("pthread_setaffinity_np");
    while (!atomic_load(&fire));

B:  while (atomic_load(&aint) % 2 != 0) {
        //_mm_pause();
    }
    int i = atomic_fetch_add(&aint, 1);
    if (i >= limit-2)
        return 0;
    goto B;
}

int main()
{
    atomic_init(&fire, 0);
    atomic_init(&aint, 0);
    clock_t t;
    uint64_t n;

    pthread_t t1, t2;

    pthread_create(&t1,NULL,fun1,NULL);
    pthread_create(&t2,NULL,fun2,NULL);

    sleep(1); // give created threads time to set affinity

    atomic_fetch_add(&fire, 1);
    t = clock();
    n = rdtsc();

    pthread_join(t1,NULL);   
    pthread_join(t2,NULL);   

    t = clock() - t;
    n = rdtsc() - n;
    double time_taken = ((double)t)/CLOCKS_PER_SEC;
 
    printf("Total time(clocks) for %llu R-W iterations: %fs(%llu)\n", limit, time_taken, n/limit);
    return 0;
}
