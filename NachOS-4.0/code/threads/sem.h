#ifndef SEM_H
#define SEM_H
#include "synch.h"
class Semaphore;
class Sem
{
private:
	char name[50];		// Ten cua semaphore
	Semaphore* sem;		// Tao semaphore de quan ly
public:
	// Khoi tao doi tuong Sem. Gan gia tri ban dau la null
	// Nho khoi tao sem su dung
	Sem(char* na, int i);
	~Sem();
	void wait();
	void signal();
	char* GetName();
};
#endif