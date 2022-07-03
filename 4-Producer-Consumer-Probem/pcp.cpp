#include <pthread.h>
#include <sys/ipc.h>
#include <sys/resource.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cassert>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
using namespace std;
using namespace std::chrono;

#define PENDING 0
#define ONGOING 1
#define DONE 2

const int MAXC = 15;
const int MAXJID = 100000000;
const int MAXCT = 250;

int ran(int l, int r) {
    if (l > r) {
        return -1;
    }
    return l + (rand() % (r - l + 1));
}

struct Node {
    int job_id;
    int completion_time;
    int parent;
    int child[MAXC];
    int child_count;
    pthread_mutex_t mutex;
    int status;

    Node() {
        job_id = ran(1, MAXJID);
        completion_time = ran(0, MAXCT);
        for (int i = 0; i < MAXC; i++) {
            child[i] = -1;
        }
        child_count = 0;
        parent = -1;
        status = PENDING;
    }

    Node& operator=(const Node& node) {
        job_id = node.job_id;
        completion_time = node.completion_time;
        parent = node.parent;
        for (int i = 0; i < MAXC; i++) {
            child[i] = node.child[i];
        }
        child_count = node.child_count;
        status = node.status;
        return *this;
    }
};

struct SharedMemory {
    int size;
    pthread_mutex_t mutex;
    int count;
    int rootId;
    Node* nodes;
};

SharedMemory* shm;

void InitNode(Node& node) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&node.mutex, &attr);
    node.status = DONE;
}

void InitMem() {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shm->mutex, &attr);
    shm->count = 0;
    shm->rootId = -1;
    for (int i = 0; i < shm->size; i++) {
        InitNode(shm->nodes[i]);
    }
}

int add_child(int parentId, int childId) {
    if (shm->nodes[parentId].child_count == MAXC) return -1;
    for (int i = 0; i < MAXC; i++) {
        if (shm->nodes[parentId].child[i] == -1) {
            shm->nodes[parentId].child[i] = childId;
            shm->nodes[parentId].child_count++;
            return 0;
        }
    }
    return -1;
}

int remove_child(int parentId, int childId) {
    for (int i = 0; i < MAXC; i++) {
        if (shm->nodes[parentId].child[i] == childId) {
            shm->nodes[parentId].child[i] = -1;
            shm->nodes[parentId].child_count--;
            return 0;
        }
    }
    return -1;
}

int add_node(Node& node) {
    pthread_mutex_lock(&shm->mutex);
    if (shm->count < shm->size) {
        for (int i = 0; i < shm->size; i++) {
            if (shm->nodes[i].status == DONE) {
                pthread_mutex_lock(&shm->nodes[i].mutex);
                shm->nodes[i] = node;
                pthread_mutex_unlock(&shm->nodes[i].mutex);
                shm->count++;
                if (shm->rootId == -1) shm->rootId = i;
                pthread_mutex_unlock(&shm->mutex);
                return i;
            }
        }
    }
    pthread_mutex_unlock(&shm->mutex);
    return -1;
}

int remove_node(int id) {
    pthread_mutex_lock(&shm->mutex);
    pthread_mutex_lock(&shm->nodes[id].mutex);
    int parent = shm->nodes[id].parent;
    shm->nodes[id].status = DONE;
    pthread_mutex_unlock(&shm->nodes[id].mutex);
    shm->count--;
    pthread_mutex_unlock(&shm->mutex);
    if (parent == -1) {
        cout << "Root node removed!!\n";
        return 0;
    }
    pthread_mutex_lock(&shm->nodes[parent].mutex);
    assert(remove_child(parent, id) == 0);
    pthread_mutex_unlock(&shm->nodes[parent].mutex);
    return 0;
}

void createTree(int size) {
    Node root;
    add_node(root);
    for (int i = 1; i < size; i++) {
        Node node;
        int parent = ran(0, shm->count - 1);
        while (shm->nodes[parent].child_count == MAXC) {
            parent = ran(0, shm->count - 1);
        }
        node.parent = parent;
        int currId = add_node(node);
        add_child(parent, currId);
        printf("%d <-- %d\n", parent, currId);
    }
}

int getRandomJob() {
    int tries = ran(1, shm->count);
    for (int i = 0; i < tries; i++) {
        int p = ran(0, shm->count - 1);
        pthread_mutex_lock(&(shm->nodes[p].mutex));
        if (shm->nodes[p].status == PENDING && shm->nodes[p].child_count != MAXC) {
            return p;
        }
        pthread_mutex_unlock(&(shm->nodes[p].mutex));
    }
    return -1;
}

int getLeaf(int id) {
    pthread_mutex_lock(&(shm->nodes[id].mutex));
    if (shm->nodes[id].status == PENDING && shm->nodes[id].child_count == 0) {
        return id;
    }
    pthread_mutex_unlock(&(shm->nodes[id].mutex));
    for (int i = 0; i < MAXC; i++) {
        int childIdx = shm->nodes[id].child[i];
        if (childIdx != -1) {
            int leaf = getLeaf(childIdx);
            if (leaf != -1) {
                return leaf;
            }
        }
    }
    return -1;
}

void* producer_func(void* param) {
    int prod_time = ran(10, 20);
    int idx = *((int*)param);
    printf("Producer %d started\n", idx);
    auto start = high_resolution_clock::now();
    while (true) {
        auto curr = high_resolution_clock::now();
        auto duration = duration_cast<seconds>(curr - start);
        if (duration.count() >= prod_time) {
            printf("Producer %d finished\n", idx);
            pthread_exit(NULL);
        }
        int parentId = getRandomJob();
        if (parentId == -1) {
            usleep(10);
            continue;
        }
        Node node;
        node.parent = parentId;
        int currId = add_node(node);
        if (currId == -1) {
            pthread_mutex_unlock(&(shm->nodes[parentId].mutex));
            usleep(10);
            continue;
        }
        add_child(parentId, currId);
        printf("Producer %d added job: %d to parent job %d\n", idx, shm->nodes[currId].job_id, shm->nodes[parentId].job_id);
        pthread_mutex_unlock(&(shm->nodes[parentId].mutex));
        usleep(1000 * (ran(200, 500)));
    }
}

void* consumer_func(void* param) {
    int idx = *((int*)param);
    printf("Consumer %d started\n", idx);
    while (1) {
        pthread_mutex_lock(&(shm->nodes[shm->rootId].mutex));
        if (shm->nodes[shm->rootId].status != PENDING) {
            printf("Consumer %d finished\n", idx);
            pthread_mutex_unlock(&(shm->nodes[shm->rootId].mutex));
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&(shm->nodes[shm->rootId].mutex));
        int currId = getLeaf(shm->rootId);
        if (currId == -1) {
            usleep(10);
            continue;
        }
        shm->nodes[currId].status = ONGOING;
        int parent = shm->nodes[currId].parent;
        printf("Consumer %d started job: %d\n", idx, shm->nodes[currId].job_id);
        pthread_mutex_unlock(&(shm->nodes[currId].mutex));
        usleep(1000 * shm->nodes[currId].completion_time);
        printf("Consumer %d finished job: %d\n", idx, shm->nodes[currId].job_id);
        remove_node(currId);
    }
}

int main() {
    srand(time(0));
    int P, y;
    cout << "Enter number of producer threads: ";
    cin >> P;
    cout << "Enter number of consumer threads: ";
    cin >> y;

    int size = (500 + (100 * P));
    cout << "Size of shared memory: " << size << endl;
    int shmid;
    shmid = shmget(IPC_PRIVATE, sizeof(SharedMemory), IPC_CREAT | 0666);
    shm = (SharedMemory*)shmat(shmid, NULL, 0);
    int shmid2 = shmget(IPC_PRIVATE, size * sizeof(struct Node), IPC_CREAT | 0666);
    shm->nodes = (Node*)shmat(shmid2, NULL, 0);
    shm->size = size;

    InitMem();
    int init_size = ran(300, 500);
    cout << "Initial number of nodes = " << init_size << endl;

    createTree(init_size);

    pthread_t producer[P], consumer[y];

    for (int i = 0; i < P; i++) {
        int* id = new int(i);
        pthread_create(&producer[i], NULL, producer_func, id);
    }

    if (fork() == 0) {
        for (int i = 0; i < y; i++) {
            int* id = new int(i);
            pthread_create(&consumer[i], NULL, consumer_func, id);
        }
        for (int i = 0; i < y; i++) {
            pthread_join(consumer[i], NULL);
        }
        pthread_exit(NULL);
    }

    for (int i = 0; i < P; i++) {
        pthread_join(producer[i], NULL);
    }

    wait(NULL);

    for (int i = 0; i < shm->size; i++) {
        pthread_mutex_destroy(&(shm->nodes[i].mutex));
    }
    pthread_mutex_destroy(&(shm->mutex));
    shmdt(shm->nodes);
    shmdt(shm);
    shmctl(shmid2, IPC_RMID, NULL);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}