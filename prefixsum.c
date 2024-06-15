#define _POSIX_C_SOURCE 200112L /* Or higher */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <string.h>

typedef struct
{
    int *arr;
    int r;
    int step;
    int i;
    pthread_barrier_t *barrier;

} ThreadData;

void *prefix_sum_worker(void *arg)
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


    for (int r = 1; r <= log_n; r++)
    {
        int step = (int)pow(2, r - 1);
        pthread_barrier_init(&barrier, NULL, n-step);

        // parallel for:
        for (int i = step; i < n; i++)
        {

            thread_data[i].arr = arr;
            thread_data[i].i = i;
            thread_data[i].r = r;
            thread_data[i].step = step;
            thread_data[i].barrier = &barrier;

            pthread_create(&threads[i], NULL, prefix_sum_worker, (void *)&thread_data[i]);
        }

        for (int i = step; i < n; i++)
        {
            pthread_join(threads[i], NULL);
        }

        pthread_barrier_destroy(&barrier);

    }


    free(threads);
    free(thread_data);
    
}

void serial_prefix_sum(int *arr, int n)
{
    for (int i = 1; i < n; i++)
    {
        arr[i] += arr[i - 1];
    }
}

int main()
{
    int n = 1000; // Size of the array
    int *arr = (int *)malloc(n * sizeof(int));

    if (arr == NULL)
    {
        printf("Memory allocation failed\n");
        return 1;
    }

    // Initialize the array with random values
    for (int i = 0; i < n; i++)
    {
        arr[i] = i + 1;
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
    serial_prefix_sum(cpy, n);
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
