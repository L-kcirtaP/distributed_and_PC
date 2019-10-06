#include <cstdio>
#include <cstdlib>
#include <cmath>
#include<time.h>


void odd_even_sort(int *array, int size){
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
    int array[size];
    clock_t start, end;

    printf("Name: Liu Yang\n");
    printf("Student ID: 116010151\n");
    printf("Assignment 1, Odd-Even Transposition Sort, Sequential Implementation\n");

    printf("Before sort:\t");
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 100000;
        printf("%d\t", array[i]);
    }

    start = clock();
    odd_even_sort(array, size);
    end = clock();

    printf("\nAfter sort:\t");
    for (int i = 0; i < size; i++) {
        printf("%d\t", array[i]);
    }
    printf("\nElapsed Time: %8fs\n", ((double)end-(double)start)/CLOCKS_PER_SEC);

    return 0;
}
