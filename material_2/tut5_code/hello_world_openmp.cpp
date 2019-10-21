#include <omp.h>
#include <cstdio>
#include <vector>
#include <sstream>
#include <iomanip>
using std::ostringstream;
using std::string;
using std::vector;

#define SCALE 10000
#define ARRINIT 2000

void DoWork(long tid)
{
    int digits = (2 + tid) * 14;
    // Number of digits = 4 * (tid + 1)

    ostringstream pi;
    pi.fill('0');
    vector<long> arr(digits + 1);

    for (int i = 0; i <= digits; ++i)
    {
        arr[i] = ARRINIT;
    }

    long sum = 0;

    for (int i = digits; i > 0; i -= 14)
    {
        int carry = sum % SCALE;
        sum = 0;

        for (int j = i; j > 0; --j)
        {
            sum = sum * j + SCALE * arr[j];
            arr[j] = sum % ((j << 1) - 1);
            sum /= (j << 1) - 1;
        }

        pi << std::setw(4) << std::right << carry + sum / SCALE;
    }

    printf("Thread %2ld approximated Pi as\t %s\n", tid, pi.str().c_str());
}

int main()
{
    printf("Let's compute pi =\t\t%.37s\n",
           "3.14159265358979323846264338327950288419716939937510582");

    // Fork a team of threads
#pragma omp parallel num_threads(4)
    // This is like pthread_create(...)
    // We are using only 8 threads in this example
    // You can use
    // #pragma omp parallel
    // to run with the default number of threads on the node.
    {
        long tid = omp_get_thread_num();

        // Only thread 0 does this
        if (tid == 0)
        {
            int n_threads = omp_get_num_threads();
            printf("Number of threads = %d\n", n_threads);
        }

        // Print the thread ID
        printf("Hello World from thread = %ld\n", tid);

        // Compute some of the digits of pi
        DoWork(tid);
    } // All threads join master thread and terminate

#pragma omp parallel
    {
        long tid = omp_get_thread_num();
        // Only thread 0 does this
        if (tid == 0)
        {
            int n_threads = omp_get_num_threads();
            printf("Number of threads = %d\n", n_threads);
        }
        // Print the thread ID
        printf("Hello World from thread = %ld\n", tid);
        // Compute some of the digits of pi
        DoWork(tid);
    } // All threads join master thread and terminate

    return 0;
}
