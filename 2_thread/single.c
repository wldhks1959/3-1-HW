#include <stdio.h>
#include <time.h>

int main() 
{
    double start_time, end_time, single_time;
    start_time = clock(); // 시간 측정 시작
    
    // 곱셈 후 출력 
    for (int i = 1; i <= 100; i++) printf("%d ", i * 3);
    for (int i = 1; i <= 100; i++) printf("%d ", i * 5);
    for (int i = 1; i <= 100; i++) printf("%d ", i * 7);
    for (int i = 1; i <= 100; i++) printf("%d ", i * 9);
    printf("\n");
    
    end_time = clock(); // 시간 측정 종료
    single_time = (end_time - start_time) / CLOCKS_PER_SEC; // 소요 시간 계산
    printf("single_time : %lf secs\n", single_time);
    
    return 0;
}
