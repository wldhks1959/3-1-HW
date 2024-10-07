/*       input
    process AT BT 
       P1   0  10 
       P2   1  28 
       P3   2  06 
       P4   3  04 
       P5   4  14 
*/

/*  간트 차트 
    P1 (0-10)
    P4 (10-14)
    P3 (14-20)
    P5 (20-34)
    P2 (34-62)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // fork
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#define NUM_PROCESSES 5 // 프로세스의 총 수를 정의

// 프로세스 구조체 정의
typedef struct Process 
{
    int pid;              // 프로세스 ID
    int AT;               // 도착 시간
    int BT;               // 실행 시간
    int prev_BT;          // 이전 실행 시간
    int WT;               // 대기 시간
    int RT;               // 반환 시간
    double response_ratio;// 응답 비율
    bool isCompleted;     // 프로세스 완료 상태
    struct Process* next; // 다음 프로세스를 가리키는 포인터
} Process;

// 큐 구조체 정의
typedef struct Queue
{
    Process* front;       
    Process* back;        
} Queue;

// 전역 변수
int curT = 0;             // 현재 시간
double totalWT = 0;       // 총 대기 시간
double totalRT = 0;       // 총 반환 시간

// 프로세스 초기값
Process process1 = {1, 0, 10, 0, 0, 0, 0.0, false, NULL};
Process processes[NUM_PROCESSES - 1] = 
{
    {2, 1, 28, 0, 0, 0, 0.0, false, NULL},
    {3, 2, 6, 0, 0, 0, 0.0, false, NULL},
    {4, 3, 4, 0, 0, 0, 0.0, false, NULL},
    {5, 4, 14, 0, 0, 0, 0.0, false, NULL}
};

// 큐 초기화 함수
Queue *initQueue() 
{
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    queue->front = NULL;
    queue->back = NULL;
    return queue;
}

// 큐가 비어 있는지 확인
bool isEmpty(Queue* queue)
{
    return (queue->front == NULL);
}

// 큐에 프로세스 추가
void enqueue(Queue *queue, Process *process) 
{
    process->next = NULL;
    if (queue->back == NULL) 
    {
        queue->front = process;
        queue->back = process;
    } 
    else 
    {
        queue->back->next = process;
        queue->back = process;
    }
}

// 큐에서 프로세스 제거
Process *dequeue(Queue *queue) 
{
    if (queue->front == NULL) 
    {
        fprintf(stderr, "큐가 비어 있음.\n");
        exit(EXIT_FAILURE);
    }
    Process *process = queue->front;
    queue->front = queue->front->next;
    if (queue->front == NULL)
        queue->back = NULL;
    return process;
}

// HRN 스케줄링 함수
void hrn(Queue* queue) 
{
    while (!isEmpty(queue)) 
    {
        Process* highRRProcess = NULL;     // 높은 응답 비율 프로세스 
        Process* prev = NULL;              // 이전 프로세스 
        Process* highestRRPrev = NULL;     // 높은 응답 비율 프로세스의 이전 프로세스
        Process* iter = NULL;

        // 큐에서 가장 높은 응답 비율을 가진 프로세스 탐색
        for (Process* iter = queue->front; iter != NULL; prev = iter, iter = iter->next) 
        {
            // 도착 시간이 현재 시간보다 작거나 같고, 완료되지 않은 프로세스일 경우
            if (iter->AT <= curT && !iter->isCompleted) 
            {
                iter->WT = curT - iter->AT; // 대기 시간 계산
                
                // 응답 비율 = (대기 시간 + 실행 시간) / 실행 시간 
                iter->response_ratio = (double)(iter->WT + iter->BT) / iter->BT; // 응답 비율 계산

                // 높은 응답 비율 가진 프로세스 갱신
                if (highRRProcess == NULL || iter->response_ratio > highRRProcess->response_ratio) 
                {
                    highRRProcess = iter;
                    highestRRPrev = prev;
                }
            }
        }

        // 가장 높은 응답 비율을 가진 프로세스가 없는 경우 시간 증가
        if (highRRProcess == NULL) 
        {
            curT++;
            continue;
        }

        /* 가장 높은 응답 비율 가진 프로세스의 위치에 따라 경우를 나눔 */ 

        // 큐의 맨 앞일 경우 
        if (highestRRPrev == NULL) queue->front = highRRProcess->next;
        // 큐의 중간일 경우
        else highestRRPrev->next = highRRProcess->next;
        // 큐의 끝일 경우
        if (queue->back == highRRProcess) queue->back = highestRRPrev;

        // 프로세스 실행
        int startTime = curT; // 실행 시작 시간
        while (highRRProcess->BT > 0) 
        {
            printf("P%d: %d x %d = %d\n", 
                    highRRProcess->pid, highRRProcess->prev_BT + 1, 
                    highRRProcess->pid, (highRRProcess->prev_BT + 1) *  highRRProcess->pid);
            curT++; // 현재 시간 증가
            highRRProcess->prev_BT++;  // 이전 실행 시간 증가
            highRRProcess->BT--;       // 실행 시간 감소
        }

        // 간트 차트 출력
        printf("P%d (%d-%d)\n", highRRProcess->pid, startTime, curT);

        highRRProcess->RT = curT - highRRProcess->AT; // 반환 시간 계산
        highRRProcess->isCompleted = true;            // 프로세스 완료 상태 설정

        // 반환 시간 / 대기 시간 출력
        printf("P%d [return:%d], [wait:%d]\n", 
                highRRProcess->pid, highRRProcess->RT, highRRProcess->WT);

        // 총 대기 시간, 반환 시간 갱신
        totalWT += highRRProcess->WT;
        totalRT += highRRProcess->RT;
    }
}

int main() 
{
    Queue* queue = initQueue(); // 큐 생성 및 초기화
    pid_t pid[5]; // 프로세스 ID 배열
    
    pid[0] = getpid();
    enqueue(queue, &process1); // 프로세스1 실행

    // 모든 프로세스를 큐에 삽입
    for(int i = 0; i < NUM_PROCESSES - 1; i++)
        enqueue(queue, &processes[i]); 

    // HRN 스케줄링 함수 호출
    if(fork() != 0) 
    {
        while(!isEmpty(queue))
            hrn(queue);
    }

    for (int i = 0; i < NUM_PROCESSES; i++) 
    {
        wait(NULL); // 모든 자식 프로세스가 종료될 때까지 대기
        if(wait(NULL) == -1) break;
    } 
    if (pid[0] != getpid()) return 0; 

    // 평균 대기시간과 평균 반환시간 계산 및 출력
    double avgWT = totalWT / NUM_PROCESSES;
    double avgRT = totalRT / NUM_PROCESSES;
    printf("평균 대기시간: %.2f\n", avgWT);
    printf("평균 반환시간: %.2f\n", avgRT);

    return 0;
}