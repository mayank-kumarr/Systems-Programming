#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct _process_data {
double **A;
double **B;
double **C;
int veclen, i, j;
} ProcessData;

void *mult(void *arg)
{
    ProcessData* pd = (ProcessData*)arg;
    int i = pd->i;
    int j = pd->j;
    int c1 = pd->veclen;
    for(int k=0; k<c1; k++)
        pd->C[i][j] +=  pd->A[i][k] * pd->B[k][j];
}

int main()
{
    int r1, c1, r2, c2;
    printf("Enter r1, c1, r2 and c2\n");
    scanf("%d %d %d %d", &r1, &c1, &r2, &c2);
    if(c1 != r2)
    {
        printf("Matrix A and B cannot be multiplied\n");
        exit(EXIT_FAILURE);
    }

    size_t size = sizeof(double) * (r1 * c1 + r2 * c2 + r1 * c2) + sizeof(double*) * (r1 + r2 + r1);

    int shmid = shmget(IPC_PRIVATE, size, IPC_CREAT | 0644);
    
    if(shmid == -1)
    {
        perror("Error in shmget");
        exit(EXIT_FAILURE);
    }

    double* mem = (double*) (shmat(shmid, (void*) NULL, 0));
    double** mem2 = (double**) (mem + r1 * c1 + r2 * c2 + r1 * c2);

    for (int i = 0; i < r1; i++)
        mem2[i] = mem + i * c1;
    for (int i = 0; i < r2; i++)
        mem2[i + r1] = mem + r1 * c1 + i * c2;
    for (int i = 0; i < r1; i++)
        mem2[i + r1 + r2] = mem + r1 * c1 + r2 * c2 + i * c2;
    
    ProcessData pd;
    void* (*vfunc)(void *);
    vfunc = mult;

    pd.A = mem2;
    pd.B = mem2 + r1;
    pd.C = mem2 + r1 + r2;
    pd.veclen = c1;

    int i, j, k;
    double x;

    for(i=0; i<r1; i++)
    {
        for(j=0; j<c1; j++)
        {
            pd.A[i][j] = (double) rand()/RAND_MAX * 10;
        }
    }

    for(i=0; i<r2; i++)
    {
        for(j=0; j<c2; j++)
        {
            pd.B[i][j] = (double) rand()/RAND_MAX * 10;
        }
    }
    
    for(i=0; i<r1; i++)
    {
        for(j=0; j<c2; j++)
        {
            int pid = fork();
            if(pid == 0)
            {
                pd.i = i;
                pd.j = j;
                mult(&pd);
                exit(0);
            }
            else if(pid<0)
            {
                printf ("Error in fork(): %d - %s\n", errno, strerror(errno));
                exit(0);
            }
        }
    }

    for(i=0; i<r1*c2; i++)
        wait(NULL);
    
    printf("Resultant Matrix is:\n");
    for(i=0; i<r1; i++)
    {
        for(j=0; j<c2; j++)
        {
            printf("%.3lf ", pd.C[i][j]);
        }
        printf("\n");
    }

    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}