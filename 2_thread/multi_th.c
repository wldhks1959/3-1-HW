#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <time.h>

#define WEIGHT {3, 5, 7, 9} // 가중치 

// 구조체 선언
typedef struct 
{
    int start;
    int end;
    int weight; // 가중치
} Thread;

// 곱셈 출력
void* func(void* arg) 
{
    Thread* args = (Thread*)arg;
    for (int i = args->start; i <= args->end; i++) 
        printf("%d ", i * args->weight);
    pthread_exit(NULL); // thread exit 
}

int main()
{
    double start_time, end_time, multi_time;
    start_time = clock(); // 시간 측정 시작 

    int weight[4] = WEIGHT;

    for (int i = 0; i < 4; i++) 
    {
        pid_t pid = fork(); // 프로세스 생성

        if(pid==-1) // fork 실패 시 
        {
            fprintf(stderr, "fork failed");
            exit(1); // 프로그램 종료
        }

        else if (pid==0) // 자식 프로세스인 경우
        {
            pthread_t threads[4]; // 스레드 배열 선언 
            Thread arr[4]; // 스레드에 전달할 인자 배열 선언
            for(int j=0;j<4;j++) // 각 자식 프로세스마다 스레드 생성
            {
                arr[j].start = j*25 +1;
                arr[j].end = (j+1)*25;
                arr[j].weight = weight[i]; // 가중치 설정
               
                // 스레드 생성 실패 시 
                if(pthread_create(&threads[j], NULL, func, (void*)&arr[j]) !=0)
                {
                    fprintf(stderr,"thread create error\n");
                    exit(1);
                }
                // 프로세스 + 스레드 번호 출력 (개수 확인용)
                printf("\nprocess %d번->thread %d번\n",i+1, j+1);
            }

            // wait thread
            for(int j=0;j<4;j++)
            {
                // 스레드 대기 실패 시 
                if(pthread_join(threads[j],NULL)!=0)
                {
                    fprintf(stderr, "pthread_join error\n");
                    exit(1);
                }
            }
            exit(1); // 자식 프로세스 종료
        }   
    }

    // wait process
    for(int i=0;i<4;i++) wait(NULL);
    
    printf("\n");
    end_time = clock(); // 시간 측정 종료
    multi_time = (end_time - start_time) /CLOCKS_PER_SEC; // 소요 시간 계산
    printf("multi_time : %lf secs\n", multi_time); 

    return 0;
}