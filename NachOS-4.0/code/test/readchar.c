#include "syscall.h"
int main(){
    char result;
    result = ReadNum();
    PrintNum(result);
    Halt();
}