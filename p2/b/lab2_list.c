// NAME: Gawun Kim
// EMAIL: gawun92@g.ucla.edu
// ID: 305186572


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <getopt.h> 
#include <signal.h>
#include <errno.h>
#include "SortedList.h"


static struct option long_options[] = {
    {"threads",    required_argument, NULL, 't'},
    {"iterations", required_argument, NULL, 'i'},
    {"yield",      required_argument, NULL, 'y'},
    {"sync",       required_argument, NULL, 's'},
    {"lists",      required_argument, NULL, 'l'},
    {0,            0,                 NULL,  0 }
};


//<---- global variables ---->//
int opt_yield  = 0;
int num_List   = 1;
int num_Thread = 1;
char lockType  = 'n';
long long num_Itr = 1;
long long num_Run = 0;
char str[50] = "list-";


//<---- allocation variable ---->//
int                 *spin_Lock;
int                 *pthread_ID;
pthread_t           *pthread_Arr;
pthread_mutex_t     *mutex_Arr;
SortedListElement_t *element_Arr;
SortedList_t        *hash_Table;
long                *hash_Element;
struct timespec     *total_Time_Lock;




void corrupted_error(char *message){
    fprintf(stderr, "Error in the function(%s) : the list has been corrupted\n", message);
    exit(2);
}

void usage_error(char *type){
    fprintf(stderr, "    Unrecognized argument : %s\n ", type);
    fprintf(stderr, "    proper usage :\n");
    fprintf(stderr, "--threads=The_number_of_Threads\n");  
    fprintf(stderr, "--iterations=The_number_of_iteration\n");
    fprintf(stderr, "--yield = {none, i,d,l,id,il,dl,idl}\n");
    fprintf(stderr, "--sync={none,s,m}\n");       
    exit(1);
}


unsigned long hash(const char *str){
  return str[0] % num_List;
}

void generateRandomKeys(SortedListElement_t *element_Arr){
    srand(time(NULL));
    int i;
     for( i = 0; i < num_Run; ++i){
        char *rand_Key = malloc(2*sizeof(char));
        rand_Key[0] = (rand() % 26) + 'a';
        rand_Key[1] = '\0';
        element_Arr[i].key = rand_Key;
        hash_Element[i] = hash(rand_Key);
    }
}

void signalHandler(int sigNum){
    if (sigNum == SIGSEGV)
        corrupted_error("signal handling");
}



void time_Lock_Spin(int *spinLock, void *threadID){
    struct timespec start_T, end_T; 
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &start_T) < 0){
        fprintf(stderr, "ERROR(getting start time) : %s\n", strerror(errno));
        exit(1);
    }
    while (__sync_lock_test_and_set(spinLock, 1) == 1); // spin lock
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &end_T) < 0){
        fprintf(stderr, "ERROR(getting end time) : %s \n", strerror(errno));
        exit(1);
    }
    total_Time_Lock[*(int *)threadID].tv_sec += end_T.tv_sec - start_T.tv_sec;
    total_Time_Lock[*(int *)threadID].tv_nsec += end_T.tv_nsec - start_T.tv_nsec;
}

void time_Lock_Mutex(pthread_mutex_t *mutex,void *threadID){
    struct timespec start_T, end_T; 
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &start_T) < 0){
        fprintf(stderr, "ERROR(getting start time) : %s\n", strerror(errno));
        exit(1);
    }
    pthread_mutex_lock(mutex);
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &end_T) < 0){
        fprintf(stderr, "ERROR(getting end time) : %s \n", strerror(errno));
        exit(1);
    }
    total_Time_Lock[*(int *)threadID].tv_sec += end_T.tv_sec - start_T.tv_sec;
    total_Time_Lock[*(int *)threadID].tv_nsec += end_T.tv_nsec - start_T.tv_nsec;

}


void *listOpsRegular(void *threadID){
    int i,length = 0;
    SortedListElement_t *temp;

    i = *(int *)threadID;
    while (i < num_Run)
    {
        SortedList_insert(&hash_Table[hash_Element[i]], &element_Arr[i]);
        i += num_Thread;
    }
    for (i = 0; i < num_List; i++)
        length += SortedList_length(&hash_Table[i]);
    if (length < 0)
        corrupted_error("length of sorted list");

    i = *(int *)threadID;
    while (i < num_Run){
        temp = SortedList_lookup(&hash_Table[hash_Element[i]], element_Arr[i].key);
        if(temp != NULL){
            if (SortedList_delete(temp) != 0){
                corrupted_error("delete of sorted list");
            }
        }
        else{
            corrupted_error("lookup of sorted list");
        }
        i+= num_Thread;
    }
    return NULL;
}


void *listOpsMutex(void *threadID){
    int i, length = 0;
    SortedListElement_t *temp;

    i = *(int *)threadID;
    while (i < num_Run){
        time_Lock_Mutex(&mutex_Arr[hash(element_Arr[i].key)],threadID);
        SortedList_insert(&hash_Table[hash(element_Arr[i].key)], &element_Arr[i]);
        pthread_mutex_unlock(&mutex_Arr[hash(element_Arr[i].key)]);
        i += num_Thread;
    }
    for (i = 0; i < num_List; ++i){
        time_Lock_Mutex(&mutex_Arr[i],threadID);
        length += SortedList_length(&hash_Table[i]);
        pthread_mutex_unlock(&mutex_Arr[i]);
    }
    if (length < 0)
        corrupted_error("length of sorted list");

    i = *(int *)threadID;
    while (i < num_Run){
        time_Lock_Mutex(&mutex_Arr[hash(element_Arr[i].key)], threadID);
        temp = SortedList_lookup(&hash_Table[hash(element_Arr[i].key)], element_Arr[i].key);
        if(temp != NULL){
            if (SortedList_delete(temp) != 0){
                corrupted_error("delete of sorted list");
            }
        }
        else{
            corrupted_error("lookup of sorted list");
        }
        pthread_mutex_unlock(&mutex_Arr[hash(element_Arr[i].key)]);
        i += num_Thread;
    }
    return NULL;
}


void *listOpsSpinLock(void *threadID){
    int i, length = 0;
    SortedListElement_t *currentElement;


    i = *(int *)threadID;
    while (i < num_Run){
        time_Lock_Spin(&spin_Lock[hash_Element[i]], threadID);
        SortedList_insert(&hash_Table[hash_Element[i]], &element_Arr[i]);
        __sync_lock_release(&spin_Lock[hash_Element[i]]);
        i += num_Thread;
    }

    for (i = 0; i < num_List; ++i){
        time_Lock_Spin(&spin_Lock[i], threadID);
        length += SortedList_length(&hash_Table[i]);
        __sync_lock_release(&spin_Lock[i]);
    }
    if (length < 0)
        corrupted_error("length of sorted list");

    i = *(int *)threadID;
    while (i < num_Run){
        time_Lock_Spin(&spin_Lock[hash_Element[i]], threadID);
        currentElement = SortedList_lookup(&hash_Table[hash_Element[i]], element_Arr[i].key);
        if (currentElement != NULL){
            if (SortedList_delete(currentElement) != 0){
                corrupted_error("delete of sorted list");
            }
        }
        else{
            corrupted_error("lookup or sorted list");
        }
        __sync_lock_release(&spin_Lock[hash_Element[i]]); 
        i += num_Thread;
    }
    return NULL;
}

void initial_setting(){
    int i;
    num_Run = num_Itr * num_Thread;
    //Register signal handler
    if (signal(SIGSEGV, signalHandler) == SIG_ERR){
        fprintf(stderr, "ERROR(SIG_ERR) : %s \n", strerror(errno));
        exit(1);
    }
    //Initialize array of elements
    element_Arr  = malloc(num_Run * sizeof(SortedListElement_t));
    hash_Element = malloc(num_Run * sizeof(unsigned long));
    spin_Lock    = malloc(num_List * sizeof(int));
    pthread_Arr  = malloc(num_Thread * sizeof(pthread_t));
    pthread_ID   = malloc(num_Thread * sizeof(int));
    hash_Table   = calloc(num_List, sizeof(SortedList_t));
    mutex_Arr    = calloc(num_List, sizeof(pthread_mutex_t));

    for (i = 0; i < num_Thread; ++i)
        pthread_ID[i] = i;

    generateRandomKeys(element_Arr);

    for (i = 0; i < num_List; ++i){
        // Initialize the linklist.
        hash_Table[i].prev = hash_Table + i;
        hash_Table[i].next = hash_Table[i].prev; 
        //Initialize the spinLock
        spin_Lock[i] = 0;
        //Initialize the mutex
        if (pthread_mutex_init(&mutex_Arr[i], NULL) != 0){
            fprintf(stderr, "ERROR(Init of mutex) : %s \n", strerror(errno));
            exit(1);
        }
    }

    if (lockType != 'n')
        total_Time_Lock = (struct timespec *)calloc(num_Thread, sizeof(struct timespec));
    else
        total_Time_Lock = NULL;
}

void creating_and_joining_Threads(){
    int i;
    int check;
    for (i = 0; i < num_Thread; ++i){
        if(lockType == 's'){
            check = pthread_create(&pthread_Arr[i], NULL, listOpsSpinLock, &pthread_ID[i]);
        }
        else if(lockType == 'm'){
            check = pthread_create(&pthread_Arr[i], NULL, listOpsMutex, &pthread_ID[i]);
        }
        else{
            check = pthread_create(&pthread_Arr[i], NULL, listOpsRegular, &pthread_ID[i]);
        }

        if(check != 0){
            fprintf(stderr, "ERROR(create thread) : %s \n", strerror(errno));
            exit(1); 
        }
    }
    for (i = 0; i < num_Thread; ++i){
        if (pthread_join(pthread_Arr[i], NULL) != 0){
            fprintf(stderr, "ERROR(join thread) : %s \n", strerror(errno));
            exit(1);
        }
    }

}


void get_yield_print(){    
    if(opt_yield == 0)
        strcat(str, "none");
    else if((opt_yield & INSERT_YIELD) && (opt_yield & DELETE_YIELD) && (opt_yield & LOOKUP_YIELD))
        strcat(str, "idl");
    else if((opt_yield & INSERT_YIELD) && (opt_yield & LOOKUP_YIELD))
        strcat(str, "il");
    else if((opt_yield & INSERT_YIELD) && (opt_yield & DELETE_YIELD))
        strcat(str, "id");
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

void free__memory(){
    free(pthread_Arr);
    free(pthread_ID);
    free(element_Arr);
    free(hash_Table);
    free(spin_Lock);
    free(mutex_Arr);
    free(total_Time_Lock);
}



int main(int argc, char **argv){
    atexit(free__memory);
    int option, i, option_index = 0;
    char *invalid_arg = argv[0];

    while ((option = getopt_long(argc, argv, "t:i:y:s:l:", long_options, &option_index)) != -1)
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
                for (i = 0; i < (int)strlen(optarg); ++i)
                {
                    if (optarg[i] == 'i')
                        opt_yield |= INSERT_YIELD;
                    else if (optarg[i] == 'd')
                        opt_yield |= DELETE_YIELD;
                    else if (optarg[i] == 'l')
                        opt_yield |= LOOKUP_YIELD;
                    else
                        usage_error(invalid_arg);
                }
                break;
            case 's':
                if (strlen(optarg) != 1)
                    usage_error(invalid_arg);
                lockType = optarg[0];
                break;
            case 'l':
                num_List = atoi(optarg);
                break;
            default:
                usage_error(invalid_arg);
        }
    }

    initial_setting();
    
// <-------------------- gettime of start time --------------------> //
    struct timespec start_T, end_T;
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &start_T) < 0){
        fprintf(stderr, "ERROR(start of clock_gettime) : %s \n", strerror(errno));
        exit(1);
    }
    creating_and_joining_Threads();
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &end_T) < 0){
        fprintf(stderr, "ERROR(end of clock_gettime) : %s \n", strerror(errno));
        exit(1);
    }
// <-------------------- gettime of end time --------------------> //




    for (i = 0; i < num_List; ++i){
        if (pthread_mutex_destroy(&mutex_Arr[i]) != 0){
            fprintf(stderr, "ERROR(destroy mutex) : %s\n", strerror(errno));
            exit(1);
        }
    }
    long long time = (end_T.tv_sec - start_T.tv_sec) * 1000000000 + 
                        (end_T.tv_nsec - start_T.tv_nsec);
    if( (end_T.tv_nsec - start_T.tv_nsec) < 0){
        time += 1000000000;
    }

    // calculating timee for output
    long long totalLockTimeNsecs = 0;
    if (lockType != 'n'){
        for (i = 0; i < num_Thread; ++i){
            if (total_Time_Lock[i].tv_nsec < 0)
                totalLockTimeNsecs += 1000000000;
            else
                totalLockTimeNsecs += total_Time_Lock[i].tv_nsec;
            totalLockTimeNsecs += total_Time_Lock[i].tv_sec * 1000000000;
        }
    }

    get_yield_print();

    long long numOfOperations = 3 * num_Run;
    long long timePerOperation = time / numOfOperations;
    long long lockTimePerOperation = totalLockTimeNsecs / numOfOperations;

    printf("%s,%d,%lld,%d,%lld,%lld,%lld,%lld\n", str, num_Thread, num_Itr, 
            num_List, numOfOperations, time, timePerOperation, lockTimePerOperation);
    exit(0);
    return 1;
}
