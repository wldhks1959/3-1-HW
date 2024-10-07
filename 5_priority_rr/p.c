/*        input
    process AT BT Priority
       P1   0  10    3    
       P2   1  28    2    
       P3   2  06    4    
       P4   3  04    1    
       P5   4  14    2    
*/

/*  간트 차트 
    P1 (0-2)
    P3 (2-8)
    P1 (8-16)
    P2 (16-44)
    P5 (44-58)
    P4 (58-62)
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
    int prev_BT;          // 이전에 실행된 시간
    int WT;               // 대기 시간
    int RT;               // 반환 시간
    int priority;         // 프로세스 우선순위
    bool isCompleted;     // 프로세스 완료 상태
    struct Process* next; // 다음 프로세스를 가리키는 포인터
} Process;

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
Process process1 = {1, 0, 10, 0, 0, 0, 3, false, NULL};
Process processes[NUM_PROCESSES - 1] = 
{
    {2, 1, 28, 0, 0, 0, 2, false, NULL},
    {3, 2, 6, 0, 0, 0, 4, false, NULL},
    {4, 3, 4, 0, 0, 0, 1, false, NULL},
    {5, 4, 14, 0, 0, 0, 2, false, NULL}
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
bool isEmpty(Queue*queue)
{
    return (queue->front == NULL);
}

// 큐에 프로세스 추가
void enqueue(Queue *queue, Process *process) {
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

// 우선순위에 따라 프로세스 정렬
int comparePriority(const void *a, const void *b) 
{
    const Process *processA = (const Process *)a;
    const Process *processB = (const Process *)b;
    return processB->priority - processA->priority;
}

// 선점 우선 순위 스케줄링 
void pps(Queue* q, Process* process) 
{
    if (process->isCompleted) return; // 이미 완료된 프로세스는 실행하지 않음

    int timer = 0; // 실행 시간 카운터
    int pid = process->pid; 
    int startTime = curT; // 프로세스 시작시간 

    // 프로세스의 실행 시간이 완료될 때까지 반복
    while (timer < process->BT) 
    {
        // 대기 시간 계산 : 시작 시간 - 도착 시간 - 이전에 실행된 시간
        process->WT = startTime - process->AT - process->prev_BT;

        printf("P%d: %d x %d = %d\n", 
                pid, timer + 1 + process->prev_BT, pid, (timer + 1) * pid);

        curT++;   // 현재 시간 증가
        timer++;  // 현재 프로세스의 실행 시간 카운트

        // 모든 프로세스를 검사하여 현재 시간에 도착하고 우선 순위가 더 높은 프로세스가 있는지 확인
        for (int i = 0; i < NUM_PROCESSES - 1; i++) 
        {
            // 도착 시간과 우선 순위 확인
            if (processes[i].AT == curT && processes[i].priority > process->priority) 
            {
                // 간트 차트 출력 
                printf("P%d (%d-%d)\n", pid, startTime, curT);
                process->BT -= timer;      // 남은 실행시간 계산
                process->prev_BT += timer; // 이전 실행시간 업데이트
                enqueue(q, process);       // 현재 프로세스를 큐에 다시 삽입
                pps(q, &processes[i]);     // 더 높은 우선순위의 프로세스 실행
                return;
            }
        }
    }

    // 간트 차트 출력 
    printf("P%d (%d-%d)\n", pid, startTime, curT);

    process->RT = curT - process->AT; // 반환 시간 계산
    process->isCompleted = true;  // 프로세스 완료 상태 설정

    // 반환 시간 / 대기 시간 출력
    printf("P%d [return:%d], [wait:%d]\n", pid, process->RT, process->WT);

    totalWT += process->WT;
    totalRT += process->RT;
}


int main() 
{
    Queue* queue = initQueue(); // 큐 생성 및 초기화
    pid_t pid[5]; // 프로세스 ID 배열

    pid[0] = getpid();
    pps(queue, &process1); // 프로세스1 실행
    
    // 우선순위에 따라 프로세스 정렬
    qsort(processes, NUM_PROCESSES - 1, sizeof(Process), comparePriority); 

    // 정렬된 프로세스를 큐에 삽입
    for(int i = 0; i < NUM_PROCESSES - 1; i++)
        enqueue(queue, &processes[i]); 
    
    if(fork() != 0) 
    {
        while(!isEmpty(queue))
            pps(queue, dequeue(queue)); 
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
    printf("평균 대기시간: %.1f\n", avgWT);
    printf("평균 반환시간: %.1f\n", avgRT);

    return 0;
}