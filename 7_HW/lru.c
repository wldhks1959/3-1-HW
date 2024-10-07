#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define FRAME_SIZE 3 

// 스택 구조체 정의
typedef struct Stack 
{
    int* data; 
    int top;   // 상단 인덱스
    int capacity; // 스택 용량
} Stack;

// 스택 초기화 함수
Stack* initStack(int capacity) 
{
    Stack* stack = (Stack*)malloc(sizeof(Stack)); 
    stack->data = (int*)malloc(capacity * sizeof(int)); 
    stack->top = -1; // 초기 상단 인덱스 설정
    stack->capacity = capacity; // 스택 용량 설정
    return stack;
}

// push
void push(Stack* stack, int value) 
{
    if (stack->top == stack->capacity - 1) return; // 스택이 가득 찼을 경우
    // 기존 요소들을 아래로 밀기
    for (int i = stack->top; i >= 0; i--) 
        stack->data[i + 1] = stack->data[i];
    stack->data[0] = value; // 새로운 요소를 상단에 추가
    stack->top++; // 상단 인덱스 증가
}

// pop
void pop(Stack* stack, int value) 
{
    int idx = -1;
    // 특정 값의 인덱스 찾기
    for (int i = 0; i <= stack->top; i++) 
    {
        if (stack->data[i] == value) 
        {
            idx = i;
            break;
        }
    }
    if (idx != -1) 
    {
        // 값을 제거하고 나머지 요소들을 위로 이동
        for (int i = idx; i < stack->top; i++) 
            stack->data[i] = stack->data[i + 1];
        stack->top--; // 상단 인덱스 감소
    }
}

// 스택에 특정 값이 존재하는지 확인하는 함수
bool isExist(Stack* stack, int value) 
{
    for (int i = 0; i <= stack->top; i++) 
        if (stack->data[i] == value) return true; // 값이 존재 : true 반환
    return false; // 값이 존재 x : false 반환
}

// 프레임 상태를 저장하는 함수
void StackState(Stack* stack, int frameSize, int time, int result[][20]) 
{
    for (int i = 0; i <= stack->top; i++) 
        result[i][time] = stack->data[i]; // 현재 스택 상태를 결과 배열에 저장
    // 남은 프레임을 0으로 채우기
    for (int i = stack->top + 1; i < frameSize; i++) 
        result[i][time] = 0;
}

int main() 
{
    int input[20] = {1, 2, 3, 2, 1, 5, 2, 1, 6, 2, 5, 6, 3, 1, 3, 6, 1, 2, 4, 3}; // 입력 데이터
    int time = 20;
    int result[FRAME_SIZE][20] = {0}; // 프레임 상태를 저장할 배열
    int pageFaultIdx[20]; // 페이지 부재가 발생한 인덱스를 저장할 배열
    int pageFaultCnt = 0; // 페이지 부재 카운터

    Stack* stack = initStack(FRAME_SIZE); // 스택 초기화

    for (int i = 0; i < time; i++) 
    {
        if (!isExist(stack, input[i])) 
        {
            if (stack->top == FRAME_SIZE - 1) // 스택이 가득 찼을 경우
                stack->top--; // 가장 오래된 요소 제거
            push(stack, input[i]); // 새로운 요소를 스택에 추가
            pageFaultIdx[pageFaultCnt++] = i; // 페이지 부재 인덱스를 저장
        } 

        else 
        {
            // 이미 존재하는 요소를 스택에서 제거하고 다시 추가
            pop(stack, input[i]);
            push(stack, input[i]);
        }
        StackState(stack, FRAME_SIZE, i, result); // 현재 스택 상태를 결과 배열에 저장
    }

    // // 입력 상태 출력
    // printf("Input States:\n");
    // for (int i = 0; i < 20; i++) printf("%d ", input[i]);
    // printf("\n");

    // 각 시간에서의 프레임 상태 출력
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