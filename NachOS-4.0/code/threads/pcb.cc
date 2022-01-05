#include "pcb.h"
#include "thread.h"

#include "addrspace.h"
#include "main.h"
PCB::PCB(){
    pid = 0;
    parentID = -1;
    joinsem = new Semaphore("joinsem",0);
    exitsem = new Semaphore("exitsem",0);
    mutex = new Semaphore("mutex",1);
    exitcode = 0;
    numwait = 0;
    thread = NULL;
}

PCB::PCB(int id){
    pid = id;
    parentID = kernel->currentThread->processID;
    joinsem = new Semaphore("joinsem",0);
    exitsem = new Semaphore("exitsem",0);
    mutex = new Semaphore("mutex",1);
    exitcode = 0;
    numwait = 0;
    thread = NULL;
}
PCB::~PCB(){
    if(joinsem != NULL){
        delete joinsem;
    }
    if(exitsem != NULL){
        delete exitsem;
    }
    if(mutex != NULL){
        delete mutex;
    }
    if(thread != NULL){
        thread->Finish();
    }
}
void StartProcess_2(void* id)
{
    
    AddrSpace *space;
    char* filename;
    //TODO
    filename = pTab->GetFileName((int)id); 
    //TODO
    space = new AddrSpace(filename);
	if(space == NULL)
	{
		printf("\nPCB::Exec: Can't create AddSpace.");
		return;
	}

    kernel->currentThread->space = space;

    space->InitRegisters();	// set the initial register values	
    space->RestoreState();	// load page table register	

    kernel->machine->Run();	// jump to the user progam	
    ASSERT(FALSE);	// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"	
}
int PCB::Exec(char* filename, int pid){  
	mutex->P();
	this->thread = new Thread(filename); 
	if(this->thread == NULL){
		printf("\nPCB::Exec: Khong the tao duoc 1 tien trinh \n");
        mutex->V();
	    return -1; 
	}
	this->thread->processID = pid;
	this->parentID = kernel->currentThread->processID;
    this->thread->Fork(StartProcess_2,(void*)pid);
    this->SetFileName(filename);
    mutex->V();
	return pid;

}
int PCB::GetID(){
    return pid;
}
int PCB::GetNumWait(){
    return numwait;
}
void PCB::JoinWait(){
    joinsem->P();
}
void PCB::ExitWait(){
    exitsem->P();
}
void PCB::JoinRelease(){
    joinsem->V();
}
void PCB::ExitRelease(){
    exitsem->V();
}
void PCB::IncNumWait(){
    mutex->P();
    numwait++;
    mutex->V();
}
void PCB::DecNumWait(){
    mutex->P();
    if(numwait > 0){
        numwait--;
    }
    mutex->V();
}
void PCB::SetFileName(char* fn){
    strcpy(filename,fn);
}
char* PCB::GetFileName(){
    return filename;
}
