#include <stdio.h>

#define F_SIZE 3
#define MAX 20

// 프레임 구조체 정의 
typedef struct 
{
    int page;    // 페이지 번호 
    int use_bit; // 사용 비트 
} Frame;

// 프레임을 출력하는 함수
void printFrame(int matrix[][MAX]) 
{
    for (int i = 0; i < 3; i++) 
    {
        for (int j = 0; j < 20; j++) 
            printf("%d ", matrix[i][j] ? matrix[i][j] : 0); // 페이지 번호 출력, 없으면 0 출력 
        printf("\n");
    }
}
// 교체될 프레임을 찾는 함수
int findReplace(Frame frames[], int *ptr) 
{
    int iter = 0; 
    while (iter < 3) // 최대 size번 반복
    {
        if (frames[*ptr].use_bit == 0) // 사용 비트가 0인 프레임을 찾으면 
        {
            int replaceFrame = *ptr;
            *ptr = (*ptr + 1) % 3; // 포인터를 다음 위치로 이동 
            return replaceFrame; // 교체될 프레임 반환
        }
        frames[*ptr].use_bit = 0; // 사용 비트를 0으로 리셋
        *ptr = (*ptr + 1) % 3; // 포인터를 다음 위치로 이동
        iter++; // 반복 횟수 증가
    }
    return *ptr; 
}


int main() 
{
    int input[MAX] = {1, 2, 3, 2, 1, 5, 2, 1, 6, 2, 5, 6, 3, 1, 3, 6, 1, 2, 4, 3};
    Frame frames[F_SIZE] = {{0, 0}}; // 프레임 초기화
    int frameMatrix[F_SIZE][MAX] = {0};              // 프레임 배열
    int pageFaultIdx[MAX] = {0}; // 페이지 부재 발생 인덱스 배열
    int pageFaultCnt = 0;
    int ptr = 0; // 클럭 포인터 시작

    for (int i = 0; i < MAX; i++) 
    {
        int page = input[i];
        int found = 0; // 페이지가 이미 프레임에 있는지 체크
        int hitIndex = -1; // 히트된 프레임의 인덱스

        for (int j = 0; j < F_SIZE; j++) 
        {
            if (frames[j].page == page) 
            {
                hitIndex = j; // 히트된 프레임 인덱스 저장
                found = 1; // 페이지 히트 시 사용 비트 설정 
                break;
            }
        }

        if (found) 
        { // 페이지가 프레임에 있는 경우 
            // 히트된 페이지의 사용 비트를 1로 설정하고, 나머지는 0으로 설정
            for (int k = 0; k < F_SIZE; k++) 
                frames[k].use_bit = (k == hitIndex) ? 1 : 0;
        } 
        else 
        { // 페이지 부재 발생 시 
            int replaceFrame = findReplace(frames, &ptr); // 교체할 프레임 탐색
            frames[replaceFrame].page = page; // 새 페이지를 프레임에 삽입 
            frames[replaceFrame].use_bit = 1; // 새로운 페이지 삽입시 use_bit을 1로 설정
            pageFaultIdx[pageFaultCnt++] = i; // 페이지 부재 인덱스 기록 
        }

        // 현재 프레임 상태 기록
        for (int j = 0; j < F_SIZE; j++) 
            frameMatrix[j][i] = frames[j].page;
    }

    // 프레임 상태 출력 
    printf("Frame States:\n");
    printFrame(frameMatrix);

    // 페이지 부재 인덱스 출력 
    printf("Page Fault Idx:\n");
    for (int i = 0; i < pageFaultCnt; i++) 
        printf("%d ", pageFaultIdx[i]);
        
    printf("\n");
    return 0;
}