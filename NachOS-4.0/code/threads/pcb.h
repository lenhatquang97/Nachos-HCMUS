#ifndef PCB_H
#define PCB_H
#include "thread.h"
#include "synch.h"

#define MAX_SIZE 20
#define MAX_LEN 255
#define MAX_FILE 10
class PCB{
    private:
        Semaphore* joinsem;
        Semaphore* exitsem;
        Semaphore* mutex;
        int exitcode;
        Thread* thread;
        int pid;        
        int numwait;
        Bitmap* bmfile;
    public:
        int parentID;
        PCB();
        PCB(int id);
        ~PCB();
        int Exec(char *filename, int pid); 
        int GetID(); 
        int GetNumWait(); 
        void JoinWait(); 
        void ExitWait(); 
        void JoinRelease(); 
        void ExitRelease(); 
        void IncNumWait(); 
        void DecNumWait(); 
        void SetExitCode(int ec);
        int GetExitCode();
        void SetFileName(char* fn); 
        char* GetFileName(); 
        int GetFreeSlot();
        bool IsExist(int id);
};
#endif