#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

char* input;
char* output;
int num_threads;
int type, x, y, z=1, iter, N, P;
int *mat;
int *aux;
int *auxCasuta;

#define ON 1
#define OFF 0
#define DYING 2

pthread_barrier_t barrier;

void getArgs(int argc, char **argv)
{
    if (argc < 4)
    {
        printf("Not enough parameters: ./program INPUT_FILE OUTPUT_FILE NUM_THREADS\n");
        exit(1);
    }

    input = malloc(sizeof(char) * (strlen(argv[1]) + 1));
    output = malloc(sizeof(char) * (strlen(argv[2]) + 1));

    strcpy(input, argv[1]);
    strcpy(output, argv[2]);
    num_threads = atoi(argv[3]);
}

void writeMat()
{
    FILE* outfile;
    outfile = fopen(output, "w");
    if(type==2)
    {
        fprintf(outfile, "%d %d %d\n", type, x, y);
        int i,j;
        for (j = 0; j < y; j++)
        {
            for (i = 0; i < x; i++)
            {
                fprintf(outfile, "%d ", mat[i * y + j]);
            }
        }
    }
    else if(type==3)
    {
        fprintf(outfile, "%d %d %d %d\n", type, x, y, z);
        int i,j,k;
        for (k = 0; k < z; k++)
        {
            for (j = 0; j < y; j++)
            {
                for (i = 0; i < x; i++)
                {
                    fprintf(outfile, "%d ", mat[i * y * z + j * z + k]);
                }
            }
        }
    }

    fclose(outfile);
}

void readMat()
{
    FILE *infile;
    infile = fopen(input, "r");

    if (type == 2) 
    {
        fscanf(infile, "%d %d %d %d", &type, &x, &y, &iter);  

        mat = malloc(x * y * sizeof(int));

        int i, j;
        for (j = 0; j < y; j++)
        {
            for (i = 0; i < x; i++)
            {
                fscanf(infile, "%d", &mat[i * y + j]);
            }
        }
    }
    else if(type==3)
    {
        fscanf(infile, "%d %d %d %d %d", &type, &x, &y, &z, &iter);

        mat = (int *)malloc(x * y * z * sizeof(int));

        int i, j, k;
        for (k = 0; k < z; k++)
        {
            for (j = 0; j < y; j++)
            {
                for (i = 0; i < x; i++)
                {
                    fscanf(infile, "%d", &mat[i * y * z + j * z + k]);
                }
            }
        }
    }

    fclose(infile);
}

void *threadFunction2D(void *args)
{
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
    for(int k=0; k<iter; k++)
    {
        for (i = start; i < end; i++)
        {
            for (j = 0; j < y; j++)
            {
                offset = i * y + j;
                if(mat[offset]==OFF)
                {
                    int nrVeciniON=0;
                    //vecinul din stanga sus
                    if(i-1>=0 && j-1>=0 && mat[(i-1)*y+(j-1)]==ON)
                        nrVeciniON++;
                    //vecinul de sus
                    if(i-1>=0 && mat[(i-1)*y+j]==ON)
                        nrVeciniON++;
                    //vecinul din dreapta sus
                    if(i-1>=0 && j+1<y && mat[(i-1)*y+(j+1)]==ON)
                        nrVeciniON++;
                    //vecinul din stanga
                    if(j-1>=0 && mat[i*y+(j-1)]==ON)
                        nrVeciniON++;
                    //vecinul din dreapta
                    if(j+1<y && mat[i*y+(j+1)]==ON)
                        nrVeciniON++;
                    //vecinul din stanga jos
                    if(i+1<x && j-1>=0 && mat[(i+1)*y+(j-1)]==ON)
                        nrVeciniON++;
                    //vecinul de jos
                    if(i+1<x && mat[(i+1)*y+j]==ON)
                        nrVeciniON++;
                    //vecinul din dreapta jos
                    if(i+1<x && j+1<y && mat[(i+1)*y+(j+1)]==ON)
                        nrVeciniON++;
                    if(nrVeciniON==2)
                    {
                        auxCasuta[offset]=ON;
                    }
                    else
                    {
                        auxCasuta[offset]=OFF;
                    }
                }
                else if(mat[offset]==ON)
                {
                    auxCasuta[offset]=DYING;
                }
                else if(mat[offset]==DYING)
                {
                    auxCasuta[offset]=OFF;
                }
            }
        }
        pthread_barrier_wait(&barrier);
        if (thread_id == 0)
        {
            aux = mat;
            mat = auxCasuta;
            auxCasuta = aux;
        }
        pthread_barrier_wait(&barrier);
    }
}

void *threadFunction3D(void *args)
{
    int thread_id = *(int *)args;
    int perThr = N / P;
    int forLastThread = N % P;
    int i, j, l, offset, nrVeciniON;
    int start = perThr * thread_id;
    int end = perThr * thread_id + perThr;
    if (thread_id == P - 1)
    {
        end = end + forLastThread;
    }

    for (int k = 0; k < iter; k++)
    {
        for (i = start; i < end; i++)
        {
            for (j = 0; j < y; j++)
            {
                for (l = 0; l < z; l++)
                {
                    offset = i * y * z + j * z + l;

                    if (mat[offset] == OFF)
                    {
                        int nrVeciniON = 0;
                        if (i-1 >= 0 && j -1 >= 0 && l - 1 >= 0 && i -1 < x && j -1 < y && l -1 < z)
                        {
                            int offsetAux = (i -1) * y * z + (j -1) * z + (l -1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i-1 >= 0 && j -1 >= 0 && l >= 0 && i -1 < x && j -1 < y && l < z)
                        {
                            int offsetAux = (i -1) * y * z + (j -1) * z + l;
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i-1 >= 0 && j -1 >= 0 && l + 1 >= 0 && i -1 < x && j -1 < y && l + 1 < z)
                        {
                            int offsetAux = (i -1) * y * z + (j -1) * z + (l + 1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i-1 >= 0 && j >= 0 && l - 1 >= 0 && i -1 < x && j < y && l -1 < z)
                        {
                            int offsetAux = (i -1) * y * z + j * z + (l -1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i-1 >= 0 && j >= 0 && l >= 0 && i -1 < x && j < y && l < z)
                        {
                            int offsetAux = (i -1) * y * z + j * z + l;
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i-1 >= 0 && j >= 0 && l + 1 >= 0 && i -1 < x && j < y && l + 1 < z)
                        {
                            int offsetAux = (i -1) * y * z + j * z + (l + 1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i-1 >= 0 && j + 1 >= 0 && l - 1 >= 0 && i -1 < x && j + 1 < y && l -1 < z)
                        {
                            int offsetAux = (i -1) * y * z + (j + 1) * z + (l -1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i-1 >= 0 && j + 1 >= 0 && l >= 0 && i -1 < x && j + 1 < y && l < z)
                        {
                            int offsetAux = (i -1) * y * z + (j + 1) * z + l;
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i-1 >= 0 && j + 1 >= 0 && l + 1 >= 0 && i -1 < x && j + 1 < y && l + 1 < z)
                        {
                            int offsetAux = (i -1) * y * z + (j + 1) * z + (l + 1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i >= 0 && j -1 >= 0 && l - 1 >= 0 && i < x && j -1 < y && l -1 < z)
                        {
                            int offsetAux = i * y * z + (j -1) * z + (l -1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i >= 0 && j -1 >= 0 && l >= 0 && i < x && j -1 < y && l < z)
                        {
                            int offsetAux = i * y * z + (j -1) * z + l;
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i >= 0 && j -1 >= 0 && l + 1 >= 0 && i < x && j -1 < y && l + 1 < z)
                        {
                            int offsetAux = i * y * z + (j -1) * z + (l + 1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i >= 0 && j >= 0 && l - 1 >= 0 && i < x && j < y && l -1 < z)
                        {
                            int offsetAux = i * y * z + j * z + (l -1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i >= 0 && j >= 0 && l + 1 >= 0 && i < x && j < y && l + 1 < z)
                        {
                            int offsetAux = i * y * z + j * z + (l + 1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i >= 0 && j + 1 >= 0 && l - 1 >= 0 && i < x && j + 1 < y && l -1 < z)
                        {
                            int offsetAux = i * y * z + (j + 1) * z + (l -1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i >= 0 && j + 1 >= 0 && l >= 0 && i < x && j + 1 < y && l < z)
                        {
                            int offsetAux = i * y * z + (j + 1) * z + l;
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i >= 0 && j + 1 >= 0 && l + 1 >= 0 && i < x && j + 1 < y && l + 1 < z)
                        {
                            int offsetAux = i * y * z + (j + 1) * z + (l + 1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i + 1 >= 0 && j -1 >= 0 && l - 1 >= 0 && i + 1 < x && j -1 < y && l -1 < z)
                        {
                            int offsetAux = (i + 1) * y * z + (j -1) * z + (l -1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i + 1 >= 0 && j -1 >= 0 && l >= 0 && i + 1 < x && j -1 < y && l < z)
                        {
                            int offsetAux = (i + 1) * y * z + (j -1) * z + l;
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i + 1 >= 0 && j -1 >= 0 && l + 1 >= 0 && i + 1 < x && j -1 < y && l + 1 < z)
                        {
                            int offsetAux = (i + 1) * y * z + (j -1) * z + (l + 1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i + 1 >= 0 && j >= 0 && l - 1 >= 0 && i + 1 < x && j < y && l -1 < z)
                        {
                            int offsetAux = (i + 1) * y * z + j * z + (l -1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i + 1 >= 0 && j >= 0 && l >= 0 && i + 1 < x && j < y && l < z)
                        {
                            int offsetAux = (i + 1) * y * z + j * z + l;
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i + 1 >= 0 && j >= 0 && l + 1 >= 0 && i + 1 < x && j < y && l + 1 < z)
                        {
                            int offsetAux = (i + 1) * y * z + j * z + (l + 1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i + 1 >= 0 && j + 1 >= 0 && l - 1 >= 0 && i + 1 < x && j + 1 < y && l -1 < z)
                        {
                            int offsetAux = (i + 1) * y * z + (j + 1) * z + (l -1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i + 1 >= 0 && j + 1 >= 0 && l >= 0 && i + 1 < x && j + 1 < y && l < z)
                        {
                            int offsetAux = (i + 1) * y * z + (j + 1) * z + l;
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (i + 1 >= 0 && j + 1 >= 0 && l + 1 >= 0 && i + 1 < x && j + 1 < y && l + 1 < z)
                        {
                            int offsetAux = (i + 1) * y * z + (j + 1) * z + (l + 1);
                            if (mat[offsetAux] == ON)
                            {
                                nrVeciniON++;
                            }
                        }
                        if (nrVeciniON == 2)
                        {
                            auxCasuta[offset] = ON;
                        }
                        else
                        {
                            auxCasuta[offset] = OFF;
                        }
                    }
                    else if (mat[offset]==ON)
                    {
                        auxCasuta[offset]=DYING;
                    }
                    else if (mat[offset]==DYING)
                    {
                        auxCasuta[offset]=OFF;
                    }
                }
            }
        }
        pthread_barrier_wait(&barrier);
        if (thread_id == 0)
        {
            aux = mat;
            mat = auxCasuta;
            auxCasuta = aux;
        }
        pthread_barrier_wait(&barrier);
    } 
}


int main(int argc, char *argv[])
{
    int i;
    getArgs(argc, argv);

    FILE *infile;
    infile = fopen(input, "r");
    fscanf(infile, "%d", &type);
    fclose(infile);

    readMat();   
    auxCasuta = (int*)malloc(x * y * z * sizeof(int));
    N=x;
    aux=(int*)malloc(x * y * z * sizeof(int));
    P = num_threads;
    pthread_barrier_init(&barrier, NULL, P);

    pthread_t tid[P];
    int thread_id[P];

    for (i = 0; i < P; i++)
    {
        thread_id[i] = i;
    }

    for (i = 0; i < P; i++)
    {
        if (type == 2)
        {
            pthread_create(&(tid[i]), NULL, threadFunction2D, &(thread_id[i]));
        }
        else if (type == 3)
        {
            pthread_create(&(tid[i]), NULL, threadFunction3D, &(thread_id[i]));
        }
    }

    for (i = 0; i < P; i++)
    {
        pthread_join(tid[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
    
    writeMat();

    return 0;

}
