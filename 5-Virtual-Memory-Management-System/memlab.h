#ifndef __MEMLAB_H // Control inclusion of header files
#define __MEMLAB_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <csignal>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>
#include <bits/stdc++.h>

using namespace std;

const int INT = -1;
const int CHAR = -2;
const int MEDIUM_INT = -3;
const int BOOLEAN = -4;

const int SZINT = 4;
const int SZCHAR = 1;
const int SZMEDIUM_INT = 3;

const int SZPAGE = 4;

#define MAX_STACK_SIZE 1024
#define MAX_SIZE 1024

class medium_int;
class PageTableEntry;
class PageTable;

class medium_int
{
private:
    char num[3];
    int toInt() const;

public:
    medium_int(int = 0);
    medium_int(const medium_int &);
    ~medium_int();
    medium_int operator+(const medium_int &);
    medium_int operator-(const medium_int &);
    medium_int operator*(const medium_int &);
    friend ostream &operator<<(ostream &, const medium_int &);
};

class PageTableEntry
{
public:
    long int offset;
    int dtype;
    string name;
    int szarr;
    bool valid;
    bool marked;
    PageTableEntry(long int, int, string, int, bool=true);
    PageTableEntry(const PageTableEntry &);
    PageTableEntry();
};

class PageTable
{
public:
    PageTableEntry pte[MAX_SIZE];
    PageTable *parent;
    long int entry_count;
    long int max_size;
    PageTable(PageTable *_parent, long int _max_size);
    PageTableEntry *lookup(string _name);
    int insertEntry(const PageTableEntry &p);
    void printTable();
};
struct Stack
{
    int _elems[MAX_STACK_SIZE];
    int _top;
    Stack() : _top(-1) {}
    void push(int elem)
    {
        _elems[++_top] = elem;
        cout << "Stack Operation: " << elem << " pushed into stack" << endl;
    }
    int pop()
    {
        cout << "Stack Operation: " << _elems[_top] << " popped from stack" << endl;
        return _elems[_top--];
    }
    int top()
    {
        return _elems[_top];
    }
};

int freeElem(string name);
int freeElem(int idx);

int createMem(long int num_bytes);
int createVar(string name, int dtype);

int assignVar(string name, int value);
int assignVar(string name, char value);
int assignVar(string name, const medium_int &value);
int assignVar(string name, bool value);

int readInt(string name, int index = 0);
char readChar(string name, int index = 0);
medium_int readMediumInt(string name, int index = 0);
bool readBool(string name, int index = 0);

int createArr(string name, int dtype, int szarr);

int assignArr(string name, int value, int index = -1);
int assignArr(string name, char value, int index = -1);
int assignArr(string name, const medium_int &value, int index = -1);
int assignArr(string name, bool value, int index = -1);

void initScope();
void endScope();
void gc_initialize();
void gc_compaction();
void cleanExit();
void printMemStatus(int n);

#endif // __MEMLAB_H