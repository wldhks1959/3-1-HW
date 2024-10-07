#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define FRAME_SIZE 3
// 노드 구조체 정의
typedef struct Node 
{
    int data; 
    int idx; 
    struct Node* next; 
} Node;

// 큐 구조체 정의
typedef struct Queue 
{
    Node* front; 
    Node* back; 
    int size;     // 큐의 현재 크기
    int capacity; // 큐의 최대 용량
} Queue;

// 큐 초기화 함수
Queue* initQueue(int capacity) 
{
    Queue* queue = (Queue*)malloc(sizeof(Queue)); 
    queue->front = queue->back = NULL; 
    queue->size = 0; // 큐의 초기 크기를 0으로 설정
    queue->capacity = capacity; // 큐의 최대 용량 설정
    return queue;
}

// 큐에 요소를 추가하는 함수
void enqueue(Queue* queue, int data, int idx) 
{
    Node* tmp = (Node*)malloc(sizeof(Node)); // 새로운 노드 생성
    tmp->data = data; 
    tmp->idx = idx; 
    tmp->next = NULL; 
    if (queue->back == NULL) 
    {
        queue->front = queue->back = tmp; 
    } 
    else 
    {
        queue->back->next = tmp; 
        queue->back = tmp; 
    }
    queue->size++; // 큐의 크기 증가
}

// dequeue
void dequeue(Queue* queue, int idx) 
{
    Node* tmp = queue->front;
    Node* prev = NULL;
    // 해당 인덱스를 가진 노드를 찾을 때까지 순회
    while (tmp != NULL && tmp->idx != idx) 
    {
        prev = tmp;
        tmp = tmp->next;
    }
    if (tmp == NULL) return; // 해당 인덱스를 가진 노드가 없을 경우 함수 종료
    
    if (prev == NULL) 
        queue->front = tmp->next; 
    else 
        prev->next = tmp->next; 
    
    if (tmp == queue->back) 
        queue->back = prev; 
    
    free(tmp); // 제거할 노드의 메모리 해제
    queue->size--; // 큐의 크기 감소
}

// 큐에 특정 데이터가 존재하는지 확인하는 함수
bool isExist(Queue* queue, int data) 
{
    Node* tmp = queue->front;
    while (tmp != NULL) 
    {
        if (tmp->data == data) return true; // 데이터 존재 : true 반환
        tmp = tmp->next;
    }
    return false; // 데이터가 존재 x : false 반환
}

// OPT 알고리즘에서 교체할 페이지를 찾는 함수
int opt(Queue* queue, int* input, int curT, int time)
{
    int maxDistance = -1; // 가장 멀리 있는 거리를 초기화
    int replaceIdx = -1;  // 교체 인덱스 초기화
    Node* tmp = queue->front;
    // 큐를 순회하며 교체할 페이지 탐색
    while (tmp != NULL) 
    {
        int distance = 0;
        // 현재 시간 이후 가장 오랫동안 사용되지 않을 페이지 탐색
        for (int j = curT + 1; j < time; j++) 
        {
            if (input[j] == tmp->data) break;
            distance++;
        }
        if (distance > maxDistance) 
        {
            maxDistance = distance;
            replaceIdx = tmp->idx;
        }
        tmp = tmp->next;
    }
    return replaceIdx; // 교체할 인덱스 반환
}

// 큐의 상태를 저장하는 함수
void QueueState(Queue* queue, int frameSize, int time, int result[][20]) 
{
    Node* tmp = queue->front;
    int i = 0;
    // 큐의 현재 상태를 결과 배열에 저장
    while (tmp != NULL) 
    {
        result[tmp->idx][time] = tmp->data;
        tmp = tmp->next;
        i++;
    }
    // 남은 프레임을 0으로 채우기
    for (int j = queue->size; j < frameSize; j++) 
        result[j][time] = 0;
}

int main() 
{
    int input[20] = {1, 2, 3, 2, 1, 5, 2, 1, 6, 2, 5, 6, 3, 1, 3, 6, 1, 2, 4, 3}; // 입력 참조 문자열
    int time = 20; // 전체 시간
    int result[3][20] = {0}; // 프레임 상태 저장
    int pageFaultIdx[20]; // 페이지 부재가 발생한 인덱스를 저장할 배열
    int pageFaultCnt = 0; // 페이지 부재 카운터
    int curIdx = 0; // 현재 페이지 인덱스

    Queue* queue = initQueue(FRAME_SIZE); // 큐 초기화

    // 각 시간 단계에서 페이지 교체를 수행
    for (int i = 0; i < time; i++) 
    {
        if (!isExist(queue, input[i])) 
        {
            if (queue->size == FRAME_SIZE)
            {
                // OPT 알고리즘으로 교체할 페이지 탐색
                int replaceIdx = opt(queue, input, i, time); 
                dequeue(queue, replaceIdx); // 교체할 페이지 제거
                curIdx = replaceIdx;
            }
            enqueue(queue, input[i], curIdx); 
            curIdx = (curIdx + 1) % FRAME_SIZE;
            pageFaultIdx[pageFaultCnt++] = i; // 페이지 부재 인덱스 저장
        }
        QueueState(queue, FRAME_SIZE, i, result); // 현재 큐 상태 저장
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