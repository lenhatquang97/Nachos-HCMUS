#include "syscall.h"

int main() {
	int i,pid, file_output,lengthWrite;
	char temp;
	lengthWrite = 0;
	file_output = Open("output.txt", 0);
	if(file_output == -1) {
		PrintString("Can't open output file");
		return 1;
	}
	pid = CurrentThreadId();
	for (i = 0; i < 3; i++) {
		Wait("voinuoc");
		//PrintNum(pid);
		lengthWrite = Seek(-1, file_output);
		temp = pid + '0';
		WriteAt(&temp, 1, file_output, lengthWrite);
		lengthWrite = Seek(-1, file_output);
		temp = ' ';
		WriteAt(&temp, 1, file_output, lengthWrite);
		Signal("voinuoc");
	} 
	Close(file_output);
	Signal("main");
	return 0;
}
