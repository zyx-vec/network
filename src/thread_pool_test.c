#include <stdio.h>
#include <stdlib.h>
#include "thread_pool.h"

extern pthread_t* threads;

void* work(void* args) {
    int sum = 0;
    int i;
    for (i = 0; i < 10000; i++) {
        sum += i;
    }
    printf("sum from 0 to 9999 is: %d\n", sum);

    return NULL;
}

int main() {
    int num_of_thread = 10;
    int num_of_work = 50;
    init_thread_pool(num_of_thread);
    
    int i;
    for (i = 0; i < num_of_work; i++) {
        struct job_t* job = (struct job_t*)malloc(sizeof(struct job_t));
        job->args = NULL;
        job->fun = &work;
        add_job(job);
    }

    for (i = 0; i < num_of_thread; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

