/*        input
    process AT BT Priority
       P1   0  10    3    
       P2   1  28    2    
       P3   2  06    4    
       P4   3  04    1    
       P5   4  14    2    
*/

/*  간트 차트 
    P1 (0-10)
    P3 (10-16)
    P2 (16-44)
    P5 (44-58)
    P4 (58-62)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // fork
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_PROCESSES 5 

int curT = 0;             // 현재 시간 
double totalWT = 0;       // 총 대기 시간 
double totalRT = 0;       // 총 반환 시간 

typedef struct Process {
    int pid;              // 프로세스 ID 
    int AT;               // 도착 시간 
    int BT;               // 실행 시간 
    int WT;               // 대기 시간 
    int RT;               // 반환 시간 
    int priority;         // 우선순위
    struct Process* next; // 다음 프로세스를 가리키는 포인터
} Process;

typedef struct Queue{
    Process* front;
    Process* back;
} Queue;

// 큐를 초기화
Queue *initQueue() {
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    queue->front = NULL;
    queue->back = NULL;
    return queue;
}

// 프로세스를 큐의 뒤에 추가
void enqueue(Queue *queue, Process *process) {
    process->next = NULL;
    if (queue->back == NULL) {
        queue->front = process;
        queue->back = process;
    } else {
        queue->back->next = process;
        queue->back = process;
    }
}

// 프로세스 제거
Process *dequeue(Queue *queue) {
    if (queue->front == NULL) {
        fprintf(stderr, "큐가 비어 있음.\n");
        exit(EXIT_FAILURE);
    }
    Process *process = queue->front;
    queue->front = queue->front->next;
    if (queue->front == NULL)
        queue->back = NULL;
    return process;
}

// 우선 순위 기준 정렬
int comparePriority(const void *a, const void *b) 
{
    const Process *processA = (const Process *)a;
    const Process *processB = (const Process *)b;
    return processB->priority - processA->priority;
}

// 비선점 우선 순위 스케줄링 
void* nps(Process* process)
{
    int pid = process->pid;
    int end = process->BT;

    // 반환 시간 / 대기 시간 계산 
    process->RT = end + curT;
    process->WT = curT - process->AT;

    // 간트 차트 출력 
    printf("P%d (%d-%d)\n", pid, curT, process->RT);
    int ret = process->RT - (process->AT);
    int wait = process->WT;
    printf("P%d [return:%d], [wait:%d]\n", pid, ret, wait);

    for(int i = 1 ; i <= end ; i++)
    {
        process->WT++; // 대기 시간 증가
        printf("P%d: %d x %d = %d\n", pid, i, pid, i*pid);
    }

    curT += process -> BT;
    totalWT += wait;
    totalRT += ret;
}

int main()
{
    // var
    Queue* queue = initQueue();
    Process process1;
    Process processes[NUM_PROCESSES-1];
    pid_t pid[5];

    // 프로세스1 실행
    pid[0] = getpid();
    process1 = (Process){1, 0, 10, 0, 0, 3, NULL};
    nps(&process1);
    
    processes[0] = (Process){2, 1, 28, 0, 0, 2, NULL};
    processes[1] = (Process){3, 2, 6, 0, 0, 4, NULL};
    processes[2] = (Process){4, 3, 4, 0, 0, 1, NULL};
    processes[3] = (Process){5, 4, 14, 0, 0, 2, NULL};
    qsort(processes, NUM_PROCESSES - 1, sizeof(Process), comparePriority);

    // 정렬된 프로세스 큐 삽입 
    for(int i=0 ; i < NUM_PROCESSES - 1 ; i++)
        enqueue(queue, &processes[i]);
    
    if(fork()!=0)
    {
        for(int i=0;i<NUM_PROCESSES-1;i++)
        {
            nps(dequeue(queue));

        }   
    }

    // 모든 자식 끝날 때까지 대기 
    for (int i = 0; i < NUM_PROCESSES; i++)
    {
        wait(NULL);
        if(wait(NULL)==-1) break;
    } 
    if (pid[0] != getpid()) return 0; 

    double avgWT = totalWT / NUM_PROCESSES;
    double avgRT = totalRT / NUM_PROCESSES;
    printf("평균 대기시간: %.1f\n", avgWT);
    printf("평균 반환시간: %.1f\n", avgRT);

    return 0;
}