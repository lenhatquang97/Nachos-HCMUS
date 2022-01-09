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
  //printf("\n Filename u2s:");
  for (i = 0; i < limit; i++)
  {
    //Doc 1 ki tu
    kernel->machine->ReadMem(virtAddr + i, 1, &oneChar);
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
      kernel->machine->WriteRegister(2, 1); //tra ve OpenFileID
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
        kernel->machine->WriteRegister(2, freeSlot); //tra ve OpenFileID
        delete[] tempWrite;
        return;
      }
    }
  }
  kernel->machine->WriteRegister(2, -1);
}
void CloseSC()
{
  OpenFileId fid = kernel->machine->ReadRegister(4);
  if (fid >= 0 && fid <= 9) //Chi xu li khi fid nam trong [0, 9]
  {
    if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[fid]) //neu mo file thanh cong
    {
      delete pTab->GetPCB(kernel->currentThread->processID)->fileTable[fid]; //Xoa vung nho luu tru file
      pTab->GetPCB(kernel->currentThread->processID)->fileTable[fid] = NULL; //Gan vung nho NULL
      kernel->machine->WriteRegister(2, 0);
    }
  }
  kernel->machine->WriteRegister(2, -1);
}
void ReadSC()
{
  int virtAddr = kernel->machine->ReadRegister(4);  // Lay dia chi cua tham so buffer tu thanh ghi so 4
  int charcount = kernel->machine->ReadRegister(5); // Lay charcount tu thanh ghi so 5
  int id = kernel->machine->ReadRegister(6);
  int OldPos;
  int NewPos;
  char *buf;
  buf = User2System(virtAddr, charcount);
  // Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
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
  if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->type == 0 && strcmp(buf, "stdout") == 0)
  {
    printf("\nKhong the read file.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  OldPos = pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->GetCurrentPos(); // Kiem tra thanh cong thi lay vi tri OldPos

  if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->type == 1 && strcmp(buf, "stdin") == 0)
  {
    // Su dung ham Read cua lop SynchConsole de tra ve so byte thuc su doc duoc
    //int size = gSynchConsole->Read(buf, charcount);
    int size = System2User(virtAddr, charcount, buf); // Copy chuoi tu vung nho System Space sang User Space voi bo dem buffer co do dai la so byte thuc su
    kernel->machine->WriteRegister(2, size);          // Tra ve so byte thuc su doc duoc
    delete buf;
    return;
  }
  // Xet truong hop doc file binh thuong thi tra ve so byte thuc su
  if ((pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->Read(buf, charcount)) > 0)
  {
    // So byte thuc su = NewPos - OldPos
    NewPos = pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->GetCurrentPos();
    // Copy chuoi tu vung nho System Space sang User Space voi bo dem buffer co do dai la so byte thuc su
    System2User(virtAddr, NewPos - OldPos, buf);
    kernel->machine->WriteRegister(2, NewPos - OldPos);
    delete buf;
    return;
  }
  else
  {
    // Truong hop con lai la doc file co noi dung la NULL tra ve -2
    //printf("\nDoc file rong.");
    kernel->machine->WriteRegister(2, -2);
    delete buf;
    return;
  }

  return;
}
void WriteSC()
{
  int virtAddr = kernel->machine->ReadRegister(4);  // Lay dia chi cua tham so buffer tu thanh ghi so 4
  int charcount = kernel->machine->ReadRegister(5); // Lay charcount tu thanh ghi so 5
  int id = kernel->machine->ReadRegister(6);        // Lay id cua file tu thanh ghi so 6
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
                                                                                           // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai charcount
  //Xet truong hop ghi file read & write (type quy uoc la 0) thi tra ve so byte thuc su
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
      int i = 0;
      while (buf[i] != 0 && buf[i] != '\n') // Vong lap de write den khi gap ky tu '\n'
      {
        kernel->synchConsoleOut->PutChar(buf[i]); // Su dung ham Write cua lop SynchConsole
        i++;
      }
      buf[i] = '\n';
      kernel->synchConsoleOut->PutChar(buf[i]); // Write ky tu '\n'
      kernel->machine->WriteRegister(2, i - 1); // Tra ve so byte thuc su write duoc
      delete buf;
      return;
    }
  }
}
void WriteAtSC()
{
  int virtAddr = kernel->machine->ReadRegister(4);  // Lay dia chi cua tham so buffer tu thanh ghi so 4
  int charcount = kernel->machine->ReadRegister(5); // Lay charcount tu thanh ghi so 5
  int id = kernel->machine->ReadRegister(6);        // Lay id cua file tu thanh ghi so 6
  int pos = kernel->machine->ReadRegister(7);       // Lay pos tu thanh ghi so 7
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
                                                                                           // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai charcount
  //Xet truong hop ghi file read & write (type quy uoc la 0) thi tra ve so byte thuc su
  if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->type == 0)
  {
    if ((pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->WriteAt(buf, charcount,pos)) > 0)
    {
      // So byte thuc su = NewPos - OldPos
      NewPos = pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->GetCurrentPos();
      kernel->machine->WriteRegister(2, NewPos - OldPos);
      delete buf;
      return;
    }
    if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->type == 0)
    {
      int i = 0;
      while (buf[i] != 0 && buf[i] != '\n') // Vong lap de write den khi gap ky tu '\n'
      {
        kernel->synchConsoleOut->PutChar(buf[i]); // Su dung ham Write cua lop SynchConsole
        i++;
      }
      buf[i] = '\n';
      kernel->synchConsoleOut->PutChar(buf[i]); // Write ky tu '\n'
      kernel->machine->WriteRegister(2, i - 1); // Tra ve so byte thuc su write duoc
      delete buf;
      return;
    }
  }
}

//Part 2
void ExecSC()
{
  int virtAddr;
  virtAddr = kernel->machine->ReadRegister(4); // doc dia chi ten chuong trinh tu thanh ghi r4
  char *name;

  name = User2System(virtAddr, 255);
  if (name == NULL)
  {
    DEBUG('a', "\n Not enough memory in System");
    printf("\n Not enough memory in System");
    kernel->machine->WriteRegister(2, -1);
    ////increasePC();
    return;
  }
  OpenFile *oFile = pTab->GetPCB(kernel->currentThread->processID)->Open(name);
  if (oFile == NULL)
  {
    DEBUG('a', "Huhu\n");
    printf("\nExec:: Can't open this file.");
    kernel->machine->WriteRegister(2, -1);
    //increasePC();
    return;
  }
  DEBUG('a', "Thank god\n");

  delete oFile;

  // Return child process id
  int id = pTab->ExecUpdate(name);
  kernel->machine->WriteRegister(2, id);
  //increasePC();
  delete[] name;
  return;
}
void JoinSC()
{
  // int Join(SpaceId id)
  // Input: id dia chi cua thread
  // Output:
  int id = kernel->machine->ReadRegister(4);

  int res = pTab->JoinUpdate(id);

  kernel->machine->WriteRegister(2, res);
  //increasePC();
  return;
}
void ExitSC()
{
  int exitStatus = kernel->machine->ReadRegister(4);

  if (exitStatus != 0)
  {
    //increasePC();
    return;
  }

  int res = pTab->ExitUpdate(exitStatus);
  //machine->WriteRegister(2, res);

  kernel->currentThread->FreeSpace();
  kernel->currentThread->Finish();
  //increasePC();
  return;
}
void CreateSemaphoreSC()
{
  // int CreateSemaphore(char* name, int semval).
  int virtAddr = kernel->machine->ReadRegister(4);
  int semval = kernel->machine->ReadRegister(5);

  char *name = User2System(virtAddr, 255);
  if (name == NULL)
  {
    DEBUG('a', "\n Not enough memory in System");
    printf("\n Not enough memory in System");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    //increasePC();
    return;
  }

  int res = semTab->Create(name, semval);

  if (res == -1)
  {
    DEBUG('a', "\n Khong the khoi tao semaphore");
    printf("\n Khong the khoi tao semaphore");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    //increasePC();
    return;
  }
  delete[] name;
  kernel->machine->WriteRegister(2, res);
  ////increasePC(); 
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
    //increasePC();
    return;
  }

  int res = semTab->Wait(name);

  if (res == -1)
  {
    DEBUG('a', "\n Khong ton tai ten semaphore nay!");
    printf("\n Khong ton tai ten semaphore nay!");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    //increasePC();
    return;
  }

  delete[] name;
  kernel->machine->WriteRegister(2, res);
  //increasePC();
  return;
}
void SignalSC()
{
  int virtAddr = kernel->machine->ReadRegister(4);

  char *name = User2System(virtAddr, 255);
  if (name == NULL)
  {
    DEBUG('a', "\n Not enough memory in System");
    printf("\n Not enough memory in System");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    //increasePC();
    return;
  }

  //printf("\n SignalSC:: name: %s", name);
  int res = semTab->Signal(name);
  if (res == -1)
  {
    DEBUG('a', "\n Khong ton tai ten semaphore nay!");
    printf("\n Khong ton tai ten semaphore nay!");
    kernel->machine->WriteRegister(2, -1);
    delete[] name;
    //increasePC();
    return;
  }

  delete[] name;
  kernel->machine->WriteRegister(2, res);
  //increasePC();
  return;
}
void SeekSC()
{
  int pos = kernel->machine->ReadRegister(4); // Lay vi tri can chuyen con tro den trong file
  int id = kernel->machine->ReadRegister(5);  // Lay id cua file
  printf("\n SeekSC:: pos: %d, id: %d", pos, id);
  // Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
  if (id < 0 || id > 9)
  {
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  // Kiem tra file co ton tai khong
  if (pTab->GetPCB(kernel->currentThread->processID)->fileTable[id] == NULL)
  {
    printf("\nKhong the seek vi file nay khong ton tai.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  // Kiem tra co goi Seek tren console khong
  if (id == 0 || id == 1)
  {
    printf("\nKhong the seek tren file console.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  // Neu pos = -1 thi gan pos = Length nguoc lai thi giu nguyen pos
  pos = (pos == -1) ? pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->Length() : pos;
  if (pos > pTab->GetPCB(kernel->currentThread->processID)->fileTable[id]->Length() || pos < 0) // Kiem tra lai vi tri pos co hop le khong
  {
    printf("\nKhong the seek file den vi tri nay.");
    kernel->machine->WriteRegister(2, -1);
  }
  else
  {
    // Neu hop le thi tra ve vi tri di chuyen thuc su trong file
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
