#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "thread_pool.h"

static pthread_mutex_t in;
static pthread_mutex_t out;
static sem_t thread_sem;
static sem_t slot_sem;

struct job_queue;
pthread_t* threads;
static struct job_queue queue;
static int num_of_thread;


struct job_queue {
    int l, r;
    struct job_t** jobs;
};

static void* worker_thread(void* args) {
    pthread_t tid = pthread_self();
    unsigned char* p = (unsigned char*)(void*)(&tid);
    size_t i;

    // pthread_mutex_lock(&mutex);
    // fprintf(stdout, "0x");
    // for (i = 0; i < sizeof(tid); i++) {
    //     fprintf(stdout, "%02x", (unsigned)(p[i]));
    // }
    // fprintf(stdout, "\n");
    // pthread_mutex_unlock(&mutex);

    struct job_t* job;
    int l;
    for (;;) {
        sem_wait(&thread_sem);
        pthread_mutex_lock(&out);
        l = queue.l;
        job = queue.jobs[l];
        if (++l == num_of_thread)
            l = 0;
        queue.l = l;
        pthread_mutex_unlock(&out);
        sem_post(&slot_sem);
        
        (job->fun)(job->args);
        free(job);
    }

    return NULL;
}

void add_job(struct job_t* job) {
    sem_wait(&slot_sem);
    pthread_mutex_lock(&in);
    int r = queue.r;
    queue.jobs[r] = job;
    if (++r == num_of_thread)
        r = 0;
    queue.r = r;
    pthread_mutex_unlock(&in);
    sem_post(&thread_sem);
}

void init_thread_pool(int num) {
    int i, n = num;
    if (n < 1) return;

    pthread_mutex_init(&in, NULL);
    pthread_mutex_init(&out, NULL);
    num_of_thread = num;
    sem_init(&thread_sem, 0, 0);
    sem_init(&slot_sem, 0, n);

    queue.l = 0;
    queue.r = 0;
    queue.jobs = (struct job_t**) malloc(n*sizeof(void*));
    threads = (pthread_t*) malloc(n*sizeof(pthread_t));
    for (i = 0; i < num_of_thread; i++) {
        pthread_create(&threads[i], NULL, worker_thread, NULL);
    }

    return;
}

