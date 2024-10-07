#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <time.h>

#define NUM_PROCESS 8
#define ITER 1000

void multiProcess(int start, int end) 
{
    for (int i = start; i <= end; i++) 
        printf("%d ", i * 7);
}

int main() 
{
    double start_time, end_time, multi8_time;
    int self = getpid();

    start_time = clock();

        fork(); fork(); fork();

        int pid = getpid(); 

        int num = pid % self; // num : 프로세스 ID 
    
        // printf("PID : %d\n", num); // PID 체크용 함수

        int start = num * (ITER / NUM_PROCESS) + 1;
        int end = (num + 1) * (ITER / NUM_PROCESS);

        multiProcess(start, end);
        
        // 모든 자식 프로세스 종료될 때까지 대기 
        for (int i = 0; i < NUM_PROCESS; i++)
        {
            wait(NULL);
            if(wait(NULL)==-1) break;
        } 

        if (pid != self) return 0; 


    end_time = clock();

    multi8_time = (end_time - start_time) / CLOCKS_PER_SEC;
    printf("\nMulti process time : %lf\n", multi8_time);

    return 0;
}