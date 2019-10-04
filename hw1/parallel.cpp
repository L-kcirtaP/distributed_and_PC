#include <mpi.h>
#include <cstdio>
#include <cstdlib>

#define ROOT 0
#define SORTED 0
#define UNSORTED 1

void sort(int* local_arr, int local_n, int rank, int num_p);

/*-------------------------------------------------------------------*/
int main(int argc, char* argv[]) {

    const int global_n = 50;
    int global_arr[global_n];
    int rank, num_p;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_p);

    srand(2019 + (rank<<2));

    if (rank == ROOT) {
        for (int i = 0; i < global_n; i++)
            global_arr[i] = rand() % 10000;
    }
    
    const int local_n = global_n / num_p;
    int* local_arr = (int*) malloc(local_n*sizeof(int));
    
    MPI_Scatter(global_arr, local_n, MPI_INT, local_arr, local_n, MPI_INT, ROOT, MPI_COMM_WORLD);

    double start_time = MPI_Wtime();
    sort(local_arr, local_n, rank, num_p);
    double finish_time = MPI_Wtime();

    MPI_Gather(local_arr, local_n, MPI_INT, global_arr, local_n, MPI_INT, ROOT, MPI_COMM_WORLD);

    if (rank == ROOT)
        // printf("Elapsed time = %e seconds\n", finish_time-start_time);
        for (int i = 0; i < global_n; i++)
            printf("%d ", global_arr[i]);

    MPI_Finalize();

    return 0;
}


void sort(int* local_arr, int local_n, int rank, int num_p) {
    int local_sorted = UNSORTED;   // 0: sorted, 1: unsorted
    int sorted = UNSORTED;         // 0: sorted, 1: unsorted
    int recdata_1, recdata_2;
    MPI_Status status;

    const int sorted_tag = 2;

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

        // printf("BEFORE: Rank %d: ", rank);
        // for (int i = 0; i < local_n; i++)
        //     printf("%d, ", local_arr[i]);
        // printf("\n");

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
