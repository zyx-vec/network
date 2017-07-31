#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "thread_pool.h"

static pthread_mutex_t mutex;

static pthread_t* threads;

static void* worker_thread(void* args) {
    pthread_t tid = pthread_self();
    unsigned char* p = (unsigned char*)(void*)(&tid);
    size_t i;

    pthread_mutex_lock(&mutex);
    fprintf(stdout, "0x");

    for (i = 0; i < sizeof(tid); i++) {
        fprintf(stdout, "%02x", (unsigned)(p[i]));
    }
    fprintf(stdout, "\n");
    
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void init_thread_pool(int num_of_thread) {
    int i, n = num_of_thread;
    if (n < 1) return;

    pthread_mutex_init(&mutex, NULL);

    threads = (pthread_t*) malloc(n * sizeof(pthread_t));
    for (i = 0; i < num_of_thread; i++) {
        pthread_create(&threads[i], NULL, worker_thread, NULL);
    }

    for (i = 0; i < num_of_thread; i++) {
        pthread_join(threads[i], NULL);
    }

    return;
}

