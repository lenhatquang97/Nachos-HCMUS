#ifndef PCB_H
#define PCB_H

#include "synch.h"
#include "thread.h"

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
        char filename[MAX_LEN];
    public:
        int parentID;
        PCB();
        PCB(int id);
        ~PCB();
        int Exec(char *filename, int pid); 
        int GetID(){return pid;} 
        int GetNumWait(); 
        void JoinWait(); 
        void ExitWait(); 
        void JoinRelease(); 
        void ExitRelease(); 
        void IncNumWait(); 
        void DecNumWait(); 
        void SetExitCode(int ec){exitcode = ec;} 
        int GetExitCode(){return exitcode;} 
        void SetFileName(char* fn); 
        char* GetFileName(); 

};
#endif