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
char *User2System(int virtAddr, int limit)
{
  int i; // index
  int oneChar;
  char *kernelBuf = NULL;
  kernelBuf = new char[limit + 1]; //need for terminal string
  if (kernelBuf == NULL)
    return kernelBuf;
  memset(kernelBuf, 0, limit + 1);
  for (i = 0; i < limit; i++)
  {
    //Doc 1 ki tu
    kernel->machine->ReadMem(virtAddr + i, 1, &oneChar);
    kernelBuf[i] = (char)oneChar;
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

void SysHalt()
{
  kernel->interrupt->Halt();
}

int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

void CreateFileSC()
{
  int bufferWrite;
  char *tempWrite;
  bufferWrite = kernel->machine->ReadRegister(4);
  //Khoi tao vung nho tu user cho system
  tempWrite = User2System(bufferWrite, 255);
  if (tempWrite == NULL || strlen(tempWrite) == 0)
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
void OpenSC()
{
  int bufferWrite, type;
  char *tempWrite;
  bufferWrite = kernel->machine->ReadRegister(4);
  type = (int)kernel->machine->ReadRegister(5);
  tempWrite = User2System(bufferWrite, 255);
  int freeSlot = pTab->GetPCB(kernel->currentThread->processID)->FindFreeSlot();
  if (freeSlot != -1)
  {

    if (strcmp(tempWrite, "stdin") == 0 && type == 1)
    {
      kernel->machine->WriteRegister(2, 0);
      delete[] tempWrite;
      return;
    }
    else if (strcmp(tempWrite, "stdout") == 0 && type == 0)
    {
      //TH la stdout
      kernel->machine->WriteRegister(2, 1); 
      delete[] tempWrite;
      return;
    }
    else
    {
      OpenFile *openFile = pTab->GetPCB(kernel->currentThread->processID)->Open(tempWrite, type);
      if (openFile != NULL)
      {
        openFile->type = type;
        pTab->GetPCB(kernel->currentThread->processID)->fileTable[freeSlot] = openFile;
        kernel->machine->WriteRegister(2, freeSlot);
        delete[] tempWrite;
        return;
      }
    }
  }
  kernel->machine->WriteRegister(2, -1);
}
void CloseSC()
{
  OpenFileId fid;
  fid = kernel->machine->ReadRegister(4);
  if (fid >= 0 && fid <= 9) //Chi xu li khi fid nam trong [0, 9]
  {
    if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[fid]) 
    {
      delete pTab->GetPCB(kernel->currentThread->processID)->fileTable[fid]; 
      pTab->GetPCB(kernel->currentThread->processID)->fileTable[fid] = NULL; 
      kernel->machine->WriteRegister(2, 0);
    }
  }
  kernel->machine->WriteRegister(2, -1);
}
void ReadSC()
{
  int OldPos, NewPos;
  char *buf;
  int virtAddr = kernel->machine->ReadRegister(4); 
  int charcount = kernel->machine->ReadRegister(5); 
  int id = kernel->machine->ReadRegister(6);
  
  
  buf = User2System(virtAddr, charcount);
  // Kiem tra id nam trong fileTable
  if (id < 0 || id > 9)
  {
    printf("\nKhong the read vi id nam ngoai bang mo ta file.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  // Kiem tra file co ton tai khong
  if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id] == NULL)
  {
    printf("\nKhong the read vi file nay khong ton tai.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  //Kiem tra la stdout
  if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->type == 0 && strcmp(buf, "stdout") == 0)
  {
    printf("\nKhong the read file.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  OldPos = pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->GetCurrentPos(); // Lay old pos
  //Kiem tra la stdin
  if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->type == 1 && strcmp(buf, "stdin") == 0)
  {
    int size = System2User(virtAddr, charcount, buf); // Copy chuoi tu vung system sang vung user
    kernel->machine->WriteRegister(2, size);         
    delete buf;
    return;
  }
  if ((pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->Read(buf, charcount)) > 0)
  {
    NewPos = pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->GetCurrentPos();
    // So byte thuc su = NewPos - OldPos
    // Copy chuoi tu vung nho System Space sang User Space
    System2User(virtAddr, NewPos - OldPos, buf);
    kernel->machine->WriteRegister(2, NewPos - OldPos);
    delete buf;
    return;
  }
  else
  {
    // Truong hop con lai tra ve -2
    kernel->machine->WriteRegister(2, -2);
    delete buf;
    return;
  }

  return;
}
void WriteSC()
{
  int virtAddr = kernel->machine->ReadRegister(4);  
  int charcount = kernel->machine->ReadRegister(5); 
  int id = kernel->machine->ReadRegister(6);        
  int OldPos, NewPos;
  char *buf;
  buf = User2System(virtAddr, charcount);
  // Kiem tra id trong fileTable ko
  if (id < 0 || id > 9)
  {
    printf("\nKhong the write vi id nam ngoai bang mo ta file.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  // Kiem tra file co ton tai khong
  if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id] == NULL)
  {
    printf("\nKhong the write vi file nay khong ton tai.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }

  if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->type == 1)
  {
    printf("\nKhong the ghi stdin hoac chi doc.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  OldPos = pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->GetCurrentPos(); // Kiem tra thanh cong Lay vi tri OldPos
                                                                                          
  //Xet truong hop ghi file read & write (type quy uoc la 0)
  if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->type == 0)
  {
    if ((pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->Write(buf, charcount)) > 0)
    {
      // So byte thuc su = NewPos - OldPos
      NewPos = pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->GetCurrentPos();
      kernel->machine->WriteRegister(2, NewPos - OldPos);
      delete buf;
      return;
    }
    if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->type == 0)
    {
      //Xuat ra stdout o console
      int i = 0;
      while (buf[i] != 0 && buf[i] != '\n') 
      {
        kernel->synchConsoleOut->PutChar(buf[i]); 
        i++;
      }
      buf[i] = '\n';
      kernel->synchConsoleOut->PutChar(buf[i]); 
      kernel->machine->WriteRegister(2, i - 1);
      delete buf;
      return;
    }
  }
}
void WriteAtSC()
{
  int virtAddr = kernel->machine->ReadRegister(4);  
  int charcount = kernel->machine->ReadRegister(5); 
  int id = kernel->machine->ReadRegister(6);      
  int pos = kernel->machine->ReadRegister(7);      
  int OldPos;
  int NewPos;
  char *buf;
  buf = User2System(virtAddr, charcount);
  // Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
  if (id < 0 || id > 9)
  {
    printf("\nKhong the write vi id nam ngoai bang mo ta file.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  // Kiem tra file co ton tai khong
  if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id] == NULL)
  {
    printf("\nKhong the write vi file nay khong ton tai.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->type == 1)
  {
    printf("\nKhong the write file stdin hoac file only read.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  OldPos = pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->GetCurrentPos(); // Kiem tra thanh cong thi lay vi tri OldPos
                                                                                           
  //Xet truong hop ghi file read & write 
  if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->type == 0)
  {
    //WriteAt chi ap dung cho File, thuong dung kem voi seek
    if ((pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->WriteAt(buf, charcount,pos)) > 0)
    {
      NewPos = pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->GetCurrentPos();
      kernel->machine->WriteRegister(2, NewPos - OldPos);
      delete buf;
      return;
    }
    if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->type == 0)
    {
      int i = 0;
      while (buf[i] != 0 && buf[i] != '\n') 
      {
        kernel->synchConsoleOut->PutChar(buf[i]);
        i++;
      }
      buf[i] = '\n';
      kernel->synchConsoleOut->PutChar(buf[i]);
      kernel->machine->WriteRegister(2, i - 1); 
      delete buf;
      return;
    }
  }
}


void ExecSC()
{
  int virtAddr;
  char *name;
  virtAddr = kernel->machine->ReadRegister(4);
  

  name = User2System(virtAddr, 255);
  if (name == NULL)
  {
    printf("\n Khong du bo nho trong he thong.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  OpenFile *oFile = pTab->GetPCB(kernel->currentThread->processID)->Open(name);
  if (oFile == NULL)
  {
    printf("\nKhong mo duoc file.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }

  delete oFile;

  //Tra ve id tien trinh con
  int id = pTab->ExecUpdate(name);
  kernel->machine->WriteRegister(2, id);
  delete[] name;
  return;
}
void JoinSC()
{
  int id = kernel->machine->ReadRegister(4);
  //Join tien trinh
  int res = pTab->JoinUpdate(id);
  kernel->machine->WriteRegister(2, res);
  return;
}
void ExitSC()
{
  int exitStatus = kernel->machine->ReadRegister(4);

  if (exitStatus != 0)
  {
    return;
  }

  int res = pTab->ExitUpdate(exitStatus);
  //Giai phong thread
  kernel->currentThread->FreeSpace();
  kernel->currentThread->Finish();
  return;
}
void CreateSemaphoreSC()
{
  int virtAddr = kernel->machine->ReadRegister(4);
  int semval = kernel->machine->ReadRegister(5);

  char *name = User2System(virtAddr, 255);
  if (name == NULL)
  {
    DEBUG('a', "\n Not enough memory in System");
    printf("\n Not enough memory in System");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    return;
  }

  int res = semTab->Create(name, semval);

  if (res == -1)
  {
    DEBUG('a', "\n Khong the khoi tao semaphore");
    printf("\n Khong the khoi tao semaphore");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    return;
  }
  delete[] name;
  kernel->machine->WriteRegister(2, res);
  return;
}
void WaitSC()
{
  // int Wait(char* name)
  int virtAddr = kernel->machine->ReadRegister(4);

  char *name = User2System(virtAddr, 255);
  if (name == NULL)
  {
    DEBUG('a', "\n Not enough memory in System");
    printf("\n Not enough memory in System");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    return;
  }

  int res = semTab->Wait(name);

  if (res == -1)
  {
    DEBUG('a', "\n Khong ton tai ten semaphore nay!");
    printf("\n Khong ton tai ten semaphore nay!");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    return;
  }

  delete[] name;
  kernel->machine->WriteRegister(2, res);
  return;
}
void SignalSC()
{
  int virtAddr = kernel->machine->ReadRegister(4);

  char *name = User2System(virtAddr, 255);
  if (name == NULL)
  {
    printf("\n Khong du bo nho trong he thong.");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    return;
  }

  int res = semTab->Signal(name);
  if (res == -1)
  {
    printf("\n Khong ton tai ten semaphore nay!");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    return;
  }

  delete[] name;
  kernel->machine->WriteRegister(2, res);
  return;
}
void SeekSC()
{
  int pos = kernel->machine->ReadRegister(4); 
  int id = kernel->machine->ReadRegister(5); 
  if (id < 0 || id > 9)
  {
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id] == NULL)
  {
    printf("\nKhong the seek vi file nay khong ton tai.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  // Co seek tren console hay khong
  if (id == 0 || id == 1)
  {
    printf("\nKhong the seek tren file console.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  pos = (pos == -1) ? pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->Length() : pos;
  if (pos > pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->Length() || pos < 0) // Kiem tra lai vi tri pos co hop le khong
  {
    printf("\nKhong the seek file den vi tri nay.");
    kernel->machine->WriteRegister(2, -1);
  }
  else
  {
    // Vi tri di chuyen thuc su trong file
    pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->Seek(pos);
    kernel->machine->WriteRegister(2, pos);
  }
  return;
}
void CurrentThreadIDSC(){
  kernel->machine->WriteRegister(2, kernel->currentThread->processID);
  return;
}

#endif /* ! __USERPROG_KSYSCALL_H__ */
