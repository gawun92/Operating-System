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
#include <signal.h>
#include "SortedList.h"


static struct option longopt[] = {
    {"threads",    required_argument, NULL, 't'},
    {"iterations", required_argument, NULL, 'i'},
    {"yield",      required_argument, NULL, 'y'},
    {"sync",       required_argument, NULL, 's'},
    {0,            0,                 NULL,  0}
};
    
//Global variables
char lockType = 'n';
long long num_Itr = 1;
int num_Thread = 1;

pthread_mutex_t mutex;
pthread_t *pThreads;
int *pthreadID;
int spinLock = 0;

SortedListElement_t *element;
SortedList_t *list;
long Total_run;
int opt_yield=0;
int status;
char str[50] = "list-";
int i;



void handle_error(int status, char *errorstr)
{
    if(status < 0)
    {
        fprintf(stderr, "Error %s: %s \n", errorstr, strerror(errno));
        exit(1);
    }
}

void corrupted_error(char *func)
{
    fprintf(stderr, "Error in the function(%s) : the list has been corrupted.\n", func);
    exit(2);
}

void print_direction()
{
    fprintf(stderr, "Recognized the wrong argument. Proper usuage:\n");  
    fprintf(stderr, "--threads=The_number_of_Threads\n");  
    fprintf(stderr, "--iterations=The_number_of_iteration\n");
    fprintf(stderr, "--yield = {none, i,d,l,id,il,dl,idl}\n");
    fprintf(stderr, "--sync={none,s,m}\n");       
    exit(1);
}


void generateRandomKeys()
{
    srand(time(NULL));
    for (i = 0; i < Total_run; i++)
    {
        char *rand_Key = malloc(2 * sizeof(char));
        rand_Key[0] = (rand() % 26) + 'a';
        rand_Key[1] = '\0';
        element[i].key = rand_Key;
    }
}



void *option_mutex(void *T_ID)
{
    int i = *(int *)T_ID;
    while(i < Total_run)
    {
        pthread_mutex_lock(&mutex);
        SortedList_insert(list, &element[i]);
        pthread_mutex_unlock(&mutex);
        i = i + num_Thread;
    }

    pthread_mutex_lock(&mutex);
    int length = SortedList_length(list);
    pthread_mutex_unlock(&mutex);

    if (length == -1)
        corrupted_error("SortedList_length");

    i = *(int *)T_ID; 
    while(i < Total_run)
    {
        pthread_mutex_lock(&mutex);
        SortedListElement_t *currentElement = SortedList_lookup(list, element[i].key);
        if (currentElement == NULL)
            corrupted_error("SortedList_lookup");
        if (SortedList_delete(currentElement) != 0)
            corrupted_error("SortList_delete");
        pthread_mutex_unlock(&mutex);
        i = i + num_Thread;
    }
    return NULL;
}


void *option_spinLock(void *T_ID)
{
    int i = *(int *)T_ID;
    while(i < Total_run)
    {
        while (__sync_lock_test_and_set(&spinLock, 1) == 1);
        SortedList_insert(list, &element[i]);
        __sync_lock_release(&spinLock);
        i = i + num_Thread;
    }

    while (__sync_lock_test_and_set(&spinLock, 1) == 1); 
    int length = SortedList_length(list);
    __sync_lock_release(&spinLock);
    if (length == -1)
        corrupted_error("SortedList_length");

    i = *(int *)T_ID;
    while( i < Total_run)
    {
        while (__sync_lock_test_and_set(&spinLock, 1) == 1); 
        SortedListElement_t *currentElement = SortedList_lookup(list, element[i].key);
        if (currentElement == NULL)
            corrupted_error("SortedList_lookup");
        if (SortedList_delete(currentElement) != 0)
            corrupted_error("SortList_delete");
        __sync_lock_release(&spinLock); 
        i = i + num_Thread;
    }
    return NULL;
}

//Regular list operations
void *option_reg(void *T_ID)
{
    int i = *(int *)T_ID;
    while(i < Total_run)
    {
        SortedList_insert(list, &element[i]);
        i = i + num_Thread;
    }
    if (SortedList_length(list) == -1)
        corrupted_error("SortedList_length");

    i = *(int *)T_ID;
    while( i < Total_run)
    {
        SortedListElement_t *currentElement = SortedList_lookup(list, element[i].key);
        if (currentElement == NULL)
            corrupted_error("SortedList_lookup");

        if (SortedList_delete(currentElement) != 0)
            corrupted_error("SortList_delete");
        i = i + num_Thread;
    } 

    return NULL;
}

void signalHandler(int sigNum)
{
    if (sigNum == SIGSEGV)
        corrupted_error("Segmentation fault during a list operation");
}



void get_yield_print(){
    
    if(opt_yield == 0)
        strcat(str, "none");
    else if((opt_yield & INSERT_YIELD) && (opt_yield & DELETE_YIELD) && (opt_yield & LOOKUP_YIELD))
        strcat(str, "idl");
    else if((opt_yield & INSERT_YIELD) && (opt_yield & DELETE_YIELD))
        strcat(str, "id");
    else if((opt_yield & INSERT_YIELD) && (opt_yield & LOOKUP_YIELD))
        strcat(str, "il");
    else if((opt_yield & DELETE_YIELD) && (opt_yield & LOOKUP_YIELD))
        strcat(str, "dl");
    else if(opt_yield & INSERT_YIELD)
        strcat(str, "i");
    else if (opt_yield & DELETE_YIELD)
        strcat(str, "d");
    else
        strcat(str, "l");

    if(lockType == 'n')
        strcat(str, "-none");
    else if(lockType == 's')
        strcat(str, "-s");
    else
        strcat(str, "-m");

}


void create_join_Thread()
{
    for (i = 0; i < num_Thread; ++i)
    {
        if(lockType == 's')
        {
            status = pthread_create(&pThreads[i], NULL, &option_spinLock, &pthreadID[i]);
            handle_error(status, "creating pthread");
        }
        else if(lockType == 'm')
        {
            status = pthread_create(&pThreads[i], NULL, &option_mutex, &pthreadID[i]);
            handle_error(status, "creating pthread");
        }
        else
        {
            status = pthread_create(&pThreads[i], NULL, &option_reg, &pthreadID[i]);
            handle_error(status, "creating pthread");
        }
    }

    //Join threads
    for (i = 0; i < num_Thread; ++i)
    {
        status = pthread_join(pThreads[i], NULL);
        handle_error(status, "joining pthread");
    }
}


void initialize_all()
{    
    pThreads = malloc(num_Thread * sizeof(pthread_t));
    pthreadID = malloc(num_Thread * sizeof(int));
    for (i = 0; i < num_Thread; i++)
        pthreadID[i] = i;

    //initialize list and make links
    list = malloc(sizeof(SortedList_t));
    list->next = list;
    list->prev = list;
    list->key = NULL;

    //Initialize elements
    element = malloc(Total_run * sizeof(SortedListElement_t));
    generateRandomKeys();
    //Initialize Mutex
    status = pthread_mutex_init(&mutex, NULL);
    handle_error(status, "initialize mutax");
}


int main(int argc, char **argv)
{
    int option;
    int option_index = 0;

    while ((option = getopt_long(argc, argv, "t:i:s", longopt, &option_index)) != -1)
    {
        //Identify which options were specified
        switch (option)
        {
        case 't':
            num_Thread = atoi(optarg);
            break;
        case 'i':
            num_Itr = atoi(optarg);
            break;
        case 'y':
            if (strlen(optarg) > 3)
                print_direction();
            else
            {
                for (i = 0; optarg[i] != '\0'; ++i)
                {
                    if (optarg[i] == 'i')
                        opt_yield |= INSERT_YIELD;
                    else if (optarg[i] == 'd')
                        opt_yield |= DELETE_YIELD;
                    else if (optarg[i] == 'l')
                        opt_yield |= LOOKUP_YIELD;
                    else
                        print_direction();
                }
            }
            break;
        case 's':
            if (strlen(optarg) != 1)
                print_direction();
            lockType = optarg[0];
            break;
        default:
            print_direction();
        }
    }
    Total_run = num_Itr * num_Thread;
    if (signal(SIGSEGV, signalHandler) == SIG_ERR){
        fprintf(stderr, "ERROR : %s\n", strerror(errno));
        exit(1);
    }
    initialize_all();

    //Initialize timer
    struct timespec start_Time;
    status = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_Time);
    handle_error(status, "getting start time");

    create_join_Thread();

    //Destroy mutex
    status = pthread_mutex_destroy(&mutex);
    handle_error(status, "destroying mutex");
        
    if (SortedList_length(list) < 0)
        printf("%d\n", SortedList_length(list));

    struct timespec end_Time;
    status = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_Time);
    handle_error(status, "getting end time");

    
    long long run_Time = (end_Time.tv_sec - start_Time.tv_sec) * 1000000000 +
                        (end_Time.tv_nsec - start_Time.tv_nsec);

    get_yield_print();

    free(element);
    free(pThreads);
    free(list); 
    free(pthreadID);

    //Final calculations
    long long num_Op = num_Itr * num_Thread * 3;
    long long Average = run_Time / num_Op;

    printf("%s,%d,%lld,%d,%lld,%lld,%lld\n", 
        str, num_Thread, num_Itr, 1, num_Op, run_Time, Average);

    exit(0);
    return 1;
}



