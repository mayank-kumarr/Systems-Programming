#include "memlab.h"
using namespace std;

char *MemSeg, *MemSegEnd;
long int MemSize;
long int nfree = 0;
int *MemStatus;
PageTable *PT;
PageTable *glbPT;
PageTable *prtPT;
struct Stack ST;
pthread_mutex_t memLock;

int medium_int::toInt() const
{
    return ((int)(this->num[0]) << 16) + ((int)(this->num[1]) << 8) + ((int)(this->num[2]));
}

medium_int::medium_int(int n)
{
    int r = (1 << 8) - 1;
    if (n >= (1 << 24))
    {
        cout << "Overflow: Value contains more than 3 bytes" << endl;
        exit(-1);
    }
    this->num[2] = (char)(n & r);
    n = n >> 8;
    this->num[1] = (char)(n & r);
    n = n >> 8;
    this->num[0] = (char)(n & r);
}

medium_int::medium_int(const medium_int &m)
{
    this->num[0] = m.num[0];
    this->num[1] = m.num[1];
    this->num[2] = m.num[2];
}

medium_int::~medium_int()
{
}

medium_int medium_int::operator+(const medium_int &m)
{
    medium_int *res = new medium_int(this->toInt() + m.toInt());
    return *res;
}

medium_int medium_int::operator-(const medium_int &m)
{
    medium_int *res = new medium_int(this->toInt() - m.toInt());
    return *res;
}

medium_int medium_int::operator*(const medium_int &m)
{
    medium_int *res = new medium_int(this->toInt() * m.toInt());
    return *res;
}

ostream &operator<<(ostream &os, const medium_int &m)
{
    os << m.toInt();
    return os;
}

PageTableEntry::PageTableEntry()
{
}

PageTableEntry::PageTableEntry(long int _offset, int _dtype, string _name, int _szarr, bool _valid)
{
    this->offset = _offset;
    this->dtype = _dtype;
    this->name = _name;
    this->szarr = _szarr;
    this->valid = _valid;
}

PageTableEntry::PageTableEntry(const PageTableEntry &p)
{
    this->offset = p.offset;
    this->dtype = p.dtype;
    this->name = p.name;
    this->szarr = p.szarr;
    this->valid = p.valid;
}

PageTable::PageTable(PageTable *_parent, long int _max_size)
{
    if (this->pte == nullptr)
    {
        cout << "Error in allocating memory for Page Table" << endl;
        exit(-1);
    }
    this->max_size = _max_size;
    this->parent = _parent;
    this->entry_count = 0;
}

PageTableEntry *PageTable::lookup(string _name)
{
    for (int i = 0; i < this->entry_count; i++)
        if ((this->pte[i]).name == _name && (this->pte[i]).valid == true)
            return &(this->pte[i]);
    PageTableEntry *ptr = nullptr;
    if (this->parent)
        ptr = this->parent->lookup(_name);
    return ptr;
}

int PageTable::insertEntry(const PageTableEntry &p)
{
    for (int i = 0; i < this->entry_count; i++)
    {
        if ((this->pte[i]).valid == false)
        {
            this->pte[i] = p;
            ST.push(i);
            return 1;
        }
    }
    if (this->entry_count <= this->max_size)
    {
        this->pte[this->entry_count] = p;
        ST.push(this->entry_count);
        this->entry_count++;
        return 1;
    }

    return 0;
}

void PageTable::printTable()
{
    cout << "__________________________________________________________________________________________________________________________________";
    cout << endl;
    cout << "Name                              ";
    cout << "Data Type                ";
    cout << "Array Size           ";
    cout << "Offset         ";
    cout << "Validity";
    cout << endl;

    for (int i = 0; i < this->entry_count; i++)
    {
        cout << (this->pte[i]).name;
        for (int j = 0; j < (35 - (this->pte[i]).name.length()); j++)
            cout << " ";
        string _dtype;
        int type = (this->pte[i]).dtype;
        if (type == INT)
            _dtype = "int";
        else if (type == CHAR)
            _dtype = "char";
        else if (type == MEDIUM_INT)
            _dtype = "medium int";
        else if (type == BOOLEAN)
            _dtype = "boolean";
        cout << _dtype;
        for (int j = 0; j < (28 - _dtype.length()); j++)
            cout << " ";
        cout << (this->pte[i]).szarr;
        for (int j = 0; j < (20 - to_string((this->pte[i]).szarr).length()); j++)
            cout << " ";
        cout << (this->pte[i]).offset;
        for (int j = 0; j < (15 - to_string((this->pte[i]).offset).length()); j++)
            cout << " ";
        cout << (this->pte[i]).valid << endl;
    }

    for (int i = 0; i < 65; i++)
        cout << "__";
    cout << "\n\n";
    return;
}

pair<long int, long int> get_qr(long int offset)
{
    pair<long int, long int> pr;
    offset /= 4;
    pr.first = offset / 32;
    pr.second = offset % 32;
    return pr;
}

bool mem_status(long int offset)
{
    pair<long int, long int> pr = get_qr(offset);
    if (MemStatus[pr.first] & (1 << pr.second))
        return true;
    else
        return false;
}

void mem_set(long int offset)
{
    pair<long int, long int> pr = get_qr(offset);
    MemStatus[pr.first] = MemStatus[pr.first] | (1 << pr.second);
    return;
}

void mem_free(long int offset)
{
    pair<long int, long int> pr = get_qr(offset);
    MemStatus[pr.first] = MemStatus[pr.first] & (~(1 << pr.second));
    return;
}

long int bestFitOffset(int szarr)
{
    long int i = 0, j, k;
    long int offset = -1;
    long int count, min_count = MemSize;
    while (i < MemSize)
    {
        j = szarr, k = i;
        count = 0;
        while (i < MemSize && !mem_status(i))
        {
            i += 4;
            if (j != 0)
                j--;
            else
                count++;
        }
        if (j == 0 && count < min_count)
        {
            offset = k;
            min_count = count;
        }
        i += 4;
    }
    return offset;
}

int createMem(long int num_bytes)
{
    MemSize = num_bytes;
    if (pthread_mutex_init(&memLock, NULL) != 0)
        perror("Mutex init failed");
    pthread_mutex_lock(&memLock);
    MemSeg = (char *)malloc(MemSize);
    MemSegEnd = MemSeg + MemSize;
    if (MemSeg != nullptr)
    {
        cout << "Memory Segment of " << MemSize << " bytes allocated successfully" << endl;
        glbPT = new PageTable(nullptr, MemSize / SZPAGE);
        PT = glbPT;
        prtPT = nullptr;
        long int MSS = (long int)ceil(MemSize / 128.0);
        MemStatus = (int *)calloc(MSS, sizeof(int));
        if (MemStatus != nullptr)
        {
            cout << "Memory allocated successfully for status array" << endl;
            pthread_mutex_unlock(&memLock);
            return 1;
        }
        perror("Memory allocation failed for status array");
        pthread_mutex_unlock(&memLock);
        return 0;
    }
    perror("Memory Segment allocation failed");
    pthread_mutex_unlock(&memLock);
    return 0;
}

int createVar(string name, int dtype)
{
    pthread_mutex_lock(&memLock);
    long int available_offset = bestFitOffset(1);
    if (available_offset == -1)
    {
        cout << "No sufficiently big free segment found" << endl;
        pthread_mutex_unlock(&memLock);
        return 0;
    }
    PageTableEntry var(available_offset, dtype, name, 1);
    if (PT->insertEntry(var))
    {
        mem_set(available_offset);
        cout << "Page Table Entry for " << name << " inserted in the current Page Table" << endl;
        pthread_mutex_unlock(&memLock);
        return 1;
    }
    else
    {
        cout << "Page Table size exceeded! Failed to insert " << name << " in the current Page Table" << endl;
        pthread_mutex_unlock(&memLock);
        return 0;
    }
}

bool checkAssignVar(PageTableEntry *var, int dt)
{
    if (var == nullptr)
    {
        cout << "Variable not found" << endl;
        return false;
    }
    if (var->dtype != dt)
    {
        cout << "Variable data type does not match with the value provided" << endl;
        return false;
    }
    return true;
}

bool checkAssignArr(PageTableEntry *var, int index, int dt)
{
    if (var == nullptr)
    {
        cout << "Array not found" << endl;
        return false;
    }
    if (var->dtype != dt)
    {
        cout << "Array data type does not match with the value provided" << endl;
        return false;
    }
    if (index < -1 || index >= var->szarr)
    {
        cout << "Array index out of bounds" << endl;
        return false;
    }
    return true;
}

bool checkRead(PageTableEntry *var, int index, int dt)
{
    if (var == nullptr)
    {
        cout << "Variable not found" << endl;
        return false;
    }
    if (var->dtype != dt)
    {
        cout << "Variable is not of given type" << endl;
        return false;
    }
    if (index < 0 && index >= var->szarr)
    {
        cout << "Array index out of bounds" << endl;
        return false;
    }
    return true;
}

int assignVar(string name, int val)
{
    PageTableEntry *var = PT->lookup(name);
    if (!checkAssignVar(var, INT))
        return 0;
    pthread_mutex_lock(&memLock);
    int *temp = (int *)(((unsigned char *)MemSeg) + var->offset);
    *temp = val;
    cout << "Variable " << name << " of int type assigned value " << val << " at local address " << var->offset << endl;
    pthread_mutex_unlock(&memLock);
    return 1;
}

int assignVar(string name, char val)
{
    PageTableEntry *var = PT->lookup(name);
    if (!checkAssignVar(var, CHAR))
        return 0;
    pthread_mutex_lock(&memLock);
    char *temp = (char *)(((unsigned char *)MemSeg) + var->offset);
    *temp = val;
    cout << "Variable " << name << " of char type assigned value " << val << " at local address " << var->offset << endl;
    pthread_mutex_unlock(&memLock);
    return 1;
}

int assignVar(string name, const medium_int &val)
{
    PageTableEntry *var = PT->lookup(name);
    if (!checkAssignVar(var, MEDIUM_INT))
        return 0;
    pthread_mutex_lock(&memLock);
    medium_int *temp = (medium_int *)(((unsigned char *)MemSeg) + var->offset);
    *temp = val;
    cout << "Variable " << name << " of medium int type assigned value " << val << " at local address " << var->offset << endl;
    pthread_mutex_unlock(&memLock);
    return 1;
}

int assignVar(string name, bool val)
{
    PageTableEntry *var = PT->lookup(name);
    if (!checkAssignVar(var, BOOLEAN))
        return 0;
    pthread_mutex_lock(&memLock);
    bool *temp = (bool *)(((unsigned char *)MemSeg) + var->offset);
    *temp = val;
    cout << "Variable " << name << " of boolean type assigned value " << val << " at local address " << var->offset << endl;
    pthread_mutex_unlock(&memLock);
    return 1;
}

int createArr(string name, int dtype, int szarr)
{
    pthread_mutex_lock(&memLock);
    long int available_offset = bestFitOffset(szarr);
    if (available_offset == -1)
    {
        cout << "No sufficiently big free segment found" << endl;
        pthread_mutex_unlock(&memLock);
        return 0;
    }
    PageTableEntry var(available_offset, dtype, name, szarr);
    if (PT->insertEntry(var))
    {
        for (int i = 0; i < szarr; i++)
            mem_set(available_offset + 4 * i);
        cout << "Page Table Entry corresponding to array " << name << " inserted in the current Page Table" << endl;
        pthread_mutex_unlock(&memLock);
        return 1;
    }
    else
    {
        cout << "Page Table size exceeded! Failed to insert array in the current Page Table" << endl;
        pthread_mutex_unlock(&memLock);
        return 0;
    }
}

int assignArr(string name, int value, int index)
{
    PageTableEntry *var = PT->lookup(name);
    if (!checkAssignArr(var, index, INT))
        return 0;
    pthread_mutex_lock(&memLock);
    if (index == -1)
    {
        for (int i = 0; i < var->szarr; i++)
        {
            int *temp = (int *)(((unsigned char *)MemSeg) + var->offset + 4 * i);
            *temp = value;
        }
        cout << "Array " << name << " assigned value " << value << " for " << var->szarr << " elements, starting at local address " << var->offset << endl;
    }
    else
    {
        int *temp = (int *)(((unsigned char *)MemSeg) + var->offset + 4 * index);
        *temp = value;
        cout << "Array " << name << " assigned value " << value << " at index " << index << endl;
    }
    pthread_mutex_unlock(&memLock);
    return 1;
}

int assignArr(string name, char value, int index)
{
    PageTableEntry *var = PT->lookup(name);
    if (!checkAssignArr(var, index, CHAR))
        return 0;
    pthread_mutex_lock(&memLock);
    if (index == -1)
    {
        for (int i = 0; i < var->szarr; i++)
        {
            char *temp = (char *)(((unsigned char *)MemSeg) + var->offset + 4 * i);
            *temp = value;
        }
        cout << "Array " << name << " assigned value " << value << " for " << var->szarr << " elements, starting at local address " << var->offset << endl;
    }
    else
    {
        char *temp = (char *)(((unsigned char *)MemSeg) + var->offset + 4 * index);
        *temp = value;
        cout << "Array " << name << " assigned value " << value << " at index " << index << endl;
    }
    pthread_mutex_unlock(&memLock);
    return 1;
}

int assignArr(string name, const medium_int &value, int index)
{
    PageTableEntry *var = PT->lookup(name);
    if (!checkAssignArr(var, index, MEDIUM_INT))
        return 0;
    pthread_mutex_lock(&memLock);
    if (index == -1)
    {
        for (int i = 0; i < var->szarr; i++)
        {
            medium_int *temp = (medium_int *)(((unsigned char *)MemSeg) + var->offset + 4 * i);
            *temp = value;
        }
        cout << "Array " << name << " assigned value " << value << " for " << var->szarr << " elements, starting at local address " << var->offset << endl;
    }
    else
    {
        medium_int *temp = (medium_int *)(((unsigned char *)MemSeg) + var->offset + 4 * index);
        *temp = value;
        cout << "Array " << name << " assigned value " << value << " at index " << index << endl;
    }
    pthread_mutex_unlock(&memLock);
    return 1;
}

int assignArr(string name, bool value, int index)
{
    PageTableEntry *var = PT->lookup(name);
    if (!checkAssignArr(var, index, BOOLEAN))
        return 0;
    pthread_mutex_lock(&memLock);
    if (index == -1)
    {
        for (int i = 0; i < var->szarr; i++)
        {
            bool *temp = (bool *)(((unsigned char *)MemSeg) + var->offset + 4 * i);
            *temp = value;
        }
        cout << "Array " << name << " assigned value " << value << " for " << var->szarr << " elements, starting at local address " << var->offset << endl;
    }
    else
    {
        bool *temp = (bool *)(((unsigned char *)MemSeg) + var->offset + 4 * index);
        *temp = value;
        cout << "Array " << name << " assigned value " << value << " at index " << index << endl;
    }
    pthread_mutex_unlock(&memLock);
    return 1;
}

int freeElem(string name)
{
    PageTableEntry *var = PT->lookup(name);
    if (var == nullptr)
    {
        cout << "Variable " << name << " not created! Cannot be freed" << endl;
        return 0;
    }
    pthread_mutex_lock(&memLock);
    nfree += var->szarr;
    for (int i = 0; i < var->szarr; i++)
        mem_free(var->offset + 4 * i);
    var->valid = false;
    cout << "Variable " << name << " freed from memory segment" << endl;
    pthread_mutex_unlock(&memLock);
    return 1;
}

int freeElem(int idx)
{
    PageTableEntry &var = PT->pte[idx];
    pthread_mutex_lock(&memLock);
    nfree += var.szarr;
    for (int i = 0; i < var.szarr; i++)
        mem_free(var.offset + 4 * i);
    var.valid = false;
    cout << "Variable " << var.name << " freed from memory segment" << endl;
    pthread_mutex_unlock(&memLock);
    return 1;
}

void printMemStatus(int n)
{
    for (int i = 0; i < n; i++)
        cout << mem_status(i);
    cout << endl;
}

int readInt(string name, int index)
{
    PageTableEntry *var = PT->lookup(name);
    if (!checkRead(var, index, INT))
        exit(-1);
    return *(int *)(MemSeg + var->offset + 4 * index);
}

char readChar(string name, int index)
{
    PageTableEntry *var = PT->lookup(name);
    if (!checkRead(var, index, CHAR))
        exit(-1);
    return *(char *)(MemSeg + var->offset + 4 * index);
}

medium_int readMediumInt(string name, int index)
{
    PageTableEntry *var = PT->lookup(name);
    if (!checkRead(var, index, MEDIUM_INT))
        exit(-1);
    return *(medium_int *)(MemSeg + var->offset + 4 * index);
}

bool readBool(string name, int index)
{
    PageTableEntry *var = PT->lookup(name);
    if (!checkRead(var, index, BOOLEAN))
        exit(-1);
    return *(bool *)(MemSeg + var->offset + 4 * index);
}

void initScope()
{
    pthread_mutex_lock(&memLock);
    ST.push(-1);
    pthread_mutex_unlock(&memLock);
}

void mark()
{
    pthread_mutex_lock(&memLock);
    cout << "Mark and Sweep: Marking..." << endl;
    int x = ST.pop();
    while (x != -1)
    {
        if (PT->pte[x].valid)
        {
            cout << "Variable " << PT->pte[x].name << " is now marked true" << endl;
            PT->pte[x].marked = true;
        }
        x = ST.pop();
    }
    cout << "Mark and Sweep: Marking Completed" << endl;
    pthread_mutex_unlock(&memLock);
}

void sweep()
{
    // pthread_mutex_lock(&memLock);
    cout << "Mark and Sweep: Sweeping..." << endl;
    for (int i = 0; i < PT->entry_count; i++)
    {
        if (PT->pte[i].valid && PT->pte[i].marked)
            freeElem(i);
    }
    cout << "Mark and Sweep: Sweeping Completed" << endl;
    // pthread_mutex_unlock(&memLock);
}

void endScope()
{
    mark();
    sweep();
}

void gc_compaction()
{
    pthread_mutex_lock(&memLock);
    cout << "GC Compaction..." << endl;
    for (int i = 0; i < MAX_SIZE; i++)
    {
        if (PT->pte[i].valid)
        {
            char *start = (char *)MemSeg;
            int count = 0;
            while (start < MemSeg + PT->pte[i].offset)
            {
                if (mem_status(start - (char *)MemSeg) == 0)
                    count++;
                start += 1;
            }
            if (count != 0) {
                cout << "Local address of " << PT->pte[i].name << " changed from " << PT->pte[i].offset << " to ";
                PT->pte[i].offset -= count;
                cout << PT->pte[i].offset << endl;
            }
        }
    }
    char *start = (char *)MemSeg;
    char *end = (char *)MemSegEnd;
    char *curr = start;
    char *begin = nullptr;
    int size = -1;
    int free_size = 0;
    int occ_size = 0;
    while (curr < end)
    {
        if (begin == nullptr && mem_status(curr - start) == 0)
        {
            begin = curr;
            free_size++;
        }
        else if (begin != nullptr && mem_status(curr - start) == 0)
            free_size++;
        else if (mem_status(curr - start) == 1 && begin != nullptr)
        {
            occ_size = 0;
            char *begin2 = curr;
            while (curr < end && mem_status(curr - start) == 1)
            {
                occ_size++;
                curr++;
            }
            int xyz = occ_size / 4;
            while (xyz--)
            {
                memcpy(begin, begin2, 4);
                begin += 4;
                begin2 += 4;
            }
            memset(begin, 0, free_size);
        }
        curr = curr + 1;
    }
    memset(MemStatus, 0, MemSize);
    for (int i = 0; i < MAX_SIZE; i++)
    {
        if (PT->pte[i].valid == 1)
        {
            for (int j = 0; j < PT->pte[i].szarr * 4; j++)
                mem_set(PT->pte[i].offset + j);
        }
    }
    cout << "GC Compaction Done" << endl;
    pthread_mutex_unlock(&memLock);
}

void *gc_run(void *arg)
{
    while (1)
    {
        usleep(15000);
        if (nfree > MemSize/5000) {
            gc_compaction();
            nfree = 0;
        }
    }
}

void gc_initialize()
{
    pthread_t garbage_thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int *arg = (int *)malloc(sizeof(*arg));
    *arg = 1;
    pthread_create(&garbage_thread, &attr, gc_run, arg);
}

void cleanExit()
{
    exit(1);
}