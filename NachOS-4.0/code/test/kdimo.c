#include "syscall.h"

int main() {
  //Create Semaphore 
  //We need to create semaphore with second arg == 0 to use wait function
  int n,i, create_result,file_open;
  char temp;
  
  create_result = CreateSemaphore("main", 0);
  file_open = Open("./input.txt", 1);
  
  if (create_result == -1) {
    PrintString("kdimo 7: Can't create main semaphore");
    return 1;
  }
  
  create_result = CreateFile("output.txt");
  if (create_result == -1) {
    PrintString("Can't create output file");
    return 1;
  }


  create_result = CreateSemaphore("voinuoc", 1);
  if (create_result == -1) {
    PrintString("kdimo 12: Can't create voinuoc semaphore");
  }
 
  if (file_open == -1) {
    PrintString("Can't open file input");
    return 1;
  }

  while (1) {
    Read(&temp, 1, file_open);
    if ('0' <= temp && temp <= '9') {
      n = temp - '0';
    }
    break;
  }

  Close(file_open);

  for (i = 0; i < n; i++) {
    Exec("./test/sinhvien");
  }

  for (i = 0; i < n; i++) {
    Wait("main");
  }

  return 0;
}
