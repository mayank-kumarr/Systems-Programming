#include "memlab.h"
using namespace std;
using namespace std::chrono;

int fibonacciProduct(string name)
{
    initScope();
    int val = readInt(name);
    int arr = createArr("arr", -1, val);
    int a, b, i;
    for (i=0; i<val; i++)
    {
        if (i == 0)
            assignArr("arr", 1, i);
        else if (i == 1)
            assignArr("arr", 1, i);
        else
        {
            a = readInt("arr", i - 1);
            b = readInt("arr", i - 2);
            assignArr("arr", a + b, i);
        }
    }
    int prod = 1;
    for (i=0; i<val; i++)
    {
        a = readInt("arr", i);
        prod *= a;
    }
    endScope();
    return prod;
}

int main()
{
    int i, NUM = 10;
    int acarr[NUM];
    acarr[0] = 1;
    acarr[1] = 1;
    for (i=2; i<NUM; i++)
        acarr[i] = acarr[i-1] + acarr[i-2];
    int prod = 1;
    for (i=0; i<NUM; i++)
        prod *= acarr[i];
    cout << "Actual Product: " << prod << endl;
    createMem(250 * 1024 * 1024);
    gc_initialize();
    auto start = chrono::high_resolution_clock::now();
    initScope();
    int x = createVar("var1", -1);
    assignVar("var1", NUM);
    int val = fibonacciProduct("var1");
    endScope();
    cout << "Final Product: " << val << endl;
    auto end = chrono::high_resolution_clock::now();
    double time_taken = chrono::duration_cast<milliseconds>(end - start).count();
    cout<<"Time Taken is "<<time_taken<<" milliseconds"<<endl;
    cleanExit();
}