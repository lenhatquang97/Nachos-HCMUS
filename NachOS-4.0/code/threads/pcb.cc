#include "main.h"
#include "pcb.h"
#include "directory.h"
//extern void StartProcess_2(int id);
PCB::PCB(int id)
{
	this->joinsem = new Semaphore("joinsem", 0);
	this->exitsem = new Semaphore("exitsem", 0);
	this->multex = new Semaphore("multex", 1);

	this->numwait = this->exitcode = 0;

	this->thread = NULL;
	this->bmfile = new Bitmap(MAX_FILE);
	this->pid = id;

	if (id <= 0)
		this->parentID = -1;
	else
		this->parentID = kernel->currentThread->processID;

	this->fileTable = new OpenFile *[MAX_FILE];
	for (int i = 0; i < 10; ++i)
	{
		fileTable[i] = NULL;
	}
	kernel->fileSystem->Create("stdout",0);
	fileTable[CONSOLE_OUT] = kernel->fileSystem->Open("stdout");
	fileTable[CONSOLE_OUT]->type = 0; 
	bmfile->Mark(CONSOLE_OUT);

	kernel->fileSystem->Create("stdin",0);
	fileTable[CONSOLE_INP] = kernel->fileSystem->Open("stdin");
	fileTable[CONSOLE_INP]->type = 1;
	bmfile->Mark(CONSOLE_INP);
}

PCB::~PCB()
{
	if (joinsem != NULL)
		delete this->joinsem;
	if (exitsem != NULL)
		delete this->exitsem;
	if (multex != NULL)
		delete this->multex;
	if (bmfile != NULL)
		delete this->bmfile;
	if (thread != NULL)
	{
		thread->FreeSpace();
		thread->Finish();
	}
	for (int i = 0; i < 10; ++i)
	{
		if (fileTable[i] != NULL)
			delete fileTable[i];
	}
	delete[] fileTable;
}

void StartProcess_2(void *pid)
{
	// Lay fileName cua process id nay
	int id = *((int *)pid);
	delete (int *)pid;
	char *fileName = pTab->GetFileName((int)id);
	AddrSpace *space;
	space = new AddrSpace();
	if (space == NULL)
	{
		printf("\nPCB::Exec: Can't create AddSpace.");
		return;
	}

	kernel->currentThread->space = space;

	space->InitRegisters(); // set the initial register values
	space->RestoreState();	// load page table register

	kernel->machine->Run(); // jump to the user progam
	ASSERT(FALSE);			// machine->Run never returns;
							// the address space exits 
							// by doing the syscall "exit"
}

int PCB::Exec(char *filename, int id)
{
	// Gọi mutex->P(); để giúp tránh tình trạng nạp 2 tiến trình cùng 1 lúc.
	multex->P();

	// Kiểm tra thread đã khởi tạo thành công chưa, nếu chưa thì báo lỗi là không đủ bộ nhớ, gọi mutex->V() và return.
	this->thread = new Thread(filename);

	if (this->thread == NULL)
	{
		printf("\nPCB::Exec:: Not enough memory..!\n");
		multex->V();
		return -1;
	}
	//  Đặt processID của thread này là id.
	this->thread->processID = id;

	// Đặt parrentID của thread này là processID của thread gọi thực thi Exec
	this->parentID = kernel->currentThread->processID;
	// Gọi thực thi Fork(StartProcess_2,id) => Ta cast thread thành kiểu int, sau đó khi xử ký hàm StartProcess ta cast Thread về đúng kiểu của nó.
	printf("\nThread %d is created.\n", id);
	void *pid = new int;
	*(int *)pid = id;
	this->thread->Fork(StartProcess_2, pid); // StartProcess_2 : void Func(void* Args)

	multex->V();
	// Trả về id.
	return id;
}

int PCB::FindFreeSlot()
{
	return bmfile->FindAndSet();
}
int PCB::GetID() { return this->pid; }
int PCB::GetNumWait() { return this->numwait; }
int PCB::GetExitCode() { return this->exitcode; }

void PCB::SetExitCode(int ec) { this->exitcode = ec; }

// Process tranlation to block
// Wait for JoinRelease to continue exec
void PCB::JoinWait()
{
	//Gọi joinsem->P() để tiến trình chuyển sang trạng thái block và ngừng lại, chờ JoinRelease để thực hiện tiếp.
	joinsem->P();
}

// JoinRelease process calling JoinWait
void PCB::JoinRelease()
{
	// Gọi joinsem->V() để giải phóng tiến trình gọi JoinWait().
	joinsem->V();
}

// Let process tranlation to block state
// Waiting for ExitRelease to continue exec
void PCB::ExitWait()
{
	// Gọi exitsem-->V() để tiến trình chuyển sang trạng thái block và ngừng lại, chờ ExitReleaseđể thực hiện tiếp.
	exitsem->P();
}

// Release wating process
void PCB::ExitRelease()
{
	// Gọi exitsem-->V() để giải phóng tiến trình đang chờ.
	exitsem->V();
}

void PCB::IncNumWait()
{
	multex->P();
	++numwait;
	multex->V();
}

void PCB::DecNumWait()
{
	multex->P();
	if (numwait > 0)
		--numwait;
	multex->V();
}

void PCB::SetFileName(char *fn) { strcpy(FileName, fn); }
char *PCB::GetFileName()
{
	if (thread != NULL)
	{
		return this->FileName;
	}
	return NULL;
}

bool PCB::IsExist(int id)
{
	if (id < 0 || id >= MAX_FILE)
		return false;
	return bmfile->Test(id);
}
OpenFile *PCB::Open(char *name)
{
	int fileDescriptor = OpenForReadWrite(name, FALSE);

	if (fileDescriptor == -1)
	{
		return NULL;
	}

	return new OpenFile(fileDescriptor);
}
OpenFile* PCB::Open(char *name, int type)
{
	OpenFile* f = kernel->fileSystem->Open(name);
	if(f == NULL)
		return NULL;
	f->type = type;
	return f;
	// int fileDescriptor = OpenForReadWrite(name, FALSE);

	// if (fileDescriptor == -1)
	// {
	// 	cout << "Open file error" << endl;
	// 	return NULL;
	// }
	
	// //index++;
	// return new OpenFile(fileDescriptor, type);
}
bool PCB::Remove(char *name)
{
	return Unlink(name) == 0;
}