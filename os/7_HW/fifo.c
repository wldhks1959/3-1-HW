#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define FRAME_SIZE 3

// 노드 구조체 정의
typedef struct Node 
{
    int data; // input data 
    int idx; // 페이지 인덱스
    struct Node* next;
} Node;

// 큐 구조체 정의
typedef struct Queue 
{
    Node* front;
    Node* back;
    int size;     // 큐 현재 크기 
    int capacity; // 큐 최대 용량 
} Queue;

// 큐 초기화 함수
Queue* initQueue(int capacity) 
{
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->front = queue->back = NULL;
    queue->size = 0;
    queue->capacity = capacity;
    return queue;
}

// 큐에 요소를 추가하는 함수
void enqueue(Queue* queue, int data, int idx) 
{
    // 새로운 노드 생성
    Node* tmp = (Node*)malloc(sizeof(Node));
    tmp->data = data;
    tmp->idx = idx;
    tmp->next = NULL;
    // 큐가 비어있으면 새로운 노드를 front와 back으로 설정
    if (queue->back == NULL) 
    {
        queue->front = queue->back = tmp;
    } 
    else 
    {
        queue->back->next = tmp;
        queue->back = tmp;
    }
    queue->size++;
}

// 큐에서 앞의 요소를 제거하는 함수
int dequeue(Queue* queue)
{
    if (queue->front == NULL) return -1;
    Node* tmp = queue->front;
    int idx = tmp->idx; // 제거된 노드의 인덱스를 저장
    queue->front = queue->front->next;

    if (queue->front == NULL) queue->back = NULL;
    free(tmp);
    queue->size--;
    return idx; // 제거된 노드의 인덱스 반환
}

// 큐에 특정 데이터가 존재하는지 확인하는 함수
bool isExist(Queue* queue, int data) 
{
    Node* tmp = queue->front;
    while (tmp != NULL) 
    {
        if (tmp->data == data) return true;
        tmp = tmp->next;
    }
    return false;
}

// 큐의 상태를 저장하는 함수
void QueueState(Queue* queue, int frameSize, int time, int result[][20]) 
{
    Node* tmp = queue->front;
    while (tmp != NULL) 
    {
        result[tmp->idx][time] = tmp->data; // 현재 노드의 데이터를 결과 배열의 해당 위치에 저장 
        tmp = tmp->next;
    }

    // 남은 프레임을 0으로 채우기
    for (int i = queue->size; i < frameSize; i++) 
        result[i][time] = 0;
}

int main() 
{
    int input[20] = {1, 2, 3, 2, 1, 5, 2, 1, 6, 2, 5, 6, 3, 1, 3, 6, 1, 2, 4, 3};
    int time = 20;
    int result[3][20] = {0}; // 프레임 상태를 저장할 배열
    int pageFaultIdx[20]; // 페이지 부재 참조 문자열 인덱스
    int pageFaultCnt = 0; // 페이지 부재 카운터 
    int curIdx = 0; // 현재 페이지 인덱스

    Queue* queue = initQueue(FRAME_SIZE); // 큐 초기화

    for (int i = 0; i < time; i++) 
    {
        if (!isExist(queue, input[i])) 
        {
            if (queue->size == FRAME_SIZE)
            {
                // 가장 오래된 요소 제거하고 인덱스를 가져옴
                curIdx = dequeue(queue);
            }
            enqueue(queue, input[i], curIdx);
            curIdx = (curIdx + 1) % FRAME_SIZE;
            pageFaultIdx[pageFaultCnt++] = i; // 페이지 부재 인덱스 저장 
        }
        QueueState(queue, FRAME_SIZE, i, result);
    }

    // 입력 상태 출력
    // printf("Input States:\n");
    // for (int i = 0; i < 20; i++) printf("%d ", input[i]);
    // printf("\n");

    // 프레임 상태 출력
    printf("Frame States:\n");
    for (int i = 0; i < FRAME_SIZE; i++) 
    {
        for (int j = 0; j < time; j++) 
            printf("%d ", result[i][j]);
        printf("\n");
    }

    // 페이지 부재가 발생한 인덱스 출력
    printf("Page Fault Idx: ");
    for (int i = 0; i < pageFaultCnt; i++) 
        printf("%d ", pageFaultIdx[i]);
    printf("\n");

    return 0;
}
