#include "stable.h"

Sem::Sem(char *na, int i)
{
	strcpy(this->name, na);
	sem = new Semaphore(this->name, i);
}
Sem::~Sem()
{
	if (sem){
		delete sem;
	}
}
void Sem::wait()
{
	sem->P(); // Down(sem)
}
void Sem::signal()
{
	sem->V(); // Up(sem)
}
char *Sem::GetName()
{
	return this->name;
}
// Constructor
STable::STable()
{
	this->bm = new Bitmap(MAX_SEMAPHORE);

	for (int i = 0; i < MAX_SEMAPHORE; i++)
	{
		this->semTab[i] = NULL;
	}
}

// Destructor
STable::~STable()
{
	if (this->bm)
	{
		delete this->bm;
		this->bm = NULL;
	}
	for (int i = 0; i < MAX_SEMAPHORE; i++)
	{
		if (this->semTab[i])
		{
			delete this->semTab[i];
			this->semTab[i] = NULL;
		}
	}
}

int STable::Create(char *name, int init)
{

	// Check da ton tai semaphore nay chua?
	for (int i = 0; i < MAX_SEMAPHORE; i++)
	{
		if (bm->Test(i))
		{
			if (strcmp(name, semTab[i]->GetName()) == 0)
			{
				return -1;
			}
		}
	}
	// Tim slot tren bang semTab trong
	int id = this->FindFreeSlot();

	// Neu k tim thay thi tra ve -1
	if (id < 0)
	{
		return -1;
	}

	// Neu tim thay slot trong thi nap Semaphore vao semTab[id]
	this->semTab[id] = new Sem(name, init);
	return 0;
}

int STable::Wait(char *name)
{
	for (int i = 0; i < MAX_SEMAPHORE; i++)
	{
		// Kiem tra o thu i da duoc nap semaphore chua
		if (bm->Test(i))
		{
			// Neu co thi tien hanh so sanh name voi name cua semaphore trong semTab
			if (strcmp(name, semTab[i]->GetName()) == 0)
			{
				// Neu ton tai thi cho semaphore down();
				semTab[i]->wait();
				return 0;
			}
		}
	}
	printf("Khong ton tai semaphore");
	return -1;
}

int STable::Signal(char *name)
{
	for (int i = 0; i < MAX_SEMAPHORE; i++)
	{
		// Kiem tra o thu i da duoc nap semaphore chua
		if (bm->Test(i))
		{
			// Neu co thi tien hanh so sanh name voi name cua semaphore trong semTab
			if (strcmp(name, semTab[i]->GetName()) == 0)
			{
				// Neu ton tai thi cho semaphore up();
				semTab[i]->signal();
				return 0;
			}
		}
	}
	printf("Khong ton tai semaphore");
	return -1;
}

int STable::FindFreeSlot()
{
	return this->bm->FindAndSet();
}