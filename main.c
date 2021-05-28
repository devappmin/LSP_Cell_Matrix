#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_MATRIX_SIZE 1200

typedef enum LIFE {DEAD, LIVE} LIFE;

typedef struct Matrix {
    int **table;
    int xsize;
    int ysize;
} Matrix;

char getMainInput();
void setMatrix(int fd, Matrix* matrix);
void writeMatrix(Matrix* matrix, int gen);
void printMatrix(Matrix* matrix);
LIFE rule1(Matrix* matrix, int y, int x);
LIFE rule2(Matrix* matrix, int y, int x);
void nextGenP(Matrix* matrix, int processCount);
void nextGen(Matrix* matrix);
int getIntegerCipher(int input);

int main() {
    int fd;

    if((fd = open("input.matrix", O_RDONLY)) < 0) {
        printf("Open error occurred\n");
        exit(-1);
    }

    Matrix *matrix = (Matrix *)malloc(sizeof(Matrix));
    setMatrix(fd, matrix);
    printf("Matrix Size : x[%d] y[%d]\n", matrix->xsize, matrix->ysize);
    char mode = getMainInput();
    int loop;
        
    printf("Input Loop Count\n");
    printf("\033[0;32mCell_Matrix_Game\033[0m# ");
    scanf("%d", &loop);
    struct timeval starttime, endtime;
    if(mode == '1') {
        return 0;
    } else if(mode == '2') {
        gettimeofday(&starttime, NULL);
        for(int i = 1; i <= loop; i++) {
            printf("%d generation\n", i);
            nextGen(matrix);
            //printMatrix(matrix);
            writeMatrix(matrix, i);
        }
        gettimeofday(&endtime, NULL);
    } else if(mode == '3') {
        int process;
        printf("Input Process Count\n");
        printf("\033[0;32mCell_Matrix_Game\033[0m# ");
        scanf("%d", &process);
        
        gettimeofday(&starttime, NULL);
        for(int i = 1; i <= loop; i++) {
            printf("%d generation\n", i);
            nextGenP(matrix, process);
            //printMatrix(matrix);
            writeMatrix(matrix, i);
        }
        gettimeofday(&endtime, NULL);
    }

    printf("Elapsed Time: %ld ms\n", endtime.tv_sec * 1000 + endtime.tv_usec / 1000 - starttime.tv_sec * 1000 - starttime.tv_usec / 1000);

    return 0;
}

char getMainInput() {
    printf("Cell Matrix Game Main Title\n");
    printf("Linux System Programming & Practice\n");
    printf("2021-1 Design Assignment\n");
    printf("20172608 Kim Seung Hwan\n\n\n");
    printf("[1] Quit\n");
    printf("[2] Sequential Processing\n");
    printf("[3] Multi-Processing\n");
    printf("[4] Multi Threading\n\n");

    printf("\033[0;32mCell_Matrix_Game\033[0m# ");

    char c;
    scanf("%c", &c);

    return c;
}

void setMatrix(int fd, Matrix* matrix) {
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

    int **temp;
    temp = (int **)malloc(sizeof(int *) * (j + 2));
    for (int k = 0; k < j + 2; k++) 
        temp[k] = (int *)malloc(sizeof(int) * (i + 2));
    

    for(int k = 0; k < j + 2; k++) {
        for(int l = 0; l < i + 2; l++) {
            temp[k][l] = buf[k][l];
        }
    }
    matrix->table = temp;
    matrix->xsize = i;
    matrix->ysize = j;
}

void writeMatrix(Matrix* matrix, int gen) {
    int fd;
    
    char* filename = (char *)malloc(12 + getIntegerCipher(gen));
    sprintf(filename, "gen_%d.matrix", gen);

    if((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
        printf("Writing %s file error occurred\n", filename);
        exit(-1);
    }

    for(int i = 1; i <= matrix->ysize; i++) {
        for(int j = 1; j <= matrix->xsize; j++) {
            char c = matrix->table[i][j] + 48;
            write(fd, &c, 1);

            if(j != matrix->xsize) write(fd, " ", 1);
        }
        if(i != matrix->ysize) write(fd, "\n", 1);
    }
}

void printMatrix(Matrix* matrix) {
    for(int i = 1; i <= matrix->ysize; i++) {
        for(int j = 1; j <= matrix->xsize; j++) {
            printf("%d ", matrix->table[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void nextGenP(Matrix* matrix, int processCount) {

    // 수정된 Matrix의 값을 저장하기 위한 버퍼
    int buf[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE] = {{0, 0}};

    // 파이프를 통해서 Parent - Child 간의 변수 값을 공유
    int pipe_fd[2];

    // processCount개의 자식 프로세스 pid 생성
    pid_t* pid = (pid_t *)malloc(sizeof(pid_t) * processCount);

    // Matrix의 행을 균등하게 분배해서 그 값을 넣기 위한 동적 배열
    int* divider = (int *) malloc(sizeof(int) * (processCount + 1));
    divider[0] = 0;

    // 행을 균등하게 나눠서 divider에 넣는 부분
    //
    // 행의 값이 11이고 프로세스의 개수가 4개라고 가정하면
    // 0번째 프로세스: 0, 1
    // 1번째 프로세스: 2, 3, 4
    // 2번째 프로세스: 5, 6, 7
    // 3번째 프로세스: 8, 9, 10
    // 이렇게 값을 나누기 위해 divider = {0, 2, 5, 8, 10} 순서대로 값을 저장하게 한다.
    for(int i = 1; i <= processCount; i++) {
        divider[i] = divider[i - 1] + matrix->ysize/processCount + ((i - 1) >= processCount - matrix->ysize%processCount ? 1 : 0);
    }

    for(int i = 0; i <= processCount; i++) {
        printf("[%d] ", divider[i]);
    }
    printf("\n");

    // Pipe를 생성한다.
    pipe(pipe_fd);


    // processCount만큼 fork()를 한다.
    int cnt = 0;
    while(pid[cnt] = fork()) {
        if(++cnt == processCount) break;
    }

    // 0번째 프로세스부터 processCount - 1번째 프로세스까지 각각 처리할 업무를 준다.
    for(int n = 0; n < processCount; n++) {
        if(pid[n] == 0) {
            printf("%d: Process %d Started\n", n, getpid());
            // 자신의 프로세스 번호에 맞는 처리해야 하는 행을 찾는다.
            for(int i = divider[n]; i < divider[n + 1]; i++) {

                // 해당 행의 값들을 각각 불러온다.
                for(int j = 0; j < matrix->xsize; j++) {
                    // 각각의 rule을 적용하여 결과 값을 저장한다.
                    if(matrix->table[i + 1][j + 1] == LIVE) buf[i][j] = rule1(matrix, i + 1, j + 1);
                    else if (matrix->table[i + 1][j + 1] == DEAD) buf[i][j] = rule2(matrix, i + 1, j + 1);
                }
                // 파이프를 통해서 해당 배열을 넘긴다.
                printf("%d: Starts loop %d/%d\n",n, i, divider[n + 1]);
                write(pipe_fd[1], buf[i], sizeof(int) * matrix->xsize);
                printf("%d: Finishes loop %d/%d - %d %d %d %d %d %d\n",n, i, divider[n + 1], buf[i][0], buf[i][1], buf[i][2], buf[i][3], buf[i][4], buf[i][5]);
            }

            // 다 했으면 해당 프로세스를 종료한다.
            printf("%d: Process %d Finished its job\n", n, getpid());
            exit(0);
            break;
        }
    }

    // 부모 프로세스를 정하는 변수
    int allPid = 0;
    for(int i = 0; i < processCount; i++) {
        allPid |= pid[i];
    }
    
    // 부모 프로세스가 처리해야하는 업무
    if(allPid > 0) {
        printf("Starts job for parents\n");
        for(int i = 0; i < processCount; i++) {
            int status, retval;
            for(int k = divider[i]; k < divider[i + 1]; k++) {
                read(pipe_fd[0], buf[k], sizeof(int) * matrix->xsize);
                printf("Parent: read buffer array[%d] - %d %d %d %d %d %d\n", k, buf[k][0], buf[k][1], buf[k][2], buf[k][3], buf[k][4], buf[k][5]);
            }
            printf("waiting %d is finished\n", pid[i]);
            retval = waitpid(pid[i], &status, 0);
            printf("%d is finished\n", pid[i]);
            /*
            for(int j = divider[i]; j < divider[i + 1]; j++) {
                for(int k = 0; k < matrix->xsize; k++) {
                    buf[j][k] = buf[j][k];
                }
            }*/
        }
        
        for(int i = 1; i <= matrix->ysize; i++) {
            for(int j = 1; j <= matrix->xsize; j++) {
                matrix->table[i][j] = buf[i - 1][j - 1];
            }
        }

    }
}

void nextGen(Matrix* matrix) {
    LIFE buf[matrix->ysize][matrix->xsize];

    for(int i = 1; i <= matrix->ysize; i++) {
        for(int j = 1; j <= matrix->xsize; j++) {
            if(matrix->table[i][j] == LIVE) buf[i - 1][j - 1] = rule1(matrix, i, j);
            else if(matrix->table[i][j] == DEAD) buf[i - 1][j - 1] = rule2(matrix, i, j);
        }
    }

    for(int i = 1; i <= matrix->ysize; i++) {
        for(int j = 1; j <= matrix->xsize; j++) {
            matrix->table[i][j] = buf[i - 1][j - 1];
        }
    }
}

LIFE rule1(Matrix* matrix, int y, int x) {
    int cnt = 0;

    // 살아있는 경우 이웃 세포 탐색
    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            
            // 자기 자신을 가리키는 경우 스킵
            if(i == 0 && j == 0) continue;

            // 만약에 이웃 세포가 살아있으면 살아있는 세포 수에 1 추가
            if(matrix->table[y + i][x + j] == LIVE) cnt++;
        }
    }

    // 살아있는 이웃이 0 ~ 2, 7 ~ 8개면 해당 세포를 죽이고 
    // 살아있는 이웃이 3 ~ 6개면 해당 세포를 살린다.
    return cnt >= 7 || cnt <= 2 ? DEAD : LIVE;
}

LIFE rule2(Matrix* matrix, int y, int x) {
    int cnt = 0;

    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            // 자기 자신을 가리키는 경우 스킵
            if(i == 0 && j == 0) continue;
            
            // 만약에 이웃 세포가 살아있으면 살아있는 세포 수에 1 추가
            if(matrix->table[y + i][x + j] == LIVE) cnt++;
        }
    }

    return cnt == 4 ? LIVE : DEAD;
}

int getIntegerCipher(int input) {
    int i = 0;
    while(input != 0) {
        input /= 10;
        i++;
    }
    return i;
}