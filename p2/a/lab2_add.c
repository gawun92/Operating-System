//NAME: Gawun Kim
//EMAIL: gawun@g.ucla.edu
//ID: 305186572

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <getopt.h>
#include <time.h>


static struct option longopt[] = {
    {"threads",    required_argument, NULL, 't'},
    {"iterations", required_argument, NULL, 'i'},
    {"yield",      no_argument,       NULL, 'y'},
    {"sync",       required_argument, NULL, 's'},
    {0,            0,                 NULL,  0}
};

int status;
long long counter = 0;
long long num_Itr = 1;
int num_Thread = 1;
int y_Flag = 0;  // for yield. Initially start from 0.
char lockType = 'n'; // initially set as n
int i;

pthread_mutex_t mutex;
int spinLock = 0;
pthread_t *pThreads;


void print_direction()
{
    fprintf(stderr, "Recognized the wrong argument. Proper usuage:\n");  
    fprintf(stderr, "--threads=The_number_of_Threads\n");  
    fprintf(stderr, "--iterations=The_number_of_iteration\n");
    fprintf(stderr, "--yield\n");
    fprintf(stderr, "--sync={m,s,c}\n");       
    exit(1);
}

//Helper functions handle_error
void handle_error(int status, char *errorstr)
{
    if(status < 0)
    {
        fprintf(stderr, "Error of %s: %s \n", errorstr, strerror(errno));
        exit(1);  
    } 
}

//Given add function (yields after summing if yield option specified)
void add(long long *pointer, long long value)
{
    long long sum = *pointer + value;
    if (y_Flag){
        sched_yield();
    }
    *pointer = sum;
}

void mutex_Lock(long long *pointer, long long value) //Using mutex
{
    //Try to obtain the lock
    status = pthread_mutex_lock(&mutex);
    handle_error(status, "locking mutex on sum");

    add(pointer, value);

    if ((status = pthread_mutex_unlock(&mutex)) != 0)
        fprintf(stderr, "%d\n", status);
}

void spin_Lock(long long *pointer, long long value) //Using custom spin lock
{
    while (__sync_lock_test_and_set(&spinLock, 1) == 1); 
    //spin
    add(pointer, value);
    __sync_lock_release(&spinLock);
}


// perform an atomic compare and swap
void atomic_mem_access(long long *pointer, long long value)  
{
    long long oldSum, newSum;
    do
    {
        oldSum = *pointer;
        newSum = oldSum + value;
        if (y_Flag)
            sched_yield();
    } while (oldSum != __sync_val_compare_and_swap(pointer, oldSum, newSum));
}


void *thread_start()
{
    long long i;
    for(i = 0; i < num_Itr; ++i)
    {
        if(lockType == 's')
            spin_Lock(&counter, 1);
        else if(lockType == 'm')
            mutex_Lock(&counter, 1);
        else if(lockType == 'c')
            atomic_mem_access(&counter, 1);
        else
            add(&counter, 1);
    }
    for (i = 0; i < num_Itr; ++i)
    {
        if(lockType == 's')
            spin_Lock(&counter, -1);
        else if(lockType == 'm')
            mutex_Lock(&counter, -1);
        else if(lockType == 'c')
            atomic_mem_access(&counter, -1);
        else
            add(&counter, -1);
    }
    return NULL;
}


void create_join_Thread()
{
    //creating threads
    for (i = 0; i < num_Thread; ++i)
    {
        status = pthread_create(&pThreads[i], NULL, &thread_start, NULL);
        handle_error(status, "pthread create");
    }
    //Join threads
    for (i = 0; i < num_Thread; ++i)
    {
        status = pthread_join(pThreads[i], NULL);
        handle_error(status, "pthread join");
    }
}

int main(int argc, char **argv)
{
    int option;
    int option_index = 0;

    while ((option = getopt_long(argc, argv, "t:i:s", longopt, &option_index)) != -1)
    {
        switch (option)
        {
        case 't':
            num_Thread = atoi(optarg);
            break;

        case 'i':
            num_Itr = atoi(optarg);
            break;
        case 'y':
            y_Flag = 1;
            break;

        case 's':
            if(strlen(optarg)==1)
            {
                switch(optarg[0])
                {
                    case 's':
                    case 'm':
                    case 'c':
                        lockType = optarg[0];
                        break;
                    default:
                        fprintf(stderr, "ERROR: Unrecognized sync argument.\n");
                        exit(1);
                }
                break;
            }
            else
            {
                print_direction();
            }
            break;
        default:
            print_direction();
        }
    }

    pThreads = malloc(num_Thread * sizeof(pthread_t));

    // initializing the mutax
    status = pthread_mutex_init(&mutex, NULL);
    handle_error(status, "Mutx init");
    
    //initializing the time
    struct timespec start_Time;
    status = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_Time);
    handle_error(status, "getting start time");

    create_join_Thread();

    struct timespec end_Time; // To check the time comsuming
    status = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_Time);
    handle_error(status, "getting end time");

    long long run_Time = (end_Time.tv_sec - start_Time.tv_sec) * 1000000000 +
                        (end_Time.tv_nsec - start_Time.tv_nsec);
    
    char str[30] = "add";
    char temp[2];

    if(y_Flag)
    {
        strcat(str, "-yield");
    }
    if(lockType == 'n')
        strcat(str, "-none");
    else if(lockType == 'm' || lockType == 's' || lockType == 'c')
    {
        strcat(str, "-");
        temp[0] = lockType; temp[1] = '\0';
        strcat(str, temp);
    }
    free(pThreads);
    status = pthread_mutex_destroy(&mutex);
    handle_error(status, "destroying mutex");

    //Print output
    long long num_Op = num_Itr * num_Thread * 2;
    long long Average = run_Time / num_Op;
    printf("%s,%d,%lld,%lld,%lld,%lld,%lld\n", 
        str, num_Thread, num_Itr, num_Op, run_Time, Average, counter);

    exit(0);
    return 1;
}
