#include "syscall.h"
int main(){
    PrintString("Decimal\tCharacter\n");
    for(int i = 32; i < 128; ++i) {
        PrintNum(i); PrintString("\t");
        PrintChar(i); PrintString("\n");
    }
    Halt();
}