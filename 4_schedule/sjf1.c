#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define NUM_THREADS 5 

// 전역 변수 선언
int curtime = 0;               // 현재 시간
double totalWaitTime = 0;      // 총 대기시간
double totalReturnTime = 0;    // 총 반환시간

// 프로세스 구조체
typedef struct Process {
    int id;                   // 프로세스 ID
    int arrivalTime;          // 도착시간
    int executionTime;        // 실행시간
    int waitTime;             // 대기시간
    int returnTime;           // 반환시간
    struct Process *next;     // 다음 프로세스를 가리키는 포인터
} Process;

// 큐 구조체
typedef struct {
    Process *front;  // 큐의 첫 번째 요소를 가리키는 포인터
    Process *back;   // 큐의 마지막 요소를 가리키는 포인터
} Queue;

// 큐 초기화
Queue *initQueue() {
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    queue->front = NULL;
    queue->back = NULL;
    return queue;
}

// 프로세스 추가
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

// 실행 시간 기준 정렬
int compareExecutionTime(const void *a, const void *b) {
    const Process *processA = (const Process *)a;
    const Process *processB = (const Process *)b;
    return processA->executionTime - processB->executionTime;
}

// 비선점 SJF
void *SJF(void *arg) {
    Process *process = (Process *)arg;
    int id = process->id;
    int end = process->executionTime;

    // 반환시간 / 대기시간 계산
    process->returnTime = end + curtime;
    process->waitTime = curtime - process->arrivalTime;

    // 칸트 차트 출력
    printf("P%d (%d-%d)\n", id, curtime, process->returnTime);
    int ret = process->returnTime - (process->arrivalTime);
    int wait = process->waitTime;
    printf("P%d [return:%d],[wait:%d]\n", id, ret, wait);

    for (int i = 1; i <= end; i++) {
        process->waitTime++; // 대기시간 
        printf("P%d: %d x %d = %d\n", id, i, id, i * id);
    }

    curtime += process->executionTime;
    totalWaitTime += wait;
    totalReturnTime += ret;

    pthread_exit(NULL);
}

int main() {
    // 스레드와 큐 초기화
    pthread_t threads[NUM_THREADS];
    Queue *queue = initQueue();

    // 도착시간과 실행시간 배열 초기화
    int arrivalTime[NUM_THREADS-1] = {1, 2, 3, 4};
    int executionTime[NUM_THREADS-1] = {28, 6, 4, 14};

    // 생성 및 즉시 실행
    Process process1 = {1, 0, 10, 0, 10, NULL};
    pthread_create(&threads[0], NULL, SJF, (void *)&process1);
    pthread_join(threads[0], NULL);

    // 프로세스 배열 생성 및 정렬
    Process processes[NUM_THREADS-1];
    for (int i = 0; i < NUM_THREADS-1; i++) {
        processes[i] = (Process){i + 2, arrivalTime[i], executionTime[i], 0, 0, NULL};
    }
    qsort(processes, NUM_THREADS-1, sizeof(Process), compareExecutionTime);

    // 정렬된 프로세스 큐에 추가
    for (int i = 0; i < NUM_THREADS-1; i++) {
        enqueue(queue, &processes[i]);
    }

    // 프로세스 2~5 실행
    for (int i = 0; i < NUM_THREADS-1; i++) {
        Process *process = dequeue(queue);
        pthread_create(&threads[i+1], NULL, SJF, (void *)process);
        pthread_join(threads[i+1], NULL);
    }

    // 평균 대기시간 / 반환시간
    double avgWaitTime = totalWaitTime / NUM_THREADS;
    double avgReturnTime = totalReturnTime / NUM_THREADS;
    printf("평균 대기시간: %.1f\n", avgWaitTime);
    printf("평균 반환시간: %.1f\n", avgReturnTime);

    return 0;
}
