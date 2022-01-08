#include "syscall.h"

void main()
{
	int id;
	char* buffer;
	id = Open("hello.txt",1);
	PrintNum(id);
	Write(buffer,255,id);
	PrintString(buffer);
	Close(id);
}