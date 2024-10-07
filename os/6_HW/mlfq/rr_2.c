#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // fork
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#define NUM_PROCESSES 3  // 프로세스의 총 수
#define TIME_QUANTUM 1   // 규정 시간 

// 프로세스 정보를 저장하는 구조체
typedef struct Process 
{
    int pid;               // 프로세스 ID
    int AT;               // 도착 시간
    int BT;               // 실행 시간
    int prev_BT;          // 이전 실행 시간
    int WT;               // 대기 시간
    int RT;               // 반환 시간
    bool isCompleted;     // 작업 완료 여부
    struct Process* next; // 다음 프로세스를 가리키는 포인터
} Process;

// 프로세스 큐 구조체
typedef struct Queue 
{
    Process* front; 
    Process* back;   
} Queue;

int curT = 0;          // 현재 시간
double totalWT = 0;    // 총 대기 시간
double totalRT = 0;    // 총 반환 시간

// 프로세스 초기값
Process process1 = {1, 0, 30, 0, 0, 0, false, NULL};
Process processes[NUM_PROCESSES - 1] = 
{
    {2, 0, 20, 0, 0, 0, false, NULL},
    {3, 0, 10, 0, 0, 0, false, NULL}
};

// 큐 초기화 함수
Queue *initQueue() 
{
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    queue->front = NULL;
    queue->back = NULL;
    return queue;
}

// 큐가 비었는지 확인
bool isEmpty(Queue* queue) 
{
    return (queue->front == NULL);
}

// 큐에 프로세스 추가
void enqueue(Queue *queue, Process *process) 
{
    process->next = NULL;
    if (queue->back == NULL) {
        queue->front = process;
        queue->back = process;
    } else {
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

// 라운드 로빈 스케줄링 
void RR(Queue* q, Process* process) 
{
    if (process->isCompleted) return;   // 이미 완료된 프로세스 실행 x 

    int timer = 0;                      // 실행 시간 카운터
    int prevBtSum = process->prev_BT;   // 이전까지 실행된 총 시간
    int startTime = curT;               // 시작 시간

    // 프로세스 실행
    while (timer < TIME_QUANTUM && process->BT > 0) 
    {
        // printf("P%d: %d x %d = %d\n", 
        //         process->pid, prevBtSum + 1, process->pid, prevBtSum + 1);
        curT++; // 현재 시간 증가
        timer++; // 실행 시간 카운터 증가 
        prevBtSum++; // 총 이전 실행 시간 증가
        process->BT--; // 남은 실행 시간 감소 
    }
    process->prev_BT = prevBtSum;  // 이전 총 실행 시간 업데이트

    // 간트 차트 출력 
    printf("P%d (%d-%d)\n", process->pid, startTime, curT);

    if (process->BT > 0) 
    {
        enqueue(q, process);
    }
    else 
    {
        process->RT = curT - process->AT;  // 반환 시간 계산
        process->WT = curT - process->AT - prevBtSum; // 대기 시간 계산
        process->isCompleted = true; // 프로세스 완료 설정
        printf("P%d [return:%d], [wait:%d]\n", process->pid, process->RT, process->WT);
        totalWT += process->WT;
        totalRT += process->RT;
    }  
}

// 메인 함수
int main() 
{
    Queue* queue = initQueue(); // 큐 생성 및 초기화
    pid_t pid[5]; // 프로세스 ID 배열

    pid[0] = getpid();
    enqueue(queue, &process1); // 프로세스1 실행
    
    // 프로세스 큐에 삽입
    for(int i = 0; i < NUM_PROCESSES - 1; i++) {
        enqueue(queue, &processes[i]);
    }
    
    if(fork() != 0) 
    {
        while(!isEmpty(queue))
            RR(queue, dequeue(queue)); 
    }

    for (int i = 0; i < NUM_PROCESSES; i++) 
    {
        wait(NULL); // 모든 자식 프로세스가 종료될 때까지 대기
        if(wait(NULL) == -1) break;
    } 
    if (pid[0] != getpid()) return 0; 

    // 평균 대기 시간 / 평균 반환 시간 계산 및 출력
    double avgWT = totalWT / NUM_PROCESSES;
    double avgRT = totalRT / NUM_PROCESSES;
    printf("평균 대기 시간: %.2f\n", avgWT);
    printf("평균 반환 시간: %.2f\n", avgRT);

    return 0;
}
