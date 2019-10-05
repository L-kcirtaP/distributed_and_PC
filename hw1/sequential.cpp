#include <cstdio>
#include <cstdlib>
#include <cmath>
#include<time.h>


void odd_even_sort(double *array, int size){
    bool sorted = false;
    
    while (!sorted) {
        sorted = true;

        // sort in odd positions
        for (int i = 0; i < size-1; i += 2) {
            if (array[i] > array[i+1]) {
                int temp = array[i];
                array[i] = array[i+1];
                array[i+1] = temp;
                sorted = false;
            }
        }

        // sort in even positions
        for (int i = 1; i < size-1; i += 2) {
            if (array[i] > array[i+1]) {
                int temp = array[i];
                array[i] = array[i+1];
                array[i+1] = temp;
                sorted = false;
            }
        }
    }

    return;
}


int main(int argc, char *argv[]) {
    int size = atoi(argv[1]);
    double array[size];
    clock_t start, end;

    for (int i = 0; i < size; i++) {
        array[i] = rand() % 100000;
    }

    start = clock();
    odd_even_sort(array, size);
    end = clock();

    for (int i = 0; i < size; i++) {
        printf("%8f ", array[i]);
    }
    printf("\nTotal elapsed time: %8fs\n", ((double)end-(double)start)/CLOCKS_PER_SEC);

    return 0;
}