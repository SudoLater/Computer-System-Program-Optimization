#include <stdio.h>
#include "stdlib.h"
#include "time.h"
#include <sys/time.h>
#include <pthread.h>
#include <emmintrin.h> // SSE2

#define BLOCK_NUM 1
#define IMAGE_LENGTH 1920
#define IMAGE_WIDE 1080
#define NUM_THREADS 4

typedef struct pixel {
    int r;
    int g;
    int b;
}pixel;

struct thread_data {
    int thread_id;
    pixel **img;
    pixel **new_img;
};

void* smooth_image(void* threadarg) {
    struct thread_data *my_data = (struct thread_data *) threadarg;
    int id = my_data->thread_id;
    pixel **img = my_data->img;
    pixel **new_img = my_data->new_img;
    int i, j, bi, bj;
    int rows_per_thread = IMAGE_LENGTH / NUM_THREADS;
    int start_row = id * rows_per_thread + 1;
    int end_row = (id == NUM_THREADS - 1) ? IMAGE_LENGTH - 1 : (id + 1) * rows_per_thread;
    int blockSize = IMAGE_WIDE / BLOCK_NUM;

    for (bi = start_row; bi < end_row; bi += blockSize) {
        for (bj = 1; bj < IMAGE_WIDE - 1; bj += blockSize) {
            for (i = bi; i < (bi + blockSize) && i < end_row; i++) {
                for (j = bj; j < (bj + blockSize) && j < IMAGE_WIDE - 1; j += 4) {
                    __m128i left_center, right_center, res;

                    left_center = _mm_set_epi32(img[i][j + 1].r, img[i][j + 1].g, img[i][j + 1].b, 0);
                    right_center = _mm_set_epi32(img[i][j + 2].r, img[i][j + 2].g, img[i][j + 2].b, 0);

                    res = _mm_avg_epu8(_mm_set_epi32(img[i - 1][j].r, img[i - 1][j].g, img[i - 1][j].b, 0),
                                       _mm_set_epi32(img[i + 1][j].r, img[i + 1][j].g, img[i + 1][j].b, 0));
                    res = _mm_avg_epu8(res, _mm_set_epi32(img[i][j - 1].r, img[i][j - 1].g, img[i][j - 1].b, 0));
                    res = _mm_avg_epu8(res, left_center);

                    *((int *) &new_img[i][j]) = _mm_cvtsi128_si32(res);

                    res = _mm_avg_epu8(_mm_set_epi32(img[i - 1][j + 1].r, img[i - 1][j + 1].g, img[i - 1][j + 1].b, 0),
                                       _mm_set_epi32(img[i + 1][j + 1].r, img[i + 1][j + 1].g, img[i + 1][j + 1].b, 0));
                    res = _mm_avg_epu8(res, _mm_set_epi32(img[i][j].r, img[i][j].g, img[i][j].b, 0));
                    res = _mm_avg_epu8(res, right_center);
                    *((int *) &new_img[i][j + 1]) = _mm_cvtsi128_si32(res);

                    res = _mm_avg_epu8(_mm_set_epi32(img[i - 1][j + 2].r, img[i - 1][j + 2].g, img[i - 1][j + 2].b, 0),
                                       _mm_set_epi32(img[i + 1][j + 2].r, img[i + 1][j + 2].g, img[i + 1][j + 2].b, 0));
                    res = _mm_avg_epu8(res, left_center);
                    res = _mm_avg_epu8(res, _mm_set_epi32(img[i][j + 3].r, img[i][j + 3].g, img[i][j + 3].b, 0));

                    *((int *) &new_img[i][j + 2]) = _mm_cvtsi128_si32(res);

                    res = _mm_avg_epu8(_mm_set_epi32(img[i - 1][j + 3].r, img[i - 1][j + 3].g, img[i - 1][j + 3].b, 0),
                                       _mm_set_epi32(img[i + 1][j + 3].r, img[i + 1][j + 3].g, img[i + 1][j + 3].b, 0));
                    res = _mm_avg_epu8(res, right_center);
                    res = _mm_avg_epu8(res, _mm_set_epi32(img[i][j + 4].r, img[i][j + 4].g, img[i][j + 4].b, 0));

                    *((int *) &new_img[i][j + 3]) = _mm_cvtsi128_si32(res);
                }

                for (j -= IMAGE_WIDE - 1; j > 0; j--) {
                    new_img[i][j].r = (img[i - 1][j].r + img[i + 1][j].r + img[i][j - 1].r + img[i][j + 1].r) >> 2;
                    new_img[i][j].g = (img[i - 1][j].g + img[i + 1][j].g + img[i][j - 1].g + img[i][j + 1].g) >> 2;
                    new_img[i][j].b = (img[i - 1][j].b + img[i + 1][j].b + img[i][j - 1].b + img[i][j + 1].b) >> 2;
                }
            }
        }
    }
    pthread_exit(NULL);
}

int main() {
    int i, j;
    long elapsed_seconds, elapsed_nanoseconds;
    struct timespec start_time, end_time;

    pixel **img = (pixel **) malloc(IMAGE_LENGTH * sizeof(pixel *));
    pixel **new_img = (pixel **) malloc(IMAGE_LENGTH * sizeof(pixel *));

    for (i = 0; i < IMAGE_LENGTH; ++i) {
        img[i] = (pixel *) malloc(IMAGE_WIDE * sizeof(pixel));
        new_img[i] = (pixel *) malloc(IMAGE_WIDE * sizeof(pixel));
    }
    for (i = 0; i < IMAGE_LENGTH; i++) {
        for (j = 0; j < IMAGE_WIDE; j++) {
            img[i][j].r = i + j;
            img[i][j].g = i + j;
            img[i][j].b = i + j;
        }
    }

    pthread_t threads[NUM_THREADS];
    struct thread_data thread_data_array[NUM_THREADS];

// 记录程序开始执行的时间
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (i = 0; i < NUM_THREADS; i++) {
        thread_data_array[i].thread_id = i;
        thread_data_array[i].img = img;
        thread_data_array[i].new_img = new_img;
        int rc = pthread_create(&threads[i], NULL, smooth_image, (void *) &thread_data_array[i]);
        if (rc) {
            printf("Error: pthread_create() failed\n");
            exit(-1);
        }
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

// 记录程序结束执行的时间
    clock_gettime(CLOCK_MONOTONIC, &end_time);

// 计算程序的运行时间（以毫秒为单位）
    elapsed_seconds = end_time.tv_sec - start_time.tv_sec;
    elapsed_nanoseconds = end_time.tv_nsec - start_time.tv_nsec;
    double run_time = (double) elapsed_seconds + (double) elapsed_nanoseconds / 1000000;

    printf("程序运行时间：%lf毫秒\n", run_time);

    return 0;
}
