#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define false 0
#define true 1
#define want_in 2
#define in_CS 3

int flag[4] = {false, false, false, false};
int turn = 0;
int cnt;
int i;

pthread_mutex_t mutex;

void ft0(void)
{
    do
    {
        // 임계 영역 진입 1단계
        flag[0] = want_in;
        while (turn != 0)  // 자기 차례 아니면 대기
        {
            if (flag[turn] == false) // 임계 영역에 다른 스레드 없는 경우
                turn = 0; // 자기 차례
        }

        pthread_mutex_lock(&mutex); // 뮤텍스 락

        // 임계 영역 진입  2단계
        flag[0] = in_CS; // 임계 영역에 진입
        cnt = 0;
        while ((cnt < 4) && (cnt == 0 || flag[cnt] != in_CS)) // 다른 스레드가 임계 영역에 있는 동안 대기
        {
            cnt += 1;
        }
    } while (cnt < 4);

    // 임계 영역
    printf("thread0: ");
    for (i = 1; i <= 25; i++)
        printf("%d ", i * 3);
    printf("\n");
    flag[0] = false;
    pthread_mutex_unlock(&mutex); // 뮤텍스 락 해제
}

void ft1(void)
{
    do
    {
        // 임계 영역 진입  1단계
        flag[1] = want_in;
        while (turn != 1) // 자기 차례 아니면 대기 
        {
            if (flag[turn] == false) // 임계 영역에 다른 스레드 없는 경우
                turn = 1;
        }

        pthread_mutex_lock(&mutex); // 뮤텍스 락

        // 임계 영역 진입  2단계
        flag[1] = in_CS; // 임계 영역에 진입
        cnt = 0;
        while ((cnt < 4) && (cnt == 1 || flag[cnt] != in_CS)) // 다른 스레드가 임계 영역에 있는 동안 대기
        {
            cnt += 1;
        }
    } while (cnt < 4);

    // 임계 영역
    printf("thread1: ");
    for (i = 26; i <= 50; i++)
        printf("%d ", i * 3);
    printf("\n");
    flag[1] = false;
    pthread_mutex_unlock(&mutex); // 뮤텍스 락 해제
}

void ft2(void)
{
    do
    {
        // 임계 영역 진입  1단계
        flag[2] = want_in;
        while (turn != 2) // 자기 차례 아니면 대기
        {
            if (flag[turn] == false) // 임계 영역에 다른 스레드가 없는 경우
                turn = 2; // 자기 차례
        }

        pthread_mutex_lock(&mutex); // 뮤텍스 락

        // 임계 영역 진입  2단계
        flag[2] = in_CS; // 임계 영역에 진입
        cnt = 0;
        while ((cnt < 4) && (cnt == 2 || flag[cnt] != in_CS)) // 다른 스레드가 임계 영역에 있는 동안 대기
        {
            cnt += 1;
        }
    } while (cnt < 4);

    // 임계 영역
    printf("thread2: ");
    for (i = 51; i <= 75; i++)
        printf("%d ", i * 3);
    printf("\n");
    flag[2] = false;
    pthread_mutex_unlock(&mutex); // 뮤텍스 락 해제
}

void ft3(void)
{
    do
    {
        // 임계 영역 진입  1단계
        flag[3] = want_in;
        while (turn != 3) // 자기 차례 아니면 대기
        {
            if (flag[turn] == false) // 임계 영역에 다른 스레드가 없는 경우
                turn = 3; // 자기 차례
        }

        pthread_mutex_lock(&mutex); // 뮤텍스 락
        
        // 임계 영역 진입  2단계
        flag[3] = in_CS; // 임계 영역에 진입
        cnt = 0;
        while ((cnt < 4) && (cnt == 3 || flag[cnt] != in_CS)) // 다른 스레드가 임계 영역에 있는 동안 대기
        {
            cnt += 1;
        }
    } while (cnt < 4);

    // 임계 영역
    printf("thread3: ");
    for (i = 76; i <= 100; i++)
        printf("%d ", i * 3);
    printf("\n");
    flag[3] = false;
    pthread_mutex_unlock(&mutex); // 뮤텍스 락 해제
}

int main(void)
{
    pthread_t t0, t1, t2, t3;

    // 스레드 생성
    if (pthread_create(&t0, NULL, (void *)ft0, NULL) != 0)
    {
        fprintf(stderr, "Thread create error\n");
        exit(1);
    }

    if (pthread_create(&t1, NULL, (void *)ft1, NULL) != 0)
    {
        fprintf(stderr, "Thread create error\n");
        exit(1);
    }

    if (pthread_create(&t2, NULL, (void *)ft2, NULL) != 0)
    {
        fprintf(stderr, "Thread create error\n");
        exit(1);
    }

    if (pthread_create(&t3, NULL, (void *)ft3, NULL) != 0)
    {
        fprintf(stderr, "Thread create error\n");
        exit(1);
    }

    pthread_join(t0, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    printf("\n");

    return 0;
}
