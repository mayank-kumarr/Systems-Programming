#include "memlab.h"
#include <chrono>
using namespace std;
using namespace std::chrono;

void func1(string x, string y)
{
    initScope();
    int val1 = readInt(x);
    int val2 = readInt(y);
    int val = 50000;
    int arr = createArr("arr1", -1, val);
    int a, i;
    for(i = 0; i < val; i++)
    {
        a = rand();
        assignArr("arr1", a , i);
    }
    freeElem("arr1");
    endScope();
}

void func2(string x, string y)
{
    initScope();
    char val1 = readChar(x);
    char val2 = readChar(y);
    int val = 50000;
    char arr = createArr("arr2", -2, val);
    char a;
    int i;
    for(i = 0; i < val; i++)
    {
        a = rand()%26 + 'a';
        assignArr("arr2", a , i);
    }
    freeElem("arr2");
    endScope();
}

void func3(string x, string y)
{
    initScope();
    medium_int val1 = readMediumInt(x);
    medium_int val2 = readMediumInt(y);
    int val = 50000;
    medium_int arr = createArr("arr3", -3, val);
    medium_int a; int i;
    for(i = 0; i < val; i++)
    {
        a = rand() % 16777215 - 8388608;
        assignArr("arr3", a , i);
    }
    freeElem("arr3");
    endScope();
}

void func4(string x, string y)
{
    initScope();
    bool val1 = readBool(x);
    bool val2 = readBool(y);
    int val = 50000;
    bool arr = createArr("arr4", -4, val);
    bool a;
    int i;
    for(i = 0; i < val; i++)
    {
        a = rand()%2;
        assignArr("arr4", a , i);
    }
    freeElem("arr4");
    endScope();
}

void func5(string x, string y)
{
    initScope();
    int val1 = readInt(x);
    int val2 = readInt(y);
    int val = 50000;
    int arr = createArr("arr5", -1, val);
    int a, i;
    for(i = 0; i < val; i++)
    {
        a = rand() - 2147483648;
        assignArr("arr5", a , i);
    }
    freeElem("arr5");
    endScope();
}

void func6(string x, string y)
{
    initScope();
    char val1 = readChar(x);
    char val2 = readChar(y);
    int val = 50000;
    char arr = createArr("arr6", -2, val);
    char a;
    int i;
    for(i = 0; i < val; i++)
    {
        a = rand()%26 + 'A';
        assignArr("arr6", a , i);
    }
    freeElem("arr6");
    endScope();
}

void func7(string x, string y)
{
    initScope();
    medium_int val1 = readMediumInt(x);
    medium_int val2 = readMediumInt(y);
    int val = 50000;
    medium_int arr = createArr("arr7", -3, val);
    medium_int a; int i;
    for(i = 0; i < val; i++)
    {
        a = rand() % 16777215;
        assignArr("arr7", a , i);
    }
    freeElem("arr7");
    endScope();
}

void func8(string x, string y)
{
    initScope();
    int val1 = readInt(x);
    int val2 = readInt(y);
    int val = 50000;
    int arr = createArr("arr8", -1, val);
    int a, i;
    for(i = 0; i < val; i++)
    {
        a = rand() % 1111111;
        assignArr("arr8", a , i);
    }
    freeElem("arr8");
    endScope();
}

void func9(string x, string y)
{
    initScope();
    char val1 = readChar(x);
    char val2 = readChar(y);
    int val = 50000;
    char arr = createArr("arr9", -2, val);
    char a;
    int i;
    for(i = 0; i < val; i++)
    {
        a = rand()%10 + '0';
        assignArr("arr9", a , i);
    }
    freeElem("arr9");
    endScope();
}

void func10(string x, string y)
{
    initScope();
    medium_int val1 = readMediumInt(x);
    medium_int val2 = readMediumInt(y);
    int val = 50000;
    medium_int arr = createArr("arr10", -3, val);
    medium_int a; int i;
    for(i = 0; i < val; i++)
    {
        a = rand() % 1111111;
        assignArr("arr10", a , i);
    }
    freeElem("arr10");
    endScope();
}



int main()
{
    srand(time(NULL));
    createMem(250 * 1024 * 1024);
    gc_initialize();

    int num1=250, num2=200;
    char ch1='a', ch2='A';
    medium_int num3=100, num4=101;
    bool b1=0, b2=1;
    
    auto start = chrono::high_resolution_clock::now();

    initScope();
    
    int x1 = createVar("var1", -1);
    assignVar("var1", num1);
    int y1 = createVar("var2", -1);
    assignVar("var2", num2);
    func1("var1","var2");

    char x2 = createVar("var3", -2);
    assignVar("var3", ch1);
    char y2 = createVar("var4", -2);
    assignVar("var4", ch2);
    func2("var3","var4");

    medium_int x3 = createVar("var5", -3);
    assignVar("var5", num3);
    medium_int y3 = createVar("var6", -3);
    assignVar("var6", num4);
    func3("var5","var6");

    bool x4 = createVar("var7", -4);
    assignVar("var7", b1);
    bool y4 = createVar("var8", -4);
    assignVar("var8", b2);
    func4("var7","var8");

    int x5 = createVar("var9", -1);
    assignVar("var9", num1);
    int y5 = createVar("var10", -1);
    assignVar("var10", num2);
    func5("var9","var10");

    char x6 = createVar("var11", -2);
    assignVar("var11", ch1);
    char y6 = createVar("var12", -2);
    assignVar("var12", ch2);
    func6("var11","var12");

    medium_int x7 = createVar("var13", -3);
    assignVar("var13", num3);
    medium_int y7 = createVar("var14", -3);
    assignVar("var14", num4);
    func7("var13","var14");

    int x8 = createVar("var15", -1);
    assignVar("var15", num1);
    int y8 = createVar("var16", -1);
    assignVar("var16", num2);
    func8("var15","var16");
    
    char x9 = createVar("var17", -2);
    assignVar("var17", ch1);
    char y9 = createVar("var18", -2);
    assignVar("var18", ch2);
    func9("var17","var18");

    medium_int x10 = createVar("var19", -3);
    assignVar("var19", num3);
    medium_int y10 = createVar("var20", -3);
    assignVar("var20", num4);
    func10("var19","var20");

    endScope();

    auto end = chrono::high_resolution_clock::now();
    double time_taken = chrono::duration_cast<seconds>(end - start).count();
    cout<<"Time Taken is "<<time_taken<<" seconds"<<endl;
    cleanExit();
}