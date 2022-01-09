#include "syscall.h"

int main() {
	int pid = CurrentThreadId();
	int i;
	for (i = 0; i < 3; i++) {
		Wait("voinuoc");
		PrintNum(pid);
		Signal("voinuoc");
	}
	Signal("main");

	return 0;
}
