#define _POSIX_C_SOURCE 199309L //required for clock

#include <stdio.h>   // General requirement for I/O function
#include <sys/shm.h> // For shared memory used function : shmget,shmat
#include <pthread.h> // For using thread functions.
#include <stdlib.h>  // Standard linrary for malloc.
#include <time.h>    // For time functions.
#include <unistd.h>
#include <wait.h>

#define ll long long int
#define fw(i, s, e) for (ll i = s; i < e; ++i) // forward loop so fw
#define fe(i, s, e) for (ll i = s; i <= e; ++i)
#define fb(i, e, s) for (ll i = e; i >= s; --i) //for loop but backward so fb
//codeforces snippet handle : bhaskarjoshi2001
#define ld long double

int part = 0, SZ;
int *shm_ptr;
int start = 0;
int end;

void red()
{
    printf("\033[1;35m");
}
void yellow()
{
    printf("\033[1;33m");
}
void blue()
{
    printf("\033[1;34m");
}
void green()
{
    printf("\033[1;32m");
}
void cyan()
{
    printf("\033[1;36m");
}
void reset()
{
    printf("\033[0m");
}

struct array
{
    ll L_INDEX, R_INDEX, *ARRAY;
};

// This function returns a pointer to (type casted to int) shared memory
// which gets share between the childs of a process or between threads.
// shmtget is used to allocate the memory segment.
// shmat is used to attach that shared segment to the return int pointer.
// Errors have been handled.
ll *get_shared_mem(int arr_size)
{

    key_t share_mem_key = IPC_PRIVATE;
    size_t SHM_SIZE = arr_size * 8;
    ll shared_mem_id;
    ll *shared_mem_at;
    ll f = 0;
    if ((shared_mem_id = shmget(share_mem_key, SHM_SIZE, IPC_CREAT | 0666)) < 0)
    {
        perror("SHMGET");
        f = 1;
    }
    if ((shared_mem_at = shmat(shared_mem_id, NULL, 0)) == (ll *)-1)
    {
        perror("SHMAT");
        f = 1;
    }

    if (f)
    {
        return NULL;
    }
    return shared_mem_at;
}
void print_arr(ll arr_sz, ll *arr)
{
    fw(i, 0, arr_sz)
    {
        printf("%lld ", arr[i]);
    }
    printf("\n");
}

void input(ll arr_size, ll ARR[], ll BRR[], struct array T_ARR)
{
    // Initialising the array ARR via INPUT.
    cyan();
    printf("\tEnter %lld spaced integers  : ", arr_size);
    reset();
    for (ll i = 0; i < arr_size; i++)
    {
        scanf("%lld", &ARR[i]);
        T_ARR.ARRAY[i] = BRR[i] = ARR[i];
    }
}

// Implementation of selection sort
void selection_sort(ll *s_arr, ll low, ll high)
{
    ll min, temp;
    fw(i, low, high)
    {
        min = i;
        fw(j, (1 + i), (1 + high))(s_arr[min] > s_arr[j]) ? min = j : min;
        temp = s_arr[i], s_arr[i] = s_arr[min], s_arr[min] = temp;
    }
}

// Method to merge sorted subarrays
void merge(ll *a, ll l1, ll h1, ll h2)
{
    // We can directly copy  the sorted elements
    // in the final array, no need for a temporary

    // sorted array.

    int count, m = 0;
    count = h2 + 1 - l1;
    int sorted[count + 1];
    // memset(sorted, 0, sizeof sorted);
    int i = l1;
    int k = 1 + h1;

    while (h1 >= i && h2 >= k)
    {
        if (0 < (a[k] - a[i]))
        {
            sorted[m++] = a[i++];
        }
        else if ((a[i] - a[k]) == 0)
        {
            sorted[m++] = a[i++];

            sorted[m++] = a[k++];
        }
        else if (0 < (a[i] - a[k]))
        {
            sorted[m++] = a[k++];
        }
    }

    while (0 <= (h1 - i))
    {
        sorted[m++] = a[i++];
    }
    while (0 <= (h2 - k))
    {
        sorted[m++] = a[k++];
    }
    int arr_count = 0;
    arr_count = l1;
    for (i = 0; i < count; ++l1, i++)
    {
        a[l1] = sorted[i];
    }
}
void *normal_mergesort(ll *BRR, ll low, ll high)
{
    ll sz = high + 1 - low, mid;
    if (sz >= 5)
    {
        mid = low + ((high - low) >> 1);
        normal_mergesort(BRR, low, mid), normal_mergesort(BRR, mid + 1, high);
        merge(BRR, low, mid, high);
    }
    else
    {
        selection_sort(BRR, low, high);
    }
    return NULL;
}

// Implmentation of multi-process merge-sort
void *multiprocess_mergesort(ll *ARR, ll low, ll high)
{
    ll sz = high + 1 - low, mid;
    if (sz < 5)
    {
        selection_sort(ARR, low, high);
        return NULL;
    }

    mid = low + (high - low) / 2;
    pid_t pid1;
    pid_t pid2;
    pid1 = fork();
    if (pid1 < 0)
    {
        perror("Left Child Process not created\n");
        return NULL;
    }
    else if (!pid1)
    {
        multiprocess_mergesort(ARR, low, mid);
        _exit(1);
    }
    else if (0 < pid1)
    {
        pid2 = fork();
        if (pid2 < 0)
        {
            perror("Right Child Process not created");
            return NULL;
        }
        else if (!pid2)
        {
            multiprocess_mergesort(ARR, mid + 1, high);
            _exit(1);
        }
        else
        {
            int status;
            waitpid(pid1, &status, 0), waitpid(pid2, &status, 0);
            merge(ARR, low, mid, high);
        }
    }
    return NULL;
}

// Implmentation of multi-threaded merge-sort
void *threaded_mergesort(void *T_ARR)
{
    // Extracting the struct out of parameter T_ARR
    ll l, r, mid;
    struct array *T_arr = (struct array *)T_ARR;

    // Extracting values of Array to be sorted.
    r = T_arr->R_INDEX;
    l = T_arr->L_INDEX;
    ll *arr = T_arr->ARRAY;

    // Checking for single item array.
    ll sz = r + 1 - l;
    if (sz < 5)
    {
        selection_sort(arr, l, r);
        return NULL;
    }

    // Finding middle index from where to divide the array.
    mid = l + (r - l) / 2;

    struct array L_arr, R_arr;
    // Allocating struct for left half of the provided array
    L_arr.R_INDEX = mid;
    L_arr.L_INDEX = l;
    L_arr.ARRAY = arr;

    // Allocating struct for right half of the provided array
    R_arr.R_INDEX = r;
    R_arr.L_INDEX = mid + 1;
    R_arr.ARRAY = arr;

    // Creating threads for both half.
    pthread_t tid1;
    pthread_t tid2;
    pthread_create(&tid1, NULL, threaded_mergesort, &L_arr), pthread_create(&tid2, NULL, threaded_mergesort, &R_arr);
    //Joining Threads
    pthread_join(tid1, NULL), pthread_join(tid2, NULL);

    // merge the two half.
    merge(arr, l, mid, r);
}
ld t1, t2, t3;

void print_res_multi_proc_ms(ll ARR[], ll arr_size)
{ // Multiprocess Mergesort Function call and it's performance duration.
    struct timespec ts;
    printf("\n");
    blue();
    printf("\tStarting multiprocess mergesort");

    printf("\n");

    printf("\t-------------------------------\n");
    reset();
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    ld st = ts.tv_nsec / (1e9);
    st += ts.tv_sec;
    multiprocess_mergesort(ARR, 0, arr_size - 1);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    ld en = ts.tv_nsec / (1e9);
    en += ts.tv_sec;
    yellow();
    printf("\tFinal Array = ");
    fw(i, 0, arr_size)
    {
        printf("%lld ", ARR[i]);
    }
    reset();
    printf("\n");
    t3 = en - st;
    red();
    printf("\n\tMultiprocess-Mergesort time = %Lf\n", t3);
    reset();
}

void print_res_multi_thread_ms(struct array T_ARR, ll arr_size)
{ // Multi-Threaded Mergesort Function call and it's performance duration.

    blue();
    printf("\n\tStarting multithreaded mergesort\n");
    printf("\t--------------------------------\n");
    reset();
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    ld st = ts.tv_nsec / (1e9);
    st += ts.tv_sec;

    pthread_t tid;
    pthread_create(&tid, NULL, threaded_mergesort, &T_ARR);
    pthread_join(tid, NULL);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    ld en = ts.tv_nsec / (1e9) + ts.tv_sec;
    yellow();
    printf("\tFinal Array = ");

    print_arr(arr_size, T_ARR.ARRAY);

    reset();
    red();
    t2 = en - st;
    printf("\tMultithreaded-Mergesort time = %Lf\n", t2);
    reset();
}

void print_res_normal_merge_sort(ll BRR[], int arr_size)
{
    // Normal Mergesort Function call and it's performance duration.
    struct timespec ts;
    printf("\n");
    blue();
    printf("\tStarting Normal mergesort");
    printf("\n");

    printf("\t-------------------------\n");
    reset();
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    ld st = ts.tv_nsec / (1e9);
    st += ts.tv_sec;

    normal_mergesort(BRR, 0, arr_size - 1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    ld en = ts.tv_nsec / (1e9);
    en += ts.tv_sec;
    yellow();
    printf("\tFinal Array is ");
    fw(i, 0, arr_size)
    {
        printf("%lld ", BRR[i]);
    }
    reset();
    printf("\n");
    t1 = en - st;
    red();
    printf("\tNormal Mergesort time = %Lf\n", t1);
    printf("\n");
    reset();
}

int main()
{
    // Getting size of the array to apply merge sort.
    cyan();
    printf("\tEnter Size of Array  : ");
    reset();
    ll arr_size;
    scanf("%lld", &arr_size);
    // Getting shared memory for processses implementation
    // of merge sort. This array ARR will get used in multi-process
    // mergesort.

    ll *ARR;
    //checking if the number entered is valid or not
    if (arr_size < 0 || arr_size > 99999999999)
    {
        
        printf("Enter valid number. Re-run Code\n");
        // return -1;
        _exit(1);
    }
    ARR = get_shared_mem(arr_size);
    if (ARR == 0)
    {
        printf("\tExiting...\n");
        return -1;
    }

    // BRR is just a copy of ARR array but it is not shared memory.
    // BRR will be used for normal-mergesort
    ll BRR[arr_size];

    // Initialising struct T_ARR which will be used for
    // multi-threaded mergesort.
    struct array T_ARR;
    T_ARR.R_INDEX = (arr_size - 1);
    T_ARR.L_INDEX = start;
    T_ARR.ARRAY = (ll *)malloc(arr_size * sizeof(ll));

    input(arr_size, ARR, BRR, T_ARR);

    print_res_multi_proc_ms(ARR, arr_size);

    shmdt(ARR);
    print_res_multi_thread_ms(T_ARR, arr_size);
    print_res_normal_merge_sort(BRR, arr_size);

    // Comaprison of above three method o merge-sort algo.
    green();
    printf("\tNormal mergesort is %Lf times faster than Threaded mergesort\n", t2 / t1);
    printf("\tNormal mergesort is %Lf times faster than Multi-Process mergesort\n", t3 / t1);
    printf("\tThreaded mergesort is %Lf times faster than Multi-Process mergesort\n", t3 / t2);
    reset();
    return 0;
}
