#ifndef PTABLE_H
#define PTABLE_H

const int MAX_PROCESS = 10;

class PTable{
private:
    Bitmap bm;
    PCB *pcb[MAX_PROCESS];
    int psize;
    Semaphore *bmsem;

public:
    PTable(int size);
    ~PTable();
    int ExecUpdate(char* name);
    int JoinUpdate(int id); 
    bool IsExist(int pid); 
    int GetFreeSlot();
    void Remove(int pid);
    char* GetFileName(int id);
};
#endif
