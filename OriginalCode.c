#include <stdio.h>
#include "stdlib.h"
#include "time.h"
#include <sys/time.h>

#define IMAGE_LENGTH 1920
#define IMAGE_WIDE 1080

typedef struct pixel{
    int r;
    int g;
    int b;
}pixel;

int main() {
    int i, j;

    long elapsed_seconds, elapsed_nanoseconds;
    struct timespec start_time, end_time;

    pixel** img = (pixel**) malloc(IMAGE_LENGTH * sizeof(pixel *));
    pixel** new_img = (pixel**) malloc(IMAGE_LENGTH * sizeof(pixel *));

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

    // 记录程序开始执行的时间
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for(i = 1; i < IMAGE_LENGTH - 1; i++) {
        for(j = 1; j < IMAGE_WIDE - 1; j++) {
            new_img[i][j].r = (img[i - 1][j].r + img[i + 1][j].r + img[i][j - 1].r + img[i][j + 1].r) / 4;
            new_img[i][j].g = (img[i - 1][j].g + img[i + 1][j].g + img[i][j - 1].g + img[i][j + 1].g) / 4;
            new_img[i][j].b = (img[i - 1][j].b + img[i + 1][j].b + img[i][j - 1].b + img[i][j + 1].b) / 4;
        }
    }

    // 记录程序结束执行的时间
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // 计算程序的运行时间（以秒为单位）
    elapsed_seconds = end_time.tv_sec - start_time.tv_sec;
    elapsed_nanoseconds = end_time.tv_nsec - start_time.tv_nsec;
    double run_time = (double) elapsed_seconds + (double) elapsed_nanoseconds / 1000000000;

    printf("程序运行时间：%lf毫秒\n", run_time * 1000);

    return 0;
}