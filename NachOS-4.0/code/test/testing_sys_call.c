#include "syscall.h"
int main(){
    //int answer;
    //char c;
    char *buffer;
    //answer = ReadNum();
    //PrintNum(answer);
   
    //c = ReadChar();
    //PrintChar(c);

    //answer = RandomNum();
    //PrintNum(answer);

    ReadString(buffer,255);
    PrintString(buffer);

    Halt();
}