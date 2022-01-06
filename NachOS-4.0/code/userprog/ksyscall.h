/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__
#define __USERPROG_KSYSCALL_H__

#include "kernel.h"
#include "synchconsole.h"
#include <fcntl.h>
#include <unistd.h>
void SysHalt()
{
  kernel->interrupt->Halt();
}

int SysAdd(int op1, int op2)
{
  return op1 + op2;
}
//Increase Program Counter
void increasePC()
{
  /* set previous programm counter (debugging only)*/
  kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

  /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

  /* set next programm counter for brach execution */
  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
}

char *User2System(int buffer, int limit)
{
  int i; // index
  int oneChar;
  char *kernelBuf = NULL;
  kernelBuf = new char[limit + 1]; //need for terminal string
  if (kernelBuf == NULL)
    return kernelBuf;
  memset(kernelBuf, 0, limit + 1);
  //printf("\n Filename u2s:");
  for (i = 0; i < limit; i++)
  {
    //Doc 1 ki tu
    kernel->machine->ReadMem(buffer + i, 1, &oneChar);
    kernelBuf[i] = (char)oneChar;
    //printf("%c",kernelBuf[i]);
    if (oneChar == 0)
      break;
  }
  return kernelBuf;
}

int System2User(int virtAddr, int len, char *buffer)
{
  if (len < 0)
    return -1;
  if (len == 0)
    return len;
  int i = 0;
  int oneChar = 0;
  do
  {
    oneChar = (int)buffer[i];
    //Ghi 1 ki tu
    kernel->machine->WriteMem(virtAddr + i, 1, oneChar);
    i++;
  } while (i < len && oneChar != 0);
  return i;
}
//int createFile(char* name)
void CreateFileSC()
{
  int bufferWrite;
  char *tempWrite;
  bufferWrite = kernel->machine->ReadRegister(4);
  //Khoi tao vung nho tu user cho system
  tempWrite = User2System(bufferWrite, 255);
  if (tempWrite == NULL)
  {
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
  if (creat(tempWrite, mode) == -1)
  {
    delete[] tempWrite;
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  kernel->machine->WriteRegister(2, 0);
  delete[] tempWrite;
  return;
}
//int Open(char* name, int type)
void OpenSC()
{
  int bufferWrite, type, answer;
  char *tempWrite;
  bufferWrite = kernel->machine->ReadRegister(4);
  type = (int)kernel->machine->ReadRegister(5);

  //Chuyen vung nho user sang system
  tempWrite = User2System(bufferWrite, 255);
  if (tempWrite == NULL)
  {
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  if (type < 0 || type > 1)
  {
    printf("Wtf %d", type);
    delete[] tempWrite;
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  //quyet dinh xem read only hay read and write
  if (type == 0)
  {
    type = O_RDWR;
  }
  else
  {
    type = O_RDONLY;
  }
  //file id
  answer = open(tempWrite, type);
  if (answer >= 10 || answer <= 1)
  {
    printf("Da het slot file\n");
    delete[] tempWrite;
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  printf("Thanh cong\n");
  kernel->machine->WriteRegister(2, (int)answer);
  delete[] tempWrite;
  return;
}
//int Close(int id)
void CloseSC()
{
  int id;
  id = kernel->machine->ReadRegister(4);
  //Check id available in filetable: TODO
  if (id <= 1 || id >= 10)
  {
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  //0 - 1 cannot close: TODO
  if (!close(id))
  {
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  kernel->machine->WriteRegister(2, 0);
  return;
}
//int Read(char *buffer, int charcount, int id)
void ReadSC()
{
  int bufferRead, size, id, answer;
  char *tempRead;
  bufferRead = kernel->machine->ReadRegister(4);
  size = kernel->machine->ReadRegister(5);
  id = kernel->machine->ReadRegister(6);
  tempRead = new char[size + 1];
  if (id == 0)
  {
    for (int i = 0; i < size; ++i)
    {
      tempRead[i] = kernel->synchConsoleIn->GetChar();
      //Quy uoc chuoi ket thuc la \n
      if (tempRead[i] == '\n')
      {
        tempRead[i + 1] = '\0';
        break;
      }
    }
    tempRead[size] = '\0';
    //Copy tu vung nho system sang user
    System2User(bufferRead, size, tempRead);
    delete[] tempRead;
    kernel->machine->WriteRegister(2, -2);
    return;
  }
  if (id == 1)
  {
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  if (id < 0 || id >= 10)
  {
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  answer = read(id, tempRead, size);
  if (answer == -1)
  {
    printf("Doc khong thanh cong!!\n");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  if (answer == 0)
  {
    printf("Doc toi cuoi file!!\n");
    kernel->machine->WriteRegister(2, -2);
    return;
  }
  printf("Doc file thanh cong, chieu dai la %d va noi dung la %s\n", answer, tempRead);
  delete[] tempRead;
  kernel->machine->WriteRegister(2, (int)answer);
  return;
}
//int Write(char *buffer, int charcount, int id)
void WriteSC()
{
  int bufferWrite, size, id, answer;
  char *tempWrite;
  bufferWrite = kernel->machine->ReadRegister(4);
  size = kernel->machine->ReadRegister(5);
  id = kernel->machine->ReadRegister(6);
  tempWrite = User2System(bufferWrite, 255);
  if (tempWrite == NULL)
  {
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  if (id == 0)
  {
    delete[] tempWrite;
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  if (id == 1)
  {
    int lengthWrite = 0;
    //Truong hop dung thi vi tri chuoi bang 0
    while (tempWrite[lengthWrite] != 0)
    {
      kernel->synchConsoleOut->PutChar(tempWrite[lengthWrite]);
      lengthWrite++;
      //Truong hop chuoi vuot qua 255
      if (lengthWrite == 255)
      {
        delete[] tempWrite;
        bufferWrite = bufferWrite + 255;
        tempWrite = User2System(bufferWrite, 255);
        lengthWrite = 0;
      }
    }
    delete[] tempWrite;
    kernel->machine->WriteRegister(2, -2);
    return;
  }
  if (id < 0 || id >= 10)
  {
    delete[] tempWrite;
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  answer = write(id, tempWrite, size);
  if (answer == -1)
  {
    printf("Ghi khong thanh cong!!\n");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  if (answer == sizeof(tempWrite))
  {
    printf("Ghi toi cuoi file!!\n");
    delete[] tempWrite;
    kernel->machine->WriteRegister(2, -2);
    return;
  }
  delete[] tempWrite;

  kernel->machine->WriteRegister(2, (int)answer);
  return;
}
void ExecSC()
{
  int bufferWrite, openState,id;
  char *tempWrite;
  bufferWrite = kernel->machine->ReadRegister(4);
  tempWrite = User2System(bufferWrite, 255);
  if (tempWrite == NULL)
  {
    DEBUG('a', "\nSC_Exec: not enough memory");
    printf("\nSC_Exec: not enough memory");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  openState = open(tempWrite, 0);
  if (openState == -1)
  {
    printf("\nSC_Exec: can't open file");
    kernel->machine->WriteRegister(2, -1);
    increasePC();
    return;
  }
  id = kernel->pTab->ExecUpdate(tempWrite); 
  kernel->machine->WriteRegister(2, id);
  delete[] tempWrite;
  increasePC();
  return;
}

void JoinSC()
{
  int id, res;
  id = kernel->machine->ReadRegister(4);
  res = kernel->pTab->JoinUpdate(id);
  kernel->machine->WriteRegister(2, res);
  increasePC();
  return;
}
void ExitSC()
{
  int exit_status, res;
  exit_status = kernel->machine->ReadRegister(4);
  if (exit_status != 0)
  {
    increasePC();
    return;
  }
  res = kernel->pTab->ExitUpdate(exit_status);
  kernel->currentThread->FreeSpace();
  kernel->currentThread->Finish();
  increasePC();
  return;
}
void CreateSemaphoreSC()
{
  int bufferWrite, semval,res;
  char* name;
  bufferWrite = kernel->machine->ReadRegister(4);
  semval = kernel->machine->ReadRegister(5);

  name = User2System(bufferWrite, 255);
  if (name == NULL)
  {
    DEBUG('a', "\n Not enough memory in System");
    printf("\n Not enough memory in System");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    increasePC();
    return;
  }

  res = kernel->semTab->Create(name, semval);

  if (res == -1)
  {
    DEBUG('a', "\n Khong the khoi tao semaphore");
    printf("\n Khong the khoi tao semaphore");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    increasePC();
    return;
  }

  delete[] name;
  kernel->machine->WriteRegister(2, res);
  increasePC();
  return;
}
void WaitSC()
{
  int bufferWrite,res;
  char* name;
  bufferWrite = kernel->machine->ReadRegister(4);

  name = User2System(bufferWrite, 255);
  if (name == NULL)
  {
    DEBUG('a', "\n Not enough memory in System");
    printf("\n Not enough memory in System");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    increasePC();
    return;
  }

  res = kernel->semTab->Wait(name);

  if (res == -1)
  {
    DEBUG('a', "\n Khong ton tai ten semaphore nay!");
    printf("\n Khong ton tai ten semaphore nay!");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    increasePC();
    return;
  }

  delete[] name;
  kernel->machine->WriteRegister(2, res);
  increasePC();
  return;
}
void SignalSC()
{
  int bufferWrite, res;
  char* name;
  
  
  bufferWrite = kernel->machine->ReadRegister(4);
  name = User2System(bufferWrite, 255);
  if (name == NULL)
  {
    DEBUG('a', "\n Not enough memory in System");
    printf("\n Not enough memory in System");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    increasePC();
    return;
  }

  res = kernel->semTab->Signal(name);

  if (res == -1)
  {
    DEBUG('a', "\n Khong ton tai ten semaphore nay!");
    printf("\n Khong ton tai ten semaphore nay!");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    increasePC();
    return;
  }

  delete[] name;
  kernel->machine->WriteRegister(2, res);
  increasePC();
  return;
}

#endif /* ! __USERPROG_KSYSCALL_H__ */