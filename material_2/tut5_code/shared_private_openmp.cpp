#include <omp.h>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <cassert>

#define UNITTIME 1000

using std::chrono::microseconds;
using std::this_thread::sleep_for;

void Wait()
{
    sleep_for(microseconds(UNITTIME));
}

void ProcessOpt(int argc, char **argv, int *n_thread)
{
    int c;
    extern char *optarg;
    extern int optopt;
    *n_thread = 2;

    while ((c = getopt(argc, argv, "p:h")) != -1)
        switch (c)
        {
        case 'p':
            *n_thread = atoi(optarg);
            break;

        case 'h':
            fprintf(stderr, "Options:\n-p NTHREAD\tNumber of threads\n");
            exit(2);

        case '?':
            fprintf(stderr, "Unrecognized option: -%c\n", optopt);
            exit(2);
        }
}

int main(int argc, char **argv)
{

    /* Command line option */
    int n_thread;
    ProcessOpt(argc, argv, &n_thread);
    assert(n_thread >= 1);

    // Set number of threads for this example
    // Not needed if you want to use the default number of threads
    omp_set_num_threads(n_thread);

    int shared_int = -1;

#pragma omp parallel
    {
        // shared_int is shared
        int tid = omp_get_thread_num();
        printf("Thread ID %2d: shared_int = %d\n", tid, shared_int);
    }

    int should_be_private = -2;
#pragma omp parallel
    //#pragma omp parallel private(should_be_private)
    {
        int tid = omp_get_thread_num();
        int rand_tid = rand();
        should_be_private = rand_tid;
        Wait();
        if (should_be_private == rand_tid)
            printf("Thread ID %2d: correct\n", tid);
        else
            printf("Thread ID %2d: error!\n", tid);
    }

    printf("Done with parallel block; should_be_private = %d\n", should_be_private);

    const int correct_sum = (n_thread * (n_thread - 1)) / 2;

    for (int trial = 0; trial < 10; ++trial)
    {
        int read_write = 0;
#pragma omp parallel
        {
            // Some long calculation that can be done in parallel
            // #pragma omp atomic
            read_write += omp_get_thread_num();
        }

        if (read_write != correct_sum)
        {
            printf("Test no. %d failed; %d != %d\n", trial, read_write, correct_sum);
        }
        else
        {
            printf("Test no. %d passed; %d == %d\n", trial, read_write, correct_sum);
        }
    }

    return 0;
}
