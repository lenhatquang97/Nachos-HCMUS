#include "syscall.h"
int main(){
    char result;
    result = ReadInt();
    PrintInt(result);
    Halt();
}