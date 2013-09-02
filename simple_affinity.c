/*
 *============================================================================
 *
 *       Filename:  simple_affinity.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年09月02日 00时02分42秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Oen Han (), 
 *   Organization:  
 *
 * ============================================================================
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <sched.h>
#include <pthread.h>

#include <sys/syscall.h>

#define gettid() syscall(__NR_gettid) 

void *test_thread(void *arg)
{
    cpu_set_t mask;
    int loop = 0;
    int cpu_num = 0;

    cpu_num = sysconf(_SC_NPROCESSORS_CONF);

    pthread_detach(pthread_self());

    CPU_ZERO(&mask);
    CPU_SET(1, &mask);

    if(sched_setaffinity(0, sizeof(mask), &mask) == -1)
    {
        printf("set affinity failed\n");
    }

    while(1)
    {
        CPU_ZERO(&mask);

        if(sched_getaffinity(0, sizeof(mask), &mask) == -1)
        {
            printf("get failed\n");
        }

        for(loop = 0; loop < cpu_num; loop++)
        {
            if(CPU_ISSET(loop, &mask))
            {
                printf("test thread %lu run on processor %d\n",
                        gettid(), loop);
            }
        }

        sleep(1);
    }

}

void *child_thread(void *arg)
{
    cpu_set_t mask;
    int loop = 0;
    int cpu_num = 0;

    cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    pthread_detach(pthread_self());

    while(1)
    {
        CPU_ZERO(&mask);
        
        if(sched_getaffinity(0, sizeof(mask), &mask) == -1)
        {
            printf("get failed\n");
        }

        for(loop = 0; loop < cpu_num; loop++)
        {
            if(CPU_ISSET(loop, &mask))
            {
                printf("child thread %lu run on processor %d\n",
                        gettid(), loop);
            }
        }

        sleep(1);

    }
}

int main(int argc, char *argv[])
{
    int cpu_num = 0;
    pthread_t thread;
    int cpuid = 0;
    int ret = 0;
    int loop = 0;


    cpu_set_t mask_set;
    cpu_set_t mask_get;

    if(argc != 2)
    {
        printf("usage:cpu num\n");
        return -1;
    }
    
    cpuid = atoi(argv[1]);
    

    /* 获取系统CPU的个数 */
    cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    printf("system has %i processor.\n", cpu_num);

    /* 初始化mask_set */
    CPU_ZERO(&mask_set);
    CPU_SET(cpuid, &mask_set);

    if(sched_setaffinity(0, sizeof(mask_set), &mask_set) == -1)
    {
        printf("Warning:set cpu %d affinity failed\n", cpuid);
    }

    ret = pthread_create(&thread, NULL, child_thread, NULL);
    if(ret)
    {
        printf("Error:pthread_create failed\n");
        return -1;
    }

    ret = pthread_create(&thread, NULL, test_thread, NULL);
    if(ret)
    {
        printf("Error:pthread_create failed\n");
        return -1;
    }

    while(1)
    {
        CPU_ZERO(&mask_get);
        if(sched_getaffinity(0, sizeof(mask_get), &mask_get) == -1)
        {
            printf("Warning:get cpu %d affinity failed\n", cpuid);
        }

        for(loop = 0; loop < cpu_num; loop++)
        {
            if(CPU_ISSET(loop, &mask_get))
            {
                printf("this processor %lu is running on processor: \
                        %d\n", gettid(), loop);
            }
        }

        sleep(1);
    }

    return 0;
}

