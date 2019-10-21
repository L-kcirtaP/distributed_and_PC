#include <cstdio>

int main()
{
#pragma omp parallel
  {
    printf("Hello CSC 4005\n");
  }
  return 0;
}
