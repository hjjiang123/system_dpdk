#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void testTimestampSpeed() {
    struct timeval start, end;
    long long int total_time = 0;
    int num_iterations = 100000000;

    for (int i = 0; i < num_iterations; i++) {
        gettimeofday(&start, NULL);
        // Perform some operation here
        gettimeofday(&end, NULL);

        long long int elapsed_time = (end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec);
        total_time += elapsed_time;
    }

    double average_time = (double)total_time / num_iterations;
    printf("Average time to read system timestamp: %.2f microseconds\n", average_time);
    printf("Total time: %lld microseconds\n", total_time);
}

int main() {
    testTimestampSpeed();
    return 0;
}

