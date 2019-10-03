#include <cstdio>
#include <cstdlib>
#include <cmath>


void odd_even_sort(double *array, int size){
    for (int i = 0; i < size; i++) {
        array[i] = i;
    }

    return;
}

int main(int argc, char *argv[]) {
    int size = atoi(argv[1]);
    double array[size];

    odd_even_sort(array, size);

    for (int i = 0; i < size; i++) {
        printf("%8f\n", array[i]);
    }

    return 0;
}