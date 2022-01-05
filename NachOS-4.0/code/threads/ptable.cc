#include "ptable.h"
#include "filesys.h"

int PTable::ExecUpdate(char *name) {
    mutex->P();

    FileSystem fsys;
    if (fsys.Open(name) == NULL) {
        return -1;
    }
    if (kernel->currentThread->getName() == name) {
        return -1;
    }
}
