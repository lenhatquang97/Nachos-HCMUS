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
  if (!kernel->fileSystem->Create(tempWrite, 0))
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

  int freeSlot = kernel->fileSystem->FindFreeSlot();
  if (freeSlot != -1)
  {
    if (type == 0 || type == 1) //chi xu li khi type = 0 hoac 1
    {

      if ((kernel->fileSystem->openf[freeSlot] = kernel->fileSystem->Open(tempWrite, type)) != NULL) //Mo file thanh cong
      {
        kernel->machine->WriteRegister(2, freeSlot); //tra ve OpenFileID
      }
    }
    else if (type == 2) // xu li stdin voi type quy uoc la 2
    {
      kernel->machine->WriteRegister(2, 0); //tra ve OpenFileID
    }
    else // xu li stdout voi type quy uoc la 3
    {
      kernel->machine->WriteRegister(2, 1); //tra ve OpenFileID
    }
    delete[] tempWrite;
  }
}
//int Close(int id)
void CloseSC()
{
  int fid = kernel->machine->ReadRegister(4); // Lay id cua file tu thanh ghi so 4
  if (fid >= 0 && fid <= 14)                  //Chi xu li khi fid nam trong [0, 14]
  {
    if (kernel->fileSystem->openf[fid]) //neu mo file thanh cong
    {
      delete kernel->fileSystem->openf[fid]; //Xoa vung nho luu tru file
      kernel->fileSystem->openf[fid] = NULL; //Gan vung nho NULL
      kernel->machine->WriteRegister(2, 0);
    }
  }
  kernel->machine->WriteRegister(2, -1);
}
//int Read(char *buffer, int charcount, int id)
void ReadSC()
{

  int virtAddr = kernel->machine->ReadRegister(4);  // Lay dia chi cua tham so buffer tu thanh ghi so 4
  int charcount = kernel->machine->ReadRegister(5); // Lay charcount tu thanh ghi so 5
  int id = kernel->machine->ReadRegister(6);        // Lay id cua file tu thanh ghi so 6
  int OldPos;
  int NewPos;
  char *buf;
  // Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
  if (id < 0 || id > 14)
  {
    printf("\nKhong the read vi id nam ngoai bang mo ta file.");
    kernel->machine->WriteRegister(2, -1);
    increasePC();
    return;
  }
  // Kiem tra file co ton tai khong
  if (kernel->fileSystem->openf[id] == NULL)
  {
    printf("\nKhong the read vi file nay khong ton tai.");
    kernel->machine->WriteRegister(2, -1);
    increasePC();
    return;
  }
  if (kernel->fileSystem->openf[id]->type == 3) // Xet truong hop doc file stdout (type quy uoc la 3) thi tra ve -1
  {
    printf("\nKhong the read file stdout.");
    kernel->machine->WriteRegister(2, -1);
    increasePC();
    return;
  }
  OldPos = kernel->fileSystem->openf[id]->GetCurrentPos(); // Kiem tra thanh cong thi lay vi tri OldPos
  buf = User2System(virtAddr, charcount);                  // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai charcount
  // Xet truong hop doc file stdin (type quy uoc la 2)
  if (kernel->fileSystem->openf[id]->type == 2)
  {
    // Su dung ham Read cua lop SynchConsole de tra ve so byte thuc su doc duoc
    int size = 0;
    for (int i = 0; i < charcount; ++i)
    {
      size = size + 1;
      buf[i] = kernel->synchConsoleIn->GetChar();
      //Quy uoc chuoi ket thuc la \n
      if (buf[i] == '\n')
      {
        buf[i + 1] = '\0';
        break;
      }
    }
    buf[size] = '\0';
    System2User(virtAddr, size, buf);        // Copy chuoi tu vung nho System Space sang User Space voi bo dem buffer co do dai la so byte thuc su
    kernel->machine->WriteRegister(2, size); // Tra ve so byte thuc su doc duoc
    delete buf;
    increasePC();
    return;
  }
  // Xet truong hop doc file binh thuong thi tra ve so byte thuc su
  if ((kernel->fileSystem->openf[id]->Read(buf, charcount)) > 0)
  {
    // So byte thuc su = NewPos - OldPos
    NewPos = kernel->fileSystem->openf[id]->GetCurrentPos();
    // Copy chuoi tu vung nho System Space sang User Space voi bo dem buffer co do dai la so byte thuc su
    System2User(virtAddr, NewPos - OldPos, buf);
    kernel->machine->WriteRegister(2, NewPos - OldPos);
  }
  else
  {
    // Truong hop con lai la doc file co noi dung la NULL tra ve -2
    //printf("\nDoc file rong.");
    kernel->machine->WriteRegister(2, -2);
  }
  delete buf;
  increasePC();
  return;
}
//int Write(char *buffer, int charcount, int id)
void WriteSC()
{
  int virtAddr = kernel->machine->ReadRegister(4);  // Lay dia chi cua tham so buffer tu thanh ghi so 4
  int charcount = kernel->machine->ReadRegister(5); // Lay charcount tu thanh ghi so 5
  int id = kernel->machine->ReadRegister(6);        // Lay id cua file tu thanh ghi so 6
  int OldPos;
  int NewPos;
  char *buf;
  // Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
  if (id < 0 || id > 14)
  {
    printf("\nKhong the write vi id nam ngoai bang mo ta file.");
    kernel->machine->WriteRegister(2, -1);
    increasePC();
    return;
  }
  // Kiem tra file co ton tai khong
  if (kernel->fileSystem->openf[id] == NULL)
  {
    printf("\nKhong the write vi file nay khong ton tai.");
    kernel->machine->WriteRegister(2, -1);
    increasePC();
    return;
  }
  // Xet truong hop ghi file only read (type quy uoc la 1) hoac file stdin (type quy uoc la 2) thi tra ve -1
  if (kernel->fileSystem->openf[id]->type == 1 || kernel->fileSystem->openf[id]->type == 2)
  {
    printf("\nKhong the write file stdin hoac file only read.");
    kernel->machine->WriteRegister(2, -1);
    increasePC();
    return;
  }
  OldPos = kernel->fileSystem->openf[id]->GetCurrentPos(); // Kiem tra thanh cong thi lay vi tri OldPos
  buf = User2System(virtAddr, charcount);                  // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai charcount
  // Xet truong hop ghi file read & write (type quy uoc la 0) thi tra ve so byte thuc su
  if (kernel->fileSystem->openf[id]->type == 0)
  {
    if ((kernel->fileSystem->openf[id]->Write(buf, charcount)) > 0)
    {
      // So byte thuc su = NewPos - OldPos
      NewPos = kernel->fileSystem->openf[id]->GetCurrentPos();
      kernel->machine->WriteRegister(2, NewPos - OldPos);
      delete buf;
      increasePC();
      return;
    }
  }
  if (kernel->fileSystem->openf[id]->type == 3) // Xet truong hop con lai ghi file stdout (type quy uoc la 3)
  {
    int lengthWrite = 0;
    //Truong hop dung thi vi tri chuoi bang 0
    while (buf[lengthWrite] != 0)
    {
      kernel->synchConsoleOut->PutChar(buf[lengthWrite]);
      lengthWrite++;
      //Truong hop chuoi vuot qua 255
      if (lengthWrite == 255)
      {
        delete[] buf;
        virtAddr = virtAddr + 255;
        buf = User2System(virtAddr, 255);
        lengthWrite = 0;
      }
    }
    kernel->machine->WriteRegister(2, lengthWrite - 1); // Tra ve so byte thuc su write duoc
    delete buf;
    increasePC();
    return;
  }
}
void ExecSC()
{
  int bufferWrite, openState, id;
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
  OpenFile *oFile = kernel->fileSystem->Open(tempWrite);
  if (oFile == NULL)
  {
    printf("\nExec:: Can't open this file.");
    kernel->machine->WriteRegister(2, -1);
    increasePC();
    return;
  }

  delete oFile;
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
  int bufferWrite, semval, res;
  char *name;
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
  int bufferWrite, res;
  char *name;
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
  char *name;

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