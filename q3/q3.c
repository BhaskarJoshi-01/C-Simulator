#include <assert.h>
#include <errno.h>
#include <math.h>
#include <memory.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#define SIZE 1100

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
sem_t semph[5];
// semph[1]->acouctic
// semph[2]->electric
// semph[3]->coordinator
int array_of_stage[SIZE], array_singer[SIZE];
int stages[6][SIZE];
int student_stage[SIZE];
//for array_of_stage[SIZE] 0 -> no stage , 1-> got stage
//for array_singer[SIZE] initialised  = -1 , person_onstage_without_singer = -2
// with_singer = singer_id >=0  (musician P+G+V+B)
//variables as per question
int k;
int a, e, c, t1, t2, t;

//structure
typedef struct Musician
{
    int id, arrival_time; //for input
    //piano = 1, guitar = 2, violin = 3, bass = 4, singer = 5
    char name[75], instrument;
    pthread_mutex_t music_mutex[5];
    //  music_mutex[1] -> acoustics
    //  music_mutex[2]; -> electrical
    //  music_mutex[3]; -> needed for singer
    pthread_t music_pthread;
    int stype;
} Musician;

Musician musicians[SIZE];

// typedef struct thread_arg
// {
//     Musician m;
//     int type;
// } thread_arg;

void *musiciansinit(void *m);
void *thread_func_a(void *m);
void *thread_func_e(void *m);

void case_ae(int id);
void stage(int id, int type);
void case_unity(int id, int tmp);
void case_5(int id);

void *join(void *m);

int main()
{
    srand(time(0));

    for (int i = 0; i < SIZE; i++)
    {
        array_singer[i] = -1;
        for (int j = 0; j < 5; j++)
        {
            stages[j][i] = -1;
        }
        student_stage[i] = -1;
    }
    red();
    printf("Enter : k, a, e, c, t1, t2, t\n");
    reset();
    cyan();
    scanf("%d", &k);
    assert(k >= 0);
    scanf("%d %d %d", &a, &e, &c);
    scanf("%d %d", &t1, &t2);
    assert(t1 <= t2);
    scanf("%d", &t);
    assert(t >= 0);
    sem_init(&semph[1], 0, a);
    sem_init(&semph[2], 0, e);
    sem_init(&semph[3], 0, c);
    for (int i = 0; i <= (k - 1); ++i)
    {
        char temp_1[75] = "\0", temp[2];
        scanf("%s", temp_1);
        musicians[i].id = i;
        scanf("%s", temp);
        strcpy(musicians[i].name, temp_1);
        musicians[i].instrument = temp[0];

        if (temp[0] != 'p' && temp[0] != 'g' && temp[0] != 'v' && temp[0] != 'b' && temp[0] != 's')
        {
            printf("Error : Enter correct values\n");
            return 0;
        }
        scanf("%d", &musicians[i].arrival_time);
        pthread_mutex_init(&(musicians[i].music_mutex[1]), NULL);
        // pthread_mutex_init(&(musicians[i].music_mutex[2]), NULL);
        pthread_mutex_init(&(musicians[i].music_mutex[3]), NULL);
    }
    reset();
    for (int i = 0; i <= (k - 1); ++i)
    {
        pthread_create(&(musicians[i].music_pthread), NULL, musiciansinit, &musicians[i]);
    }
    for (int i = 0; i <= (k - 1); ++i)
    {
        pthread_join(musicians[i].music_pthread, NULL);
    }
    for (int i = 0; i < (k - 1); ++i)
    {
        pthread_mutex_destroy(&musicians[i].music_mutex[1]);
        // pthread_mutex_destroy(&musicians[i].music_mutex[2]);
        pthread_mutex_destroy(&musicians[i].music_mutex[3]);
    }
    printf("\x1B[31mFinished \x1B[0m\n");
    return 0;
}

void *musiciansinit(void *m)
{
    Musician *temp = (Musician *)m;
    sleep(temp->arrival_time); //do later
    printf("\033[1;32m%s %c arrived \033[0m\n", temp->name, temp->instrument);
    if (temp->instrument == 'p')
        case_ae(temp->id);
    else if (temp->instrument == 'g')
        case_ae(temp->id);
    else if (temp->instrument == 'v')
        case_unity(temp->id, 1);
    else if (temp->instrument == 'b')
        case_unity(temp->id, 2);
    else if (temp->instrument == 's')
        case_5(temp->id);
    else
    {
        printf("\x1B[31mError :\x1B[0m No instrument matched\n");
    }
}

void case_ae(int id)
{
    pthread_t thread_p1;
    pthread_create(&thread_p1, NULL, thread_func_a, (void *)&musicians[id]);
    pthread_t thread_p2;
    pthread_create(&thread_p2, NULL, thread_func_e, (void *)&musicians[id]);
    pthread_join(thread_p1, NULL), pthread_join(thread_p2, NULL);
}

void *thread_func_a(void *m)
{

    // thread_arg *var1 = (thread_arg *)m;
    Musician *temp = (Musician *)m;

    struct timespec tim;
    int flag;
    // int type = var1->type;
    if (clock_gettime(CLOCK_REALTIME, &tim) == -1)
    {
        perror("Timespec");
        pthread_exit(NULL);
    }
    tim.tv_sec = tim.tv_sec + t;
    flag = sem_timedwait(&semph[1], &tim);

    if (flag != -1) //getting stage
    {
        pthread_mutex_lock(&temp->music_mutex[1]);

        if (array_of_stage[temp->id] != 0)
        {
            pthread_mutex_unlock(&temp->music_mutex[1]);
        }
        else
        {
            array_of_stage[temp->id] = 1;
            pthread_mutex_unlock(&temp->music_mutex[1]);
            stage(temp->id, 1);
        }
        sem_post(&semph[1]);
    }
    else
    {
        pthread_mutex_lock(&temp->music_mutex[1]);
        if (!array_of_stage[temp->id])
        {
            printf("\033[0;36m%s %c left because of impatience\033[0m\n", temp->name, temp->instrument);
            // printf("\n");
            array_of_stage[temp->id] = 1;
        }
        pthread_mutex_unlock(&temp->music_mutex[1]);
    }
    // printf("Stage is %d\n", array_of_stage[]);
}

void *thread_func_e(void *m)
{

    // thread_arg *var1 = (thread_arg *)m;
    Musician *temp = (Musician *)m;
    struct timespec tim;
    int flag;
    // int type = var1->type;
    if (clock_gettime(CLOCK_REALTIME, &tim) == -1)
    {
        perror("Timespec");
        pthread_exit(NULL);
    }
    tim.tv_sec = tim.tv_sec + t;
    flag = sem_timedwait(&semph[2], &tim);

    if (flag != -1) //getting stage
    {
        pthread_mutex_lock(&temp->music_mutex[1]);

        if (array_of_stage[temp->id] != 0)
        {
            pthread_mutex_unlock(&temp->music_mutex[1]);
        }
        else
        {
            array_of_stage[temp->id] = 1;
            pthread_mutex_unlock(&temp->music_mutex[1]);
            stage(temp->id, 2);
        }
        sem_post(&semph[2]);
    }
    else
    {
        pthread_mutex_lock(&temp->music_mutex[1]);
        if (!array_of_stage[temp->id])
        {
            printf("\033[0;36m%s %c left because of impatience\033[0m\n", temp->name, temp->instrument);
            // printf("\n");
            array_of_stage[temp->id] = 1;
        }
        pthread_mutex_unlock(&temp->music_mutex[1]);
    }
    // printf("Stage is %d\n", array_of_stage[]);
}

void stage(int id, int type)
{

    pthread_mutex_lock(&(musicians[id].music_mutex[3]));
    // int j;
    int sleep_time, z = 0;

    array_singer[id] = -2;
    pthread_mutex_unlock(&(musicians[id].music_mutex[3])),
        pthread_mutex_lock(&(musicians[id].music_mutex[3]));

    int till;
    if (type == 1)
    {
        till = a;
    }
    else
    {
        till = e;
    }

    for (z = 0; z < till; ++z)
    {
        if (stages[type][z] == -1)
        {
            stages[type][z] = 1;
            student_stage[id] = z + 1;
            break;
        }
    }

    pthread_mutex_unlock(&(musicians[id].music_mutex[3]));

    // sem_getvalue(&semph[type], &j);
    sleep_time = rand() % (1 + (t2 - t1)) + t1;
    char strr[20];
    if (type == 1)
    {
        strcpy(strr, " Acoustic ");
    }
    if (type == 2)
    {
        strcpy(strr, " Electric ");
    }
    printf("\033[0;33m%s performing %c at %s stage %d for %d sec\033[0m\n", musicians[id].name, musicians[id].instrument, strr, student_stage[id], sleep_time);
    // printf("\n");
    sleep(sleep_time); // performing
    pthread_mutex_lock(&(musicians[id].music_mutex[3]));
    if (array_singer[id] > -1)
    {
        sleep(2);
        pthread_mutex_unlock(&(musicians[id].music_mutex[3]));
    }
    else
    {
        pthread_mutex_unlock(&(musicians[id].music_mutex[3]));
    }
    // yellow();
    printf("\033[1;33m%s performance %c at %s stage %d  ended. \033[0m\n", musicians[id].name, musicians[id].instrument, strr, student_stage[id]);
    // reset();
    sem_wait(&semph[3]); //coordinator available

    printf("\033[0;34m%s collecting T-shirt\033[0m\n", musicians[id].name);
    // printf("\n");
    if (array_singer[id] > -1)
    {
        printf("\033[0;34m%s collecting T-shirt\033[0m\n", musicians[array_singer[id]].name);
        // printf("\n");
    }
    pthread_mutex_lock(&(musicians[id].music_mutex[3]));
    stages[type][z] = -1;
    pthread_mutex_unlock(&(musicians[id].music_mutex[3]));

    sem_post(&semph[3]);
    return;
}

void case_unity(int id, int type)
{
    // musicians[id].stype = type;
    struct timespec tim;
    int flag, typ1 = 1;
    if (clock_gettime(CLOCK_REALTIME, &tim) == -1)
    {
        perror("Timespec");
        pthread_exit(NULL);
    }
    tim.tv_sec = tim.tv_sec + t;
    flag = sem_timedwait(&semph[type], &tim);

    if (flag != -1)
    {
        pthread_mutex_lock(&(musicians[id].music_mutex[1]));
        if (!array_of_stage[id])
        {
            array_of_stage[id] = typ1;
            pthread_mutex_unlock(&(musicians[id].music_mutex[1]));
            typ1 = 1;
            stage(id, type);
        }
        else
        {
            typ1 = 1;
            pthread_mutex_unlock(&(musicians[id].music_mutex[1]));
        }
        sem_post(&semph[type]);
    }
    else
    {
        pthread_mutex_lock(&(musicians[id].music_mutex[1]));
        if (!array_of_stage[id])
        {
            printf("\033[0;36m%s %c left because of impatience\033[0m\n", musicians[id].name, musicians[id].instrument);
            // printf("\n");
            array_of_stage[id] = 1;
        }
        pthread_mutex_unlock(&(musicians[id].music_mutex[1]));
        pthread_exit(NULL);
    }
}

void case_5(int id)
{
    pthread_t thread_s1;
    pthread_create(&thread_s1, NULL, thread_func_a, (void *)&musicians[id]);
    pthread_t thread_s2, thread_s3;
    pthread_create(&thread_s2, NULL, thread_func_e, (void *)&musicians[id]),
        pthread_create(&thread_s3, NULL, join, (void *)&musicians[id]);
    pthread_join(thread_s1, NULL),
        pthread_join(thread_s2, NULL),
        pthread_join(thread_s3, NULL);
}

void *join(void *m)
{
    Musician *temp = (Musician *)m;
    int typ = 1;
    while (!(array_of_stage[temp->id]))
    {
        for (int i = 0; i <= (k - 1); ++i)
        {
            pthread_mutex_lock(&(temp->music_mutex[3]));
            if (!(array_singer[i] + 2) && !(array_of_stage[temp->id]))
            {
                array_singer[i] = temp->id;
                array_of_stage[temp->id] = typ;
                printf("\033[1;35m%s joined %s's performance, extending performance by 2 secs .\033[0m\n", temp->name, musicians[i].name);
                break;
            }
            pthread_mutex_unlock(&(temp->music_mutex[3]));
        }
    }
}
