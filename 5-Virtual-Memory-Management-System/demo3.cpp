#include "memlab.h"
using namespace std;

int main() {
    createMem(500);
    gc_initialize();

    initScope();

    int varint = createVar("xyz1", -1);
    int varchar = createVar("xyz2", -2);
    int arrint = createArr("arr3", -1, 3);
    int vararmint = createVar("var4", -3);

    assignVar("xyz1", 10);
    cout << readInt("xyz1") << endl;

    assignVar("xyz2", 'F');
    cout << readChar("xyz2") << endl;

    assignArr("arr3", 2, 0);
    assignArr("arr3", 3, 1);
    assignArr("arr3", 1, 2);

    assignVar("var4", medium_int(-(1 << 4)));
    cout << readMediumInt("var4") << endl;

    printMemStatus(50);
    freeElem("xyz1");
    // freeElem("xyz2");
    freeElem("arr3");
    // freeElem("var4");
    printMemStatus(50);
    gc_compaction();
    printMemStatus(50);
    cout << readChar("xyz2") << endl;
    cout << readMediumInt("var4") << endl;
    endScope();
    printMemStatus(50);
    cleanExit();
    return 0;
}