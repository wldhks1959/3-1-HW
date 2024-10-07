#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define NUM_THREADS 5

int curtime = 0;               // 현재 시간
double totalWaitTime = 0;      // 총 대기 시간
double totalReturnTime = 0;    // 총 반환 시간

// 프로세스를 나타내는 구조체
typedef struct Process {
    int id;                   // 프로세스 ID
    int arrivalTime;          // 도착 시간
    int executionTime;        // 실행 시간
    int waitTime;             // 대기 시간
    int returnTime;           // 반환 시간
    struct Process *next;     // 다음 프로세스를 가리키는 포인터
} Process;

// 큐를 나타내는 구조체
typedef struct Queue {
    Process *front;
    Process *back;
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

// 큐의 앞에서 프로세스를 제거
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

// 프로세스를 처리하는 스레드 함수
void *FCFS(void *arg) {
    Process *process = (Process *)arg;
    int id = process->id;
    int end = process->executionTime;

    // 프로세스의 반환시간 / 대기시간 계산
    process->returnTime = end + curtime;
    process->waitTime = curtime - process->arrivalTime;

    // 간트 차트를 출력
    printf("P%d (%d-%d)\n", process->id, curtime, process->returnTime);
    int ret = process->returnTime - (process->arrivalTime);
    printf("P%d [return:%d],[wait:%d]\n", process->id, ret, process->waitTime);

    curtime += process->executionTime;
    totalWaitTime += process->waitTime;
    totalReturnTime += ret;

    for (int i = 1; i <= end; i++)
    { // 대기시간 계산
        process->waitTime++;
        printf("P%d: %d x %d = %d\n", id, i, id, i * id);
    }

    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[NUM_THREADS];
    Queue *queue = initQueue();
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 뮤텍스 초기화

    // 각 프로세스의 도착시간과 실행시간
    int arrivalTime[NUM_THREADS] = {0, 1, 2, 3, 4};
    int executionTime[NUM_THREADS] = {10, 28, 6, 4, 14};

    // 도착시간에 따라 프로세스를 큐에 추가
    for (int i = 0; i < NUM_THREADS; i++)
    {
        Process *process = (Process *)malloc(sizeof(Process));
        process->id = i + 1;
        process->arrivalTime = arrivalTime[i];
        process->executionTime = executionTime[i];
        process->waitTime = 0;
        process->returnTime = 0;
        enqueue(queue, process);
    }

    // 각 프로세스를 처리하기 위해 스레드를 생성하고 큐에서 제거
    for (int i = 0; i < NUM_THREADS; i++)
    {
        Process *process = dequeue(queue);
        pthread_create(&threads[i], NULL, FCFS, (void *)process);
        pthread_join(threads[i], NULL);
    }

    // 평균 대기시간과 평균 반환시간을 출력
    printf("평균 대기시간: %.1f\n", totalWaitTime / 5.0);
    printf("평균 반환시간: %.1f\n", totalReturnTime / 5.0);

    return 0;
}
