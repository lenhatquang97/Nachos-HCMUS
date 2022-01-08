#include "syscall.h"

void main()
{
	// int pingPID, pongPID;
	// PrintString("Ping-Pong test starting...\n\n");
	// pingPID = Exec("./test/ping");
	// pongPID = Exec("./test/pong");
	// Join(pingPID);
	// Join(pongPID);
	int hello;
	hello = Open("hello.txt", 0);
	PrintString("Testing");
	PrintNum(hello);
	PrintChar('\n');

}