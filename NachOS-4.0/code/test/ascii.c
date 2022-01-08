#include "syscall.h"
int main(){
    int i;
    PrintString("Decimal\t\tCharacter\n\n");
    for(i = 32; i < 128; ++i) {
        PrintNum(i); PrintString("\t\t");
        PrintChar(i); PrintString("\n\n");
    }
}