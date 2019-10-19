#include "mpich/mpi.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <sys/time.h>

#define MASTER 0

using namespace std;

int main(int argc, char *argv[])
{

  // Some MPI magic to get started
  MPI_Init(&argc, &argv);

  // How many processes are running
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  // What's my rank?
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  // Which node am I running on?
  int len;
  char hostname[MPI_MAX_PROCESSOR_NAME];
  MPI_Get_processor_name(hostname, &len);
  int array_size=10000;
  int num_task=100;
  int sub_divide=array_size/num_task;
  int tag=0;
  //Enter into control process
  if (rank == MASTER)
  {
    int *array = new int[array_size];
    int *recv_array=new int[sub_divide+2];//the first element is rank, the second element is start_index
    int *result_array=new int[array_size];
    
    // store the idle cores into a queue
    queue<int> avail_core;
    for (int p=1;p<size;p++){
      avail_core.push(p);
    }
    srand(time(NULL));
    for (int z = 0; z < array_size; z++) {
      array[z] = rand() - rand();
    }
    //printf("array[0] is %d\n",array[0]);
    //printf("MASTER process: the number of MPI tasks is: %2d\n", size);
    int current_task=0;
    int next_core;
    int start_index;
    int recv_rank;
    int recv_start_index;
    int finished_task=0;
    //compute the running time of the program
    struct timeval timeStart, timeEnd, timeSystemStart; 
    double runTime=0, systemRunTime; 
    gettimeofday(&timeStart, NULL );
    
    while (finished_task<num_task){
        //send tasks to idel cores until all the cores are occupied
        while (avail_core.empty()==0 && current_task<num_task){
            next_core=avail_core.front();
            avail_core.pop();
            //start_index indicate which part of the result this sub-task corresponds to
            start_index=sub_divide*current_task;
            MPI_Send(&start_index,1,MPI_INT,next_core,tag,MPI_COMM_WORLD);
            MPI_Send(array+start_index, sub_divide, MPI_INT,next_core, tag, MPI_COMM_WORLD);
            current_task++;
        }
        MPI_Status status;
        //receive from any cores 
        MPI_Recv(recv_array,sub_divide+2,MPI_INT,MPI_ANY_SOURCE,tag, MPI_COMM_WORLD, &status);
        //recv_array[0] stores the core's id.
        recv_rank=recv_array[0];
        //recv_start_index indicates which part of the result this sub-task corresponds to
        recv_start_index=recv_array[1];
        for (int index=0;index<sub_divide;index++){
          //store the result received from the other process.
          result_array[index+recv_start_index]=recv_array[index+2];
        }
        //the core from which received is now idle, therefore, put this back to the queue.
        avail_core.push(recv_rank);
        finished_task++;
    }
    //send a stop signal to all the cores, so to stop the program
    int stop_signal=-1;
    for (int p=1;p<size;p++){
        MPI_Send(&stop_signal,1,MPI_INT,p,tag,MPI_COMM_WORLD);
    }
    //verify the results
    for (int i=0;i<10;i++){
        printf("array[%d] is %d, result_array[%d] is %d\n",i,array[i],i,result_array[i]);
    }
    //printf("the program is finished\n");
    gettimeofday( &timeEnd, NULL ); 
    runTime = (timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000;  
    printf("Name: Liu Haolin\n");
    printf("Student ID: 115010102\n");
    printf("demo, MPI dynamic schedulling.\n");
    printf("runTime is %lf\n", runTime); 
  }
  //enter into compute core
  if (rank!=MASTER){
    //printf("entering into process %d\n",rank);
    int start_index;
    int *recv_array=new int[sub_divide];
    int *send_array=new int[sub_divide+2];//
    MPI_Status status;
    while (1){
        MPI_Recv(&start_index,1,MPI_INT,MASTER,tag,MPI_COMM_WORLD,&status);
        //if start_index is -1, all the tasks are finished
        if (start_index==-1){
          break;
        }
        MPI_Recv(recv_array,sub_divide,MPI_INT,MASTER,tag,MPI_COMM_WORLD,&status);
        //the computation part
        for (int i=0;i<sub_divide;i++){
          send_array[i+2]=recv_array[i]/200;
          send_array[i+2]=send_array[i+2]+120;
          send_array[i+2]=send_array[i+2]*2;
        }
        send_array[0]=rank;
        send_array[1]=start_index;
        //printf("send_array[2] is %d\n",send_array[2]);
        MPI_Send(send_array, sub_divide+2, MPI_INT,MASTER, tag, MPI_COMM_WORLD);
    }
  }

  // Close down all MPI magic
  MPI_Finalize();

  return 0;
}
