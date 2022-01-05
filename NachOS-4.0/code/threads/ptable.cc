#include "ptable.h"
#include "main.h"

PTable::PTable(int size) {
    if (size < 0) return;
    psize = size;
    bm = new Bitmap(size);
    bmsem = new Semaphore("bmsem", 1);

    for (int i = 0; i < MAX_PROCESS; i+) {
        pcb[i] = 0;
    }
    bm->Mark(0);

    pcb[0] = new PCB(0);
    pcb[0]->SetFileName("./test/scheduler");
    pcb[0]->parentID = -1;
}

PTable::~PTable() {
    if (bm != 0) delete bm;
    for (int i = 0; i < psize; i++) {
        if (pcb[i] != 0) delete pcb[i];
    }
    if (bmsem != 0) delete bmsem;
}

int PTable::ExecUpdate(char *name) {
    bmsem->P();

    if (name == NULL) {
        printf("\nPTable::Exec: name is null\n");
        bmsem->V();
        return -1;
    }

    if (strcmp(name, "./test/scheduler") == 0 || strcmp(name, currentThread->getName()) == 0) {
        printf("\nPTable::Exec: Can't execute\n");
        bmsem->V();
        return -1;
    }

    int free_slot = this->GetFreeSlot();
    if (free_slot < 0) {
        printf("\nPTable::Exec: There is no free slot\n");
        bmsem->V();
        return -1;
    }
    pcb[free_slot] = new PCB(free_slot);
    pcb[free_slot]->SetFileName(name);
    pcb[free_slot]->parentID = currentThread->processID;

    int pid = pcb[free_slot]->Exec(name, free_slot);
    bmsem->V();
    return pid;
}

int PTable::JoinUpdate(int id) {
    if (id < 0) {
        printf("\nPTable::JoinUpdate: id isn't valid");
        return -1;
    }

    if (currentThread->processID != pcb[id]->parentID) {
        printf("\nPTable::JoinUpdate: ");
        return -1;
    }

    pcb[pcb[id]->parentID]->IncNumWait();
    pcb[id]->JoinWait();
    int ec = pcb[id]->GetExitCode();
    pcb[id]->ExitRelease();
    return ec;
}

int PTable::ExitUpdate(int exit_code) {
    int id = currentThread->processID;
    if (id == 0) {
        currentThread->FreeSpace();
        interrupt->Halt();
        return 0;
    }
    if (IsExist(id) == false) {
        printf("\nPTable::ExitUpdate id doesn't exist");
        return -1;
    }
    pcb[id]->SetExitCode(exit_code);
    pcb[pcb[id]->parentID]->DecNumWait();
    pcb[id]->JoinRelease();
    pcb[id]->ExitWait();
    Remove(id);
    return exit_code;
}

int PTable::GetFreeSlot() {
    return bm->Find();
}

void PTable::Remove(int pid) {
    bm->Clear(pid);
    if (pcb[pid] != 0) delete pcb[pid];
}

char *PTable::GetFileName(int id) {
    return pcb[id]->GetFileName();
}
