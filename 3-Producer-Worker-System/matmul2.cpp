#include <bits/stdc++.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <semaphore.h>
using namespace std;
const int MAX_SIZE = 8;
const int N = 1000;
const int M = N/2;
pthread_mutex_t * mptr;
pthread_mutexattr_t matr;

typedef struct process_data
{
    int prod_num;
    int status;
    double matrix[N][N];
    int matID;
} ProcessData;

typedef struct total
{
    int job_created;
    int size;
    int status;
    bool assigned[MAX_SIZE];
    ProcessData pq[8];
    pthread_mutex_t mutex;
} node;

int main()
{
    int i, j, nw, np, m, k, nmat;
    cout << "Enter the number of producers: ";
    cin >> np;
    cout << "Enter the number of workers: ";
    cin >> nw;
    cout << "Enter the number of matrices: ";
    cin >> nmat;

    int shmid = shmget(IPC_PRIVATE, sizeof(total), 0666 | IPC_CREAT);
    node *total = (node*) shmat(shmid, (void*) 0, 0);
    total->job_created = 0;
    total->size = 0;
    total->status=0;
    for(int k=0;k<MAX_SIZE;k++)
        total->assigned[k]=0;

    mptr = &(total->mutex);
    int rtn;
    
   	if (rtn = pthread_mutexattr_init(&matr))
   	{
   	    fprintf(stderr,"pthreas_mutexattr_init: %s",strerror(rtn)),exit(1);
   	}

   	if (rtn = pthread_mutexattr_setpshared(&matr,PTHREAD_PROCESS_SHARED))
   	{
   	    fprintf(stderr,"pthread_mutexattr_setpshared %s",strerror(rtn)),exit(1);
   	}

   	if (rtn = pthread_mutex_init(mptr, &matr))
   	{
   	    fprintf(stderr,"pthread_mutex_init %s",strerror(rtn)), exit(1);
   	}

    time_t initial = time(NULL);

    for (i = 0; i < np; i++)
    {
        if (fork() == 0)
        {
            while (1)
            {
                ProcessData job;
                job.prod_num = i;
                job.status = 0;
                srand(time(0) * (getpid()));
                job.matID = rand() % 100000 + 1;
                for (int i = 0; i < N; i++)
                {
                    for (int j = 0; j < N; j++)
                    {
                        job.matrix[i][j] = rand() % 19 - 9;
                    }
                }
                usleep(rand() % 3000000);
               	pthread_mutex_lock(mptr);
                if ((total->job_created) < nmat && (int) total->size < 7)
                {
                    total->pq[total->size] = job;
                    (total->size) ++;
                    (total->job_created) ++;
                    cout << "Producer: " << i << " PID: " << getpid() << " Matrix ID: " << job.matID << " Status of matrix multiplication: " << job.status << " Jobs Created: " << total->job_created << endl;
                   	pthread_mutex_unlock(mptr);
                }
                else if ((int) total->size >= 7)
                {
                    cout << "Producer " << i << " Queue is full: Waiting" << endl;
                   	pthread_mutex_unlock(mptr);
                    usleep(1000000);
                }
                else
                {
                   	pthread_mutex_unlock(mptr);
                    cout << "Exited Producer " << i << endl;
                    exit(0);
                }
            }
        }
    }

    for (int i1 = 0; i1 < nw; i1++)
    {
        if (fork() == 0)
        {
            while (1)
            {
                usleep(rand() % 3000000);

               	pthread_mutex_lock(mptr);
                if ((int) total->size > 1 && total->assigned[total->status]==0)
                {
                    int work = i1, pid = getpid();
                    int r1,c1,c2;
                    double D[M][M];
                    if (total->pq[0].status == 0 && total->pq[1].status == 0)
                    {
                        total->assigned[0]=1;
                        total->pq[0].status = 1;
                        total->pq[1].status = 1;
                        cout << "Worker: " << work << " Matrix 1 id: " << total->pq[0].matID << " Producer number: " << total->pq[0].prod_num << " Matrix 2 id: " << total->pq[1].matID << " Producer number " << total->pq[1].prod_num << " Work done: Reading\n";
                        (total->pq)[(total->size)].status = 0;
                        (total->pq)[(total->size)].prod_num = work;
                        (total->pq)[(total->size)].matID = rand() % 100000 + 1;
                        cout << "Worker: " << work << " Matrix 1 id: " << total->pq[0].matID << " Block number: 00" << " Producer number: " << total->pq[0].prod_num << " Matrix 2 id: " << total->pq[1].matID << " Block number: 00" << " Producer number " << total->pq[1].prod_num << " Work done: Copying\n";
                        r1=0,c1=0,c2=0;
                        for (int i = r1; i < r1 + M; i++)
                        {
                            for (int j = c2; j < c2 + M; j++)
                            {
                                D[i][j] = 0;
                                for (int k = c1; k < c1 + M; k++)
                                {
                                    D[i][j] += total->pq[0].matrix[i][k] + total->pq[1].matrix[k][j];
                                }
                            }
                        }
                        for (int i = 0; i < M; i++)
                        {
                            for (int j = 0; j < M; j++)
                                (total->pq)[(total->size)].matrix[i][j] = D[i][j];
                        }
                        total->size++;
                        total->status = 1;
                    }
                    else if (total->status == 1)
                    {
                        total->assigned[1]=1;
                        cout << "Worker: " << work << " Matrix 1 id: " << total->pq[0].matID << " Block number: 00" << " Producer number: " << total->pq[0].prod_num << " Matrix 2 id: " << total->pq[1].matID << " Block number: 01" << " Producer number " << total->pq[1].prod_num << " Work done: Copying\n";
                        r1=0,c1=0,c2=M;
                        for (int i = r1; i < r1 + M; i++)
                        {
                            for (int j = c2; j < c2 + M; j++)
                            {
                                D[i][j] = 0;
                                for (int k = c1; k < c1 + M; k++)
                                {
                                    D[i][j] += total->pq[0].matrix[i][k] + total->pq[1].matrix[k][j];
                                }
                            }
                        }

                        for (int i = 0; i < M; i++)
                        {
                            for (int j = M; j < N; j++)
                                ((total->pq)[(total->size) - 1]).matrix[i][j] = D[i][j - M];
                        }
                        total->status = 2;
                    }
                    else if (total->status == 2)
                    {
                        total->assigned[2]=1;
                        cout << "Worker: " << work << " Matrix 1 id: " << total->pq[0].matID << " Block number: 10" << " Producer number: " << total->pq[0].prod_num << " Matrix 2 id: " << total->pq[1].matID << " Block number: 00" << " Producer number " << total->pq[1].prod_num << " Work done: Copying\n";
                        r1=M,c1=0,c2=0;
                        for (int i = r1; i < r1 + M; i++)
                        {
                            for (int j = c2; j < c2 + M; j++)
                            {
                                D[i][j] = 0;
                                for (int k = c1; k < c1 + M; k++)
                                {
                                    D[i][j] += total->pq[0].matrix[i][k] + total->pq[1].matrix[k][j];
                                }
                            }
                        }

                        for (int i = M; i < N; i++)
                        {
                            for (int j = 0; j < M; j++)
                                ((total->pq)[(total->size) - 1]).matrix[i][j] = D[i - M][j];
                        }
                        total->status = 3;
                    }
                    else if (total->status == 3)
                    {
                        total->assigned[3]=1;
                        cout << "Worker: " << work << " Matrix 1 id: " << total->pq[0].matID << " Block number: 10" << " Producer number: " << total->pq[0].prod_num << " Matrix 2 id: " << total->pq[1].matID << " Block number: 01" << " Producer number " << total->pq[1].prod_num << " Work done: Copying\n";
                        r1=M,c1=0,c2=M;
                        for (int i = r1; i < r1 + M; i++)
                        {
                            for (int j = c2; j < c2 + M; j++)
                            {
                                D[i][j] = 0;
                                for (int k = c1; k < c1 + M; k++)
                                {
                                    D[i][j] += total->pq[0].matrix[i][k] + total->pq[1].matrix[k][j];
                                }
                            }
                        }

                        for (int i = M; i < N; i++)
                        {
                            for (int j = M; j < N; j++)
                                ((total->pq)[(total->size) - 1]).matrix[i][j] = D[i - M][j - M];
                        }
                        total->status = 4;
                    }
                    else if (total->status == 4)
                    {
                        total->assigned[4]=1;
                        cout << "Worker: " << work << " Matrix 1 id: " << total->pq[0].matID << " Block number: 01" << " Producer number: " << total->pq[0].prod_num << " Matrix 2 id: " << total->pq[1].matID << " Block number: 10" << " Producer number " << total->pq[1].prod_num << " Work done: Copying\n";
                        r1=0,c1=M,c2=0;
                        for (int i = r1; i < r1 + M; i++)
                        {
                            for (int j = c2; j < c2 + M; j++)
                            {
                                D[i][j] = 0;
                                for (int k = c1; k < c1 + M; k++)
                                {
                                    D[i][j] += total->pq[0].matrix[i][k] + total->pq[1].matrix[k][j];
                                }
                            }
                        }

                        cout << "Worker: " << work << " Matrix 1 id: " << total->pq[0].matID << " Producer number: " << total->pq[0].prod_num << " Matrix 2 id: " << total->pq[1].matID << " Producer number " << total->pq[1].prod_num << " Work done: Adding D000 and D001\n";
                        for (int i = 0; i < M; i++)
                        {
                            for (int j = 0; j < M; j++)
                                ((total->pq)[(total->size) - 1]).matrix[i][j] += D[i][j];
                        }
                        total->status = 5;
                    }
                    else if (total->status == 5)
                    {
                        total->assigned[5]=1;
                        cout << "Worker: " << work << " Matrix 1 id: " << total->pq[0].matID << " Block number: 01" << " Producer number: " << total->pq[0].prod_num << " Matrix 2 id: " << total->pq[1].matID << " Block number: 11" << " Producer number " << total->pq[1].prod_num << " Work done: Copying\n";
                        r1=0,c1=M,c2=M;
                        for (int i = r1; i < r1 + M; i++)
                        {
                            for (int j = c2; j < c2 + M; j++)
                            {
                                D[i][j] = 0;
                                for (int k = c1; k < c1 + M; k++)
                                {
                                    D[i][j] += total->pq[0].matrix[i][k] + total->pq[1].matrix[k][j];
                                }
                            }
                        }

                        cout << "Worker: " << work << " Matrix 1 id: " << total->pq[0].matID << " Producer number: " << total->pq[0].prod_num << " Matrix 2 id: " << total->pq[1].matID << " Producer number " << total->pq[1].prod_num << " Work done: Adding D010 and D011\n";
                        for (int i = 0; i < M; i++)
                        {
                            for (int j = M; j < N; j++)
                                ((total->pq)[(total->size) - 1]).matrix[i][j] += D[i][j - M];
                        }
                        total->status = 6;
                    }
                    else if (total->status == 6)
                    {
                        total->assigned[6]=1;
                        cout << "Worker: " << work << " Matrix 1 id: " << total->pq[0].matID << " Block number: 11" << " Producer number: " << total->pq[0].prod_num << " Matrix 2 id: " << total->pq[1].matID << " Block number: 10" << " Producer number " << total->pq[1].prod_num << " Work done: Copying\n";
                        r1=M,c1=M,c2=0;
                        for (int i = r1; i < r1 + M; i++)
                        {
                            for (int j = c2; j < c2 + M; j++)
                            {
                                D[i][j] = 0;
                                for (int k = c1; k < c1 + M; k++)
                                {
                                    D[i][j] += total->pq[0].matrix[i][k] + total->pq[1].matrix[k][j];
                                }
                            }
                        }

                        cout << "Worker: " << work << " Matrix 1 id: " << total->pq[0].matID << " Producer number: " << total->pq[0].prod_num << " Matrix 2 id: " << total->pq[1].matID << " Producer number " << total->pq[1].prod_num << " Work done: Adding D100 and D101\n";
                        for (int i = M; i < N; i++)
                        {
                            for (int j = 0; j < M; j++)
                                ((total->pq)[(total->size) - 1]).matrix[i][j] += D[i - M][j];
                        }
                        total->status = 7;
                    }
                    else if (total->status == 7)
                    {
                        total->assigned[7]=1;
                        cout << "Worker: " << work << " Matrix 1 id: " << total->pq[0].matID << " Block number: 11" << " Producer number: " << total->pq[0].prod_num << " Matrix 2 id: " << total->pq[1].matID << " Block number: 11" << " Producer number " << total->pq[1].prod_num << " Work done: Copying\n";
                        r1=M,c1=M,c2=M;
                        for (int i = r1; i < r1 + M; i++)
                        {
                            for (int j = c2; j < c2 + M; j++)
                            {
                                D[i][j] = 0;
                                for (int k = c1; k < c1 + M; k++)
                                {
                                    D[i][j] += total->pq[0].matrix[i][k] + total->pq[1].matrix[k][j];
                                }
                            }
                        }

                        cout << "Worker: " << work << " Matrix 1 id: " << total->pq[0].matID << " Producer number: " << total->pq[0].prod_num << " Matrix 2 id: " << total->pq[1].matID << " Producer number " << total->pq[1].prod_num << " Work done: Adding D110 and D111\n";
                        for (int i = M; i < N; i++)
                        {
                            for (int j = M; j < N; j++)
                                ((total->pq)[(total->size) - 1]).matrix[i][j] += D[i - M][j - M];
                        }

                        total->pq[0].status = 2;
                        total->pq[1].status = 2;
                        cout << "Popping out Matrix with id: " << total->pq[0].matID << " Producer number: " << total->pq[0].prod_num << endl;
                        cout << "Popping out Matrix with id: " << total->pq[1].matID << " Producer number: " << total->pq[1].prod_num << endl;
                        for (int xyz = 0; xyz < total->size - 2; xyz++)
                            (total->pq)[xyz] = (total->pq)[xyz + 2];
                        total->status = 0;
                        total->size -= 2;
                        for(int k=0;k<8;k++)
                            total->assigned[k]=0;

                    }
                   	pthread_mutex_unlock(mptr);
                    usleep(1000000);
                }
                else if (total->job_created < nmat && (int) total->size < 2)
                {
                   	pthread_mutex_unlock(mptr);
                    cout << "Worker: " << i1 << " Waiting since 8 workers already working on the process" << endl;
                    usleep(1000000);
                }
                else if ((int) total->size > 1 && total->assigned[total->status]==1)
                {
                    pthread_mutex_unlock(mptr);
                    cout << "Worker: " << i1 << " Waiting since 8 workers already working on the process" << endl;
                    usleep(1000000);
                }
                else
                {
                    cout << "Exited Worker " << i1 << endl;
                   	pthread_mutex_unlock(mptr);
                    exit(0);
                }
            }
        }
    }

    for (i = 0; i < nw + np; i++)
    {
        wait(NULL);
    }

    long long sum = 0;
    for (i = 0; i < N; i++)
    {
        sum += total->pq[0].matrix[i][i];
    }
    cout << "Sum of the diagonal elements of the final matrix is: " << sum << endl;

    time_t final = time(NULL);
    cout << "Total Time elapsed: " << (final - initial) << endl;
    shmdt(total);
    shmctl(shmid, IPC_RMID, NULL);
    exit(1);
}