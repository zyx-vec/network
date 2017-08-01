#ifndef __THREAD_POOL
#define __THREAD_POOL

struct job_t {
    void* args;             // parameter
    void* (*fun)(void*);   // function pointer
};

void init_thread_pool(int num_of_thread);

#endif

