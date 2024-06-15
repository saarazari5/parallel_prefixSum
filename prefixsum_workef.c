#define _POSIX_C_SOURCE 200112L /* Or higher */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <string.h>

typedef struct
{
    int *arr;
    int n;
    int s;
    int e;
    int r;
    int step;
    int i;

    int valToAdd; 

    pthread_barrier_t *barrier;
} ThreadData;


void serial_prefix_sum_seindex(int *arr, int s, int e, int n)
{

    int isFirst = 1;
    
    int end = e < n ? e : n;
    for (int i = s; i < end; i++)
    {
        if(isFirst) {
            isFirst = 0;
            continue;
        }
        arr[i] += arr[i - 1];
    }
}

void *prefix_sum_worker3(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int *arr = data->arr;
    int n = data->n;
    int s = data->s;
    int e = data->e;
    int v = data->valToAdd;

    int end = e < n ? e : n;

    for (int i = s; i < end; i ++) {
        arr[i] += v;
    }
}

void *prefix_sum_worker1(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int *arr = data->arr;
    int n = data->n;
    int s = data->s;
    int e = data->e;

    serial_prefix_sum_seindex(arr,s,e,n);
}

void *prefix_sum_worker2(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int *arr = data->arr;
    int r = data->r;
    int step = data->step;
    int i = data->i;

  
    int x = arr[i] + arr[i - step];

    pthread_barrier_wait(data->barrier);

    if(i >= step) {
        arr[i] = x;
    }    
    pthread_exit(NULL);
}

void parallel_prefix_sum(int *arr, int n)
{
    int log_n = (int)ceil(log2(n));
    pthread_t *threads = (pthread_t *)malloc(n * sizeof(pthread_t));
    ThreadData *thread_data = (ThreadData *)malloc(n * sizeof(ThreadData));

    pthread_barrier_t barrier;


    for(int i = 0; i < n; i += log_n) {
        thread_data[i].arr = arr;
        thread_data[i].n = n;
        thread_data[i].s = i;
        thread_data[i].e = i + log_n;

        pthread_create(&threads[i], NULL, prefix_sum_worker1, (void *)&thread_data[i]);
    }

    for (int i = 0; i < n; i += log_n)
    {
        pthread_join(threads[i], NULL);
    }

    //phase2 can be parallise:

    int *subarr = (int *)malloc((n/log_n) * sizeof(int));

    for(int i = 0 ; i < n/log_n; i ++) {

        if( log_n*i + (log_n - 1) >= n) {
            subarr[i] = arr[n-1];
        }
        subarr[i] = arr[ log_n*i + (log_n - 1)];
    }


    int loglog_n = (int)ceil(log2(n/log_n));

    for (int r = 1; r <= loglog_n; r++)
    {
        int step = (int)pow(2, r - 1);
        pthread_barrier_init(&barrier, NULL, (n/log_n)-step);

        // parallel for:
        for (int i = step; i < n/log_n; i++)
        {

            thread_data[i].arr = subarr;
            thread_data[i].i = i;
            thread_data[i].r = r;
            thread_data[i].step = step;
            thread_data[i].barrier = &barrier;

            pthread_create(&threads[i], NULL, prefix_sum_worker2, (void *)&thread_data[i]);
        }

        for (int i = step; i < n/log_n; i++)
        {
            pthread_join(threads[i], NULL);
        }

        pthread_barrier_destroy(&barrier);

    }


    //phase3:
    for(int i = 0 ; i < n/log_n; i ++) {
        thread_data[i].arr = arr;
        thread_data[i].n = n;
        thread_data[i].s = (i+1)*log_n;
        thread_data[i].e = thread_data[i].s + log_n;
        thread_data[i].valToAdd = subarr[i];

        pthread_create(&threads[i], NULL, prefix_sum_worker3, (void *)&thread_data[i]);

    }
    
    for(int i = 0 ; i < n/log_n; i ++) {
        pthread_join(threads[i], NULL);
    }



    free(threads);
    free(thread_data);
    
}



int main()
{
    int n = 16; // Size of the array
    int *arr = (int *)malloc(n * sizeof(int));

    if (arr == NULL)
    {
        printf("Memory allocation failed\n");
        return 1;
    }

    // Initialize the array with random values
    for (int i = 0; i < n; i++)
    {
        arr[i] = i+1;
    }

    int *cpy = (int *)malloc(n * sizeof(int));
    memcpy(cpy, arr, n * sizeof(int));

    clock_t start, end;
    double cpu_time_used;

    // Benchmark parallel prefix sum
    start = clock();
    parallel_prefix_sum(arr, n);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Parallel prefix sum time: %f seconds\n", cpu_time_used);

    // Benchmark serial prefix sum
    start = clock();
    serial_prefix_sum_seindex(cpy, 0 , n , n);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Serial prefix sum time: %f seconds\n", cpu_time_used);

    // Uncomment the following lines to print the result
    // for (int i = 0; i < n; i++) {
    //     printf("%d ", arr[i]);
    // }
    // printf("\n");

    // for (int i = 0; i < n; i++) {
    //     printf("%d ", cpy[i]);
    // }
    // printf("\n");

    // Clean up
    free(arr);

    return 0;
}
