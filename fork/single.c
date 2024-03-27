#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define NUM_ITER 1000

void single(int start, int end) 
{
    for (int i = start; i <= end; i++) 
        printf("%d\n", i * 7);
}

int main() {
    double start_time, end_time, single_time;

    start_time = clock();

    single(1, NUM_ITER);
    
    end_time = clock();
    single_time = (end_time - start_time) / CLOCKS_PER_SEC;

    printf("Single process time: %lf\n", single_time);

    return 0;
}