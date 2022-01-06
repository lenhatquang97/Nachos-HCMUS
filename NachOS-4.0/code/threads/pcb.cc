#include "main.h"
#include "pcb.h"
 
PCB::PCB(){ 
    pid = 0;
    parentID = -1;
    joinsem = new Semaphore("joinsem",0);
    exitsem = new Semaphore("exitsem",0);
    mutex = new Semaphore("mutex",1);
    exitcode = 0;
    numwait = 0;
    thread = NULL;
    bmfile = NULL;
}

PCB::PCB(int id){
    joinsem = new Semaphore("joinsem",0);
    exitsem = new Semaphore("exitsem",0);
    mutex = new Semaphore("mutex",1);

    exitcode = 0;
    numwait = 0;
    thread = NULL;

    pid = id;
    if (id <= 0)
        parentID = -1;
    else
        parentID = kernel->currentThread->processID;


    bmfile = new Bitmap(MAX_FILE);
    bmfile->Mark(0);
    bmfile->Mark(1);
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
    if(thread != NULL){
        thread->FreeSpace();
        thread->Finish();
    }
}
void StartProcess_2(void* pid)
{
    int id = *((int *)pid);
    AddrSpace *space;
    char* filename = kernel->pTab->GetFileName((int)pid); 
    space = new AddrSpace();
	if(space->Load(filename))
	{
		space->Execute();
        ASSERTNOTREACHED();
	}
    if(space != NULL){
        delete space;
    }
    // kernel->currentThread->space = space;

    // space->InitRegisters();	// set the initial register values	
    // space->RestoreState();	// load page table register	

    // kernel->machine->Run();	// jump to the user progam	
    // ASSERT(FALSE);	// machine->Run never returns;
	// 				// the address space exits
	// 				// by doing the syscall "exit"	
}
int PCB::Exec(char* filename, int pid){  
	mutex->P();
	thread = new Thread(filename); 
	if(thread == NULL){
		printf("\n Khong the tao duoc 1 tien trinh \n");
        mutex->V();
	    return -1; 
	}
	thread->processID = pid;
    //thread->Fork((VoidFunctionPtr)StartProcess_2,&thread->processID);
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
void PCB::SetExitCode(int ec){
    exitcode = ec;
}

int PCB::GetExitCode(){
    return exitcode;
} 

void PCB::SetFileName(char* fn){
    if(thread != NULL){
        thread->setName(fn);
    }
}
char* PCB::GetFileName(){
    if (thread != NULL){
        return thread->getName();
    }
    return NULL;
}
int PCB::GetFreeSlot(){
    return bmfile->FindAndSet();
}
bool PCB::IsExist(int id)
{
    if (id < 0 || id >= MAX_FILE)
        return false;
    return bmfile->Test(id);
}