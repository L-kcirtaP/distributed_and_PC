#include <mpi.h>
#include <cstdio>
#include <cstdlib>

#define ROOT 0
#define SORTED 0
#define UNSORTED 1
#define PLACEHOLDER 0

void sort(int* local_arr, int local_n, int rank, int num_p);


int main(int argc, char* argv[]) {

    int global_n = atoi(argv[1]);
    int* global_arr;
    int* local_arr;
    int rank, num_p, local_n;
    int additional = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_p);

    if (global_n % num_p == 0) {
        local_n = global_n / num_p;
        global_arr = (int*) malloc(global_n*sizeof(int));
    } else {
        additional = num_p - (global_n % num_p);
        local_n = global_n / num_p + 1;
        global_arr = (int*) malloc((global_n+additional)*sizeof(int));
    }

    local_arr = (int*) malloc(local_n*sizeof(int));

    srand(2019 + (rank<<2));

    if (rank == ROOT) {
        // printf("The %d-dim Array Before Sorting:\t", global_n);
        for (int i = 0; i < global_n; i++) {
            global_arr[i] = rand() % 10000;
            // printf("%d\t", global_arr[i]);
        }
        // printf("\n");
        for (int i = global_n; i < global_n+additional; i++)
            global_arr[i] = PLACEHOLDER;
    }
        
    MPI_Scatter(global_arr, local_n, MPI_INT, local_arr, local_n, MPI_INT, ROOT, MPI_COMM_WORLD);

    double start_time = MPI_Wtime();
    sort(local_arr, local_n, rank, num_p);
    double finish_time = MPI_Wtime();

    MPI_Gather(local_arr, local_n, MPI_INT, global_arr, local_n, MPI_INT, ROOT, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == ROOT) {
        // printf("The %d-dim Array After Sorting:\t\t", global_n);
        // for (int i = additional; i < global_n+additional; i++)
        //     printf("%d\t", global_arr[i]);
        // printf("\n");
        printf("%d Processes %d Elements: Elapsed Time = %e seconds\n", num_p, global_n,finish_time-start_time);
    }

    MPI_Finalize();

    return 0;
}


void sort(int* local_arr, int local_n, int rank, int num_p) {
    int local_sorted = UNSORTED;   // 0: sorted, 1: unsorted
    int sorted = UNSORTED;         // 0: sorted, 1: unsorted
    int recdata_1, recdata_2;
    MPI_Status status;

    while (sorted == UNSORTED) {
        local_sorted = SORTED;

        // sort in odd positions
        for (int i = 0; i < local_n-1; i += 2) {
            if (local_arr[i] > local_arr[i+1]) {
                int temp = local_arr[i];
                local_arr[i] = local_arr[i+1];
                local_arr[i+1] = temp;
                local_sorted = UNSORTED;
            }
        }

        // sort in even positions
        for (int i = 1; i < local_n-1; i += 2) {
            if (local_arr[i] > local_arr[i+1]) {
                int temp = local_arr[i];
                local_arr[i] = local_arr[i+1];
                local_arr[i+1] = temp;
                local_sorted = UNSORTED;
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);        

        if (rank < num_p-1) {
            MPI_Send(&local_arr[local_n-1], 1, MPI_INT, rank+1, 1, MPI_COMM_WORLD);
            // printf("Process %d sends data to next rank!\n", rank);
            MPI_Recv(&recdata_1, 1, MPI_INT, rank+1, 1, MPI_COMM_WORLD, &status);
            // printf("RECEIVE DATA %d!!!\n", recdata_1);
            if (recdata_1 >= 0) {
                // printf("Process %d receives data from next rank!\n", rank);
                local_arr[local_n-1] = recdata_1;
                local_sorted = UNSORTED;
            }
        }

        if (rank > 0) {
            MPI_Recv(&recdata_2, 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD, &status);
            // printf("Compare! Process %d receives data from previous rank!\n", rank);
            int temp;
            if (recdata_2 > local_arr[0]) {
                temp = local_arr[0];
                local_arr[0] = recdata_2;
                MPI_Send(&temp, 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD);
                // printf("Swapping! Process %d send data to previous rank!\n", rank);
            } else {
                temp = -1;
                MPI_Send(&temp, 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD);
            }
        }

        // printf("AFTER: Rank %d: ", rank);
        // for (int i = 0; i < local_n; i++)
        //     printf("%d, ", local_arr[i]);
        // printf("\n");

        MPI_Allreduce(&local_sorted, &sorted, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    }

    return;
}
