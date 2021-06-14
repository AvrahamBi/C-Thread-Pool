#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <sys/types.h>
#include "osqueue.h"
#include <stdlib.h>
#include <stdio.h>
#include "osqueue.h"
#include <pthread.h>
#include <unistd.h>

typedef struct thread_pool
{
 int numOfRunningTasks;
 int numOfThreads;
 int isRunning;
 int shouldWaitForTasks;
 int hasRunningTasks;
 OSQueue *tasksQueue;
 pthread_t *threads;
 pthread_mutex_t mutex;
 pthread_cond_t cond;

}ThreadPool;

typedef struct {
    void (*function) (void *);
    void *param;
}Task;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
