#include "syscall.h"

void main()
{
	int hello;
	// int pingPID, pongPID;
	// PrintString("Ping-Pong test starting...\n\n");
	// pingPID = Exec("./test/ping");
	// pongPID = Exec("./test/pong");
	// Join(pingPID);
	// Join(pongPID);
	hello = Open("hello.txt", 0);
	PrintNum(hello);
	PrintChar('\n');

}