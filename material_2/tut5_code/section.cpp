#include <omp.h>
#include <vector>
#include <cassert>

using std::vector;

int main()
{
    unsigned size = (1 << 13);
    unsigned inner = (1 << 13);
    vector<float> a(size), c(size), d(size);
    vector<unsigned> b(size);

    // Some initializations
    for (unsigned i(0); i < size; i++)
    {
        a[i] = 1.0 / i;
        b[i] = 1 + i % 7;
        c[i] = 0.;
        d[i] = 0.;
    }

#pragma omp parallel
    {
#pragma omp sections
        {
#pragma omp section
            for (unsigned i(0); i < size; i++)
                for (unsigned k(0); k < inner; k++)
                    c[i] += a[(i * k) % size];
#pragma omp section
            for (unsigned i(0); i < size; i++)
                for (unsigned k(0); k < inner; k++)
                    d[i] += a[(b[i] + i * k) % size];
        } // end of sections
    }     // end of parallel block

    return 0;
}
