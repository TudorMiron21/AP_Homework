#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <string.h>

#define OFF 0
#define ON 1
#define DYING 2

int **matrix = NULL;
int ***cube = NULL;
int **copy_marix = NULL;
int ***copy_cube = NULL;
int D;
int H;
int W;
int L;
int S;
int N; // no of elements
int P; // no threads

pthread_barrier_t barrier;

int **aux_matrix = NULL;
int ***aux_cube = NULL;

void allocMatrix(int Length, int Width)
{
    matrix = (int **)malloc(Length * sizeof(int *));
    copy_marix = (int **)malloc(Length * sizeof(int *));
    if (matrix == NULL || copy_marix == NULL)
    {
        printf("Alocare esuata pentru vectorul de pointeri la randuri");
        exit(1);
    }

    for (int i = 0; i < Length; i++)
    {
        matrix[i] = (int *)malloc(Width * sizeof(int)); // Alocati spatiu pentru fiecare rand
        copy_marix[i] = (int *)malloc(Width * sizeof(int));
        if (matrix[i] == NULL || copy_marix[i] == NULL)
        {
            printf("Alocare esuata pentru randul %d", i);
            exit(1);
        }
    }
}

void allocCube(int Length, int Width, int Height)
{
    cube = (int ***)malloc(Length * sizeof(int **)); // Alocati spatiu pentru vectorul de pointeri la planuri (matrice bidimensionale)
    copy_cube = (int ***)malloc(Length * sizeof(int **));
    if (cube == NULL || copy_cube == NULL)
    {
        printf("Alocare esuata pentru vectorul de pointeri la planuri");
        exit(1);
    }

    for (int i = 0; i < Length; i++)
    {
        cube[i] = (int **)malloc(Width * sizeof(int *));      // Alocati spatiu pentru fiecare plan (matrice bidimensionala)
        copy_cube[i] = (int **)malloc(Width * sizeof(int *)); // Alocati spatiu pentru fiecare plan (matrice bidimensionala)

        if (cube[i] == NULL)
        {
            printf("Alocare esuata pentru planul %d", i);
            exit(1);
        }
    }

    for (int i = 0; i < Length; i++)
    {
        for (int j = 0; j < Width; j++)
        {
            cube[i][j] = (int *)malloc(Height * sizeof(int));      // Alocati spatiu pentru fiecare coloana (vector unidimensional)
            copy_cube[i][j] = (int *)malloc(Height * sizeof(int)); // Alocati spatiu pentru fiecare coloana (vector unidimensional)

            if (cube[i][j] == NULL)
            {
                printf("Alocare esuata pentru coloana %d din planul %d", j, i);
                exit(1);
            }
        }
    }
}

void readContentFromFile(const char *fileName)
{
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        printf("File %s not found or cannot be opened.\n", fileName);
        exit(1);
    }
    fscanf(fp, "%d", &D);

    if (D == 2)
    {
        fscanf(fp, "%d", &L);
        fscanf(fp, "%d", &W);
        fscanf(fp, "%d", &S);
        allocMatrix(L, W);

        for (int i = 0; i < W; i++)
        {
            for (int j = 0; j < L; j++)
            {
                fscanf(fp, "%d", &matrix[j][i]); // interschimbare
            }
        }
    }
    else if (D == 3)
    {
        fscanf(fp, "%d", &L);
        fscanf(fp, "%d", &W);
        fscanf(fp, "%d", &H);

        fscanf(fp, "%d", &S);
        allocCube(L, W, H);

        for (int i = 0; i < H; i++)
        {
            for (int j = 0; j < W; j++)
            {
                for (int k = 0; k < L; k++)
                    fscanf(fp, "%d", &cube[k][j][i]);
            }
        }
        return;
    }
    else
    {
        printf("error reading the dimension from file\n");
        exit(-1);
    }

    fclose(fp);
}

void *threadFunction(void *args)
{
    N = L;
    int thread_id = *(int *)args;
    int perThr = N / P;
    int forLastThread = N % P;
    int i, j, offset, nrVeciniON;
    int start = perThr * thread_id;
    int end = perThr * thread_id + perThr;
    if (thread_id == P - 1)
    {
        end = end + forLastThread;
    }

    int count = S;
    while (count--)
    {
        if (D == 2)
        {
            for (int i = start; i < end; i++)
            {
                for (int j = 0; j < W; j++)
                {
                    if (matrix[i][j] == OFF)
                    {
                        int countNeighboursON = 0;

                        // Check the eight possible neighbor positions
                        if (i - 1 >= 0 && j - 1 >= 0)
                            if (matrix[i - 1][j - 1] == ON)
                                countNeighboursON++;

                        if (i - 1 >= 0)
                            if (matrix[i - 1][j] == ON)
                                countNeighboursON++;

                        if (i - 1 >= 0 && j + 1 < W)
                            if (matrix[i - 1][j + 1] == ON)
                                countNeighboursON++;

                        if (j - 1 >= 0)
                            if (matrix[i][j - 1] == ON)
                                countNeighboursON++;

                        if (j + 1 < W)
                            if (matrix[i][j + 1] == ON)
                                countNeighboursON++;

                        if (i + 1 < L && j - 1 >= 0)
                            if (matrix[i + 1][j - 1] == ON)
                                countNeighboursON++;

                        if (i + 1 < L)
                            if (matrix[i + 1][j] == ON)
                                countNeighboursON++;

                        if (i + 1 < L && j + 1 < W)
                            if (matrix[i + 1][j + 1] == ON)
                                countNeighboursON++;

                        if (countNeighboursON == 2)
                            copy_marix[i][j] = ON;
                    }
                    if (matrix[i][j] == ON)
                    {
                        copy_marix[i][j] = DYING;
                    }
                    if (matrix[i][j] == DYING)
                    {
                        copy_marix[i][j] = OFF;
                    }
                }
            }
            pthread_barrier_wait(&barrier);
            if (thread_id == 0)
            {
                for (int i = 0; i < L; i++)
                {
                    memcpy(matrix[i], copy_marix[i], W * sizeof(int));
                }
            }
            pthread_barrier_wait(&barrier);
        }
        else
        {
            for (int i = start; i < end; i++)
            {
                for (int j = 0; j < W; j++)
                {
                    for (int k = 0; k < H; k++)
                    {
                        if (cube[i][j][k] == OFF)
                        {
                            int countNeighboursON = 0;

                            if (i - 1 >= 0 && j - 1 >= 0 && k - 1 >= 0 && cube[i - 1][j - 1][k - 1] == ON)
                                countNeighboursON++;
                            if (i - 1 >= 0 && j - 1 >= 0 && cube[i - 1][j - 1][k] == ON)
                                countNeighboursON++;
                            if (i - 1 >= 0 && j - 1 >= 0 && k + 1 < H && cube[i - 1][j - 1][k + 1] == ON)
                                countNeighboursON++;

                            if (i - 1 >= 0 && k - 1 >= 0 && cube[i - 1][j][k - 1] == ON)
                                countNeighboursON++;
                            if (i - 1 >= 0 && cube[i - 1][j][k] == ON)
                                countNeighboursON++;
                            if (i - 1 >= 0 && k + 1 < H && cube[i - 1][j][k + 1] == ON)
                                countNeighboursON++;

                            if (i - 1 >= 0 && j + 1 < W && k - 1 >= 0 && cube[i - 1][j + 1][k - 1] == ON)
                                countNeighboursON++;
                            if (i - 1 >= 0 && j + 1 < W && cube[i - 1][j + 1][k] == ON)
                                countNeighboursON++;
                            if (i - 1 >= 0 && j + 1 < W && k + 1 < H && cube[i - 1][j + 1][k + 1] == ON)
                                countNeighboursON++;

                            if (j - 1 >= 0 && k - 1 >= 0 && cube[i][j - 1][k - 1] == ON)
                                countNeighboursON++;
                            if (j - 1 >= 0 && cube[i][j - 1][k] == ON)
                                countNeighboursON++;
                            if (j - 1 >= 0 && k + 1 < H && cube[i][j - 1][k + 1] == ON)
                                countNeighboursON++;

                            if (j + 1 < W && k - 1 >= 0 && cube[i][j + 1][k - 1] == ON)
                                countNeighboursON++;
                            if (j + 1 < W && cube[i][j + 1][k] == ON)
                                countNeighboursON++;
                            if (j + 1 < W && k + 1 < H && cube[i][j + 1][k + 1] == ON)
                                countNeighboursON++;

                            if (k - 1 >= 0 && cube[i][j][k - 1] == ON)
                                countNeighboursON++;
                            if (k + 1 < H && cube[i][j][k + 1] == ON)
                                countNeighboursON++;

                            if (i + 1 < L && j - 1 >= 0 && k - 1 >= 0 && cube[i + 1][j - 1][k - 1] == ON)
                                countNeighboursON++;
                            if (i + 1 < L && j - 1 >= 0 && cube[i + 1][j - 1][k] == ON)
                                countNeighboursON++;
                            if (i + 1 < L && j - 1 >= 0 && k + 1 < H && cube[i + 1][j - 1][k + 1] == ON)
                                countNeighboursON++;

                            if (i + 1 < L && k - 1 >= 0 && cube[i + 1][j][k - 1] == ON)
                                countNeighboursON++;
                            if (i + 1 < L && cube[i + 1][j][k] == ON)
                                countNeighboursON++;
                            if (i + 1 < L && k + 1 < H && cube[i + 1][j][k + 1] == ON)
                                countNeighboursON++;

                            if (i + 1 < L && j + 1 < W && k - 1 >= 0 && cube[i + 1][j + 1][k - 1] == ON)
                                countNeighboursON++;
                            if (i + 1 < L && j + 1 < W && cube[i + 1][j + 1][k] == ON)
                                countNeighboursON++;
                            if (i + 1 < L && j + 1 < W && k + 1 < H && cube[i + 1][j + 1][k + 1] == ON)
                                countNeighboursON++;
                            if (countNeighboursON == 2)
                                copy_cube[i][j][k] = ON;
                        }
                        if (cube[i][j][k] == ON)
                        {
                            copy_cube[i][j][k] = DYING;
                        }
                        if (cube[i][j][k] == DYING)
                        {
                            copy_cube[i][j][k] = OFF;
                        }
                    }
                }
            }

            pthread_barrier_wait(&barrier);
            if (thread_id == 0)
            {

                for (int i = 0; i < L; i++)
                {
                    for (int j = 0; j < W; j++)
                    {
                        memcpy(cube[i][j], copy_cube[i][j], H * sizeof(int));
                    }
                }
            }
            pthread_barrier_wait(&barrier);
        }
    }
}

void writeMatrixToFile(const char *fileName)
{
    FILE *fp = fopen(fileName, "w");
    fprintf(fp, "%d %d %d\n", D, L, W);
    for (int i = 0; i < W; i++)
    {
        for (int j = 0; j < L; j++)
        {
            fprintf(fp, "%d ", matrix[j][i]);
        }
    }
}

void writeCubeToFile(const char *fileName)
{
    FILE *fp = fopen(fileName, "w");
    fprintf(fp, "%d %d %d %d\n", D, L, W, H);

    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            for (int k = 0; k < L; k++)
            {
                fprintf(fp, "%d ", cube[k][j][i]);
            }
        }
    }
}

int main(int argc, char **argv)
{

    readContentFromFile(argv[1]);
    P = atoi(argv[3]);
    pthread_barrier_init(&barrier, NULL, P);

    pthread_t tid[P];
    int thread_id[P];

    for (int i = 0; i < P; i++)
    {
        thread_id[i] = i;
    }

    for (int i = 0; i < P; i++)
    {
        pthread_create(&(tid[i]), NULL, threadFunction, &(thread_id[i]));
    }

    for (int i = 0; i < P; i++)
    {
        pthread_join(tid[i], NULL);
    }

    pthread_barrier_destroy(&barrier);

    if (D == 2)

        writeMatrixToFile(argv[2]);
    else if (D == 3)
    {
        writeCubeToFile(argv[2]);
    }
}
