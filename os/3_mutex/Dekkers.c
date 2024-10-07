#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

#define true 1
#define false 0

int flag[2] = {false, false};
int turn = 0;
int i;

void ft0(void){
  flag[0] = true;
  while(flag[1]){
    if(turn==1){
      flag[0]= false;
      while(turn==1){
        // 바쁜 대기
      }
      flag[0]=true;
    }
  }
  printf("thread0: ");
  for(i=1;i<=50;i++) printf("%d ", i*3);
  printf("\n");
  // 임계 영역
  turn = 1;
  flag[0] = false;
}

void ft1(void){
  flag[1] = true;
  while(flag[0]){
    if(turn==0){
      flag[1]= false;
      while(turn==0){
        // 바쁜 대기
      }
      flag[1] = true;
    }
  }
  printf("thread1: ");
  for(i=51;i<=100;i++) printf("%d ", i*3);
  // 임계 영역 
  turn = 0;
  flag[1]=false;
}

int main(void)
{
  pthread_t t0, t1;
  
  // 스레드 생성
  if (pthread_create(&t0, NULL, (void*)ft0, NULL) != 0){
      fprintf(stderr, "thread create error\n"); exit(1);
  }

  if (pthread_create(&t1, NULL, (void*)ft1, NULL) != 0){
      fprintf(stderr, "thread create error\n"); exit(1);
  }
  //테스트용 
  // printf("%ld & %ld create \n", t0,t1);
  pthread_join(t0,NULL);
  pthread_join(t1,NULL);
  printf("\n");
  
  return 0;
}