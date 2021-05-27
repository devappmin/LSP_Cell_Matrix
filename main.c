#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_MATRIX_SIZE 100

typedef enum LIFE {DEAD, LIVE} LIFE;

typedef struct Matrix {
    int **matrix;
    int xsize;
    int ysize;
} Matrix;

void setMatrix(int fd, int*** matrix) {
    int i = 1, j = 1;
    int buf[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE] = {{0, 0}};

    char c;
    while(read(fd, &c, 1) > 0) {
        
        if(c == ' ') {
            i++;
            continue;
        }
        if(c == 13) continue;
        if(c == '\n') {
            j++;
            i = 1;
            continue;
        }
        buf[j][i] = c - 48;
    }
    printf("%d %d\n", i, j);

    int **temp;
    temp = (int **)malloc(sizeof(int *) * (j + 2));
    for (int k = 0; k < j + 2; k++) 
        temp[k] = (int *)malloc(sizeof(int) * (i + 2));
    

    for(int k = 0; k < j + 2; k++) {
        for(int l = 0; l < i + 2; l++) {
            temp[k][l] = buf[k][l];
        }
    }
    *matrix = temp;
}

LIFE rule1(int** matrix, int y, int x) {
    int cnt = 0;

    // 죽을 경우는 스킵한다.
    if (matrix[y][x] == DEAD) return DEAD;

    // 살아있는 경우 이웃 세포 탐색
    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            
            // 자기 자신을 가리키는 경우 스킵
            if(i == 0 && j == 0) continue;

            // 만약에 이웃 세포가 살아있으면 살아있는 세포 수에 1 추가
            if(matrix[y + i][x + j] == LIVE) cnt++;
        }
    }

    // 살아있는 이웃이 0 ~ 2, 7 ~ 8개면 해당 세포를 죽이고 
    // 살아있는 이웃이 3 ~ 6개면 해당 세포를 살린다.
    return cnt >= 7 || cnt <= 2 ? DEAD : LIVE;
}

LIFE rule2(int** matrix, int y, int x) {
    int cnt = 0;

    if (matrix[y][x] == LIVE) return LIVE;

    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {

            if(i == 0 && j == 0) continue;

            if(matrix[y + i][x + j] == LIVE) cnt++;
        }
    }

    return cnt == 4 ? LIVE : DEAD;
}

void nextgen(int** matrix) {
    for(int i = 1; i < 9; i++) {
        for(int j = 1; j < 8; j++) {
            if(matrix[i][j] == LIVE) printf("%d ", rule1(matrix, i, j));
            else if(matrix[i][j] == DEAD) printf("%d ", rule2(matrix, i, j));
        }
        printf("\n");
    }
}

int main() {
    int fd;

    if((fd = open("input.matrix", O_RDONLY)) < 0) {
        printf("Open error occurred\n");
        exit(-1);
    }
    int size = lseek(fd, 0, SEEK_END);
    printf("%d\n", size);
    lseek(fd, 0, SEEK_SET);

    int** matrix = (int **)malloc(sizeof(int *));

    setMatrix(fd, &matrix);
    nextgen(matrix);
    return 0;
}