#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

sem_t semaphore;
int i;

void ft0(void)
{
    sem_wait(&semaphore);

    // 임계 영역 진입
    printf("thread0: ");
    for(i=1;i<=25;i++) printf("%d ", i*3);
    printf("\n");
    sem_post(&semaphore);
}

void ft1(void)
{
    sem_wait(&semaphore);

    // 임계 영역 진입
    printf("thread1: ");
    for(i=26;i<=50;i++) printf("%d ", i*3);
    printf("\n");
    sem_post(&semaphore);
}

void ft2(void)
{
    sem_wait(&semaphore);

    // 임계 영역 진입
    printf("thread2: ");
    for(i=51;i<=75;i++) printf("%d ", i*3);
    printf("\n");
    sem_post(&semaphore);
}

void ft3(void)
{
    sem_wait(&semaphore);

    // 임계 영역 진입
    printf("thread3: ");
    for(i=76;i<=100;i++) printf("%d ", i*3);
    printf("\n");
    sem_post(&semaphore);
}

int main()
{
    pthread_t t0,t1,t2,t3;
    
    sem_init(&semaphore, 0, 1);

    // printf("semaphore start! \n");
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

    // 컷 
    // printf("모든 스레드 할 일 다했다!");
    printf("\n");

    // 세마포 객체 소멸 
    sem_destroy(&semaphore);
    
    return 0;
}