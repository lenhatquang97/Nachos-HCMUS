#include "syscall.h"
int main(){
    // int va;
    // va = CreateFile("hehe.txt");
    
    // PrintNum(va);
    int id, val;
    char* buffer;
    buffer = "haha";
    id = Open("hello.txt",0);
    val = Write(buffer,4,id);
    val = Write(buffer,4,id);
    //Close(id);
    Halt();
}