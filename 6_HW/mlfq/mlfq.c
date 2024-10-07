#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#define NUM_PROCESSES 3
#define Q1_TIME_QUANTUM 1
#define Q2_TIME_QUANTUM 2
#define Q3_TIME_QUANTUM 4

// 프로세스 구조체 정의
typedef struct Process 
{
    int pid;            // 프로세스 ID
    int BT;             // 실행 시간
    int prev_BT;        // 이전 실행 시간
    int start_time;     // 시작 시간
    int WT;             // 대기 시간
    int RT;             // 반환 시간
    bool isCompleted;   // 완료 여부
    struct Process* next;   // 다음 프로세스를 가리키는 포인터
} Process;

// 큐 구조체 정의
typedef struct Queue 
{
    Process* front;     
    Process* back;      
    const char* QID;   // 큐 번호 
} Queue;

// 전역 변수
int curT = 0;           // 현재 시간
double totalWT = 0;     // 총 대기 시간
double totalRT = 0;     // 총 반환 시간

// 프로세스 초기값 설정
Process processes[NUM_PROCESSES] = 
{
    {1, 30, 0, -1, 0, 0, false, NULL},
    {2, 20, 0, -1, 0, 0, false, NULL},
    {3, 10, 0, -1, 0, 0, false, NULL}
};

// 큐를 초기화하는 함수
Queue* initQueue(const char* qid) 
{
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->front = NULL;
    queue->back = NULL;
    queue->QID = qid;
    return queue;
}    

// 큐가 비어있는지 확인하는 함수
bool isEmpty(Queue* queue) 
{
    return (queue->front == NULL);
}

// 큐에 프로세스를 추가하는 함수
void enqueue(Queue* queue, Process* process) 
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

// 큐에서 프로세스를 제거하는 함수
Process* dequeue(Queue* queue) 
{
    if (queue->front == NULL) 
    {
        fprintf(stderr, "큐가 비어 있음.\n");
        exit(EXIT_FAILURE);
    }
    Process* process = queue->front;
    queue->front = queue->front->next;
    if (queue->front == NULL)
        queue->back = NULL;
    return process;
}

// 프로세스를 실행하는 함수
void executeProcess(Process* process, int timeQuantum, Queue* nextQueue, const char* qid) 
{
    int timer = 0;        // 현재 타이머 값
    int startTime = curT; // 현재 시간을 시작 시간으로 설정

    // 프로세스가 처음 시작되는 경우 시작 시간 설정
    if (process->start_time == -1) 
    {
        process->start_time = curT; // 처음 시작하는 경우 현재 시간을 시작 시간으로 설정
    }

    // 주어진 timeQuantum 동안 또는 프로세스가 완료될 때까지 실행
    while ((timeQuantum == 0 || timer < timeQuantum) && process->BT > 0) 
    {
        printf("P%d: %d x %d = %d\n", 
                process->pid, process->prev_BT + 1, process->pid, process->prev_BT + 1);
        
        curT++;              // 현재 시간 증가
        timer++;             // 타이머 증가
        process->prev_BT++;  // 이전 실행 시간 증가
        process->BT--;       // 실행 시간 감소
    }

    // 간트 차트 출력
    printf("%s P%d (%d-%d)\n", qid, process->pid, startTime, curT);

    // 프로세스가 완료되지 않았을 경우
    if (process->BT > 0) 
    {
        // 다음 큐가 NULL이 아닌 경우 다음 큐에 프로세스를 추가
        if (nextQueue != NULL) 
        {
            enqueue(nextQueue, process);
        }
    } 
    else 
    // 프로세스가 완료된 경우 
    {
        // 반환 시간 및 대기 시간 계산
        process->RT = curT;   // 반환 시간 
        process->WT = process->RT - process->prev_BT; // 대기 시간 
        process->isCompleted = true; // 프로세스 완료 표시
        totalWT += process->WT; // 총 대기 시간 갱신
        totalRT += process->RT; // 총 반환 시간 갱신
        // 프로세스 반환 / 대기 시간 출력
        printf("P%d [return:%d], [wait:%d]\n", process->pid, process->RT, process->WT);
    }
}

// mlfq 스케줄링 함수
void mlfq(Process processes[], int n, Queue* Q1, Queue* Q2, Queue* Q3) 
{
    // 모든 큐가 빌 때까지 반복
    while (!isEmpty(Q1) || !isEmpty(Q2) || !isEmpty(Q3)) 
    {
        // Q1에서 프로세스 처리 
        while (!isEmpty(Q1)) 
        {
            Process* process = dequeue(Q1);
            executeProcess(process, Q1_TIME_QUANTUM, Q2, "Q1");
        }

        // Q2에서 프로세스 처리 
        while (!isEmpty(Q2)) 
        {
            Process* process = dequeue(Q2);
            executeProcess(process, Q2_TIME_QUANTUM, Q3, "Q2");
        }

        // Q3에서 프로세스 처리 
        while (!isEmpty(Q3)) 
        {
            Process* process = dequeue(Q3);
            // Q3가 비어있는 경우 
            if (isEmpty(Q3)) 
            {
                // timeQuantum을 0으로 설정; 완료될 때까지 실행
                executeProcess(process, 0, NULL, "Q1"); 
            } 
            else 
            {
                executeProcess(process, Q3_TIME_QUANTUM, Q3, "Q3");
            }
        }
    }
}

int main() 
{
    pid_t pids[NUM_PROCESSES]; // 프로세스 ID 배열 
    pids[0] = getpid();

    // 큐 초기화
    Queue* Q1 = initQueue("Q1");
    Queue* Q2 = initQueue("Q2");
    Queue* Q3 = initQueue("Q3");
    
    for(int i = 0; i < NUM_PROCESSES ; i++)
        enqueue(Q1, &processes[i]); 

    // mlfq 실행
    if(fork() != 0) 
    {
        mlfq(processes, NUM_PROCESSES, Q1, Q2, Q3); 
    }


    for (int i = 0; i < NUM_PROCESSES; i++) 
    {
        wait(NULL); // 모든 자식 프로세스가 종료될 때까지 대기
        if(wait(NULL) == -1) break;
    } 

    if (pids[0] != getpid()) return 0; 
    
    // 부모 프로세스에서 평균 대기 시간과 반환 시간 출력
    double avgWT = totalWT / NUM_PROCESSES;
    double avgRT = totalRT / NUM_PROCESSES;
    printf("평균 대기 시간: %.2f\n", avgWT);
    printf("평균 반환 시간: %.2f\n", avgRT);

    return 0;
}