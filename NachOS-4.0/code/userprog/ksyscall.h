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
  int freeSlot = kernel->fileSystem->FindFreeSlot();
  if (freeSlot != -1)
  {
    if (type == 0 || type == 1)
    {
      if ((kernel->fileSystem->openf[freeSlot] = kernel->fileSystem->Open(tempWrite, type)) != NULL)
      {
        kernel->machine->WriteRegister(2, freeSlot); //tra ve OpenFileID
        delete[] tempWrite;
      }
      else if (type == 2) // xu li stdin voi type quy uoc la 2
      {
        kernel->machine->WriteRegister(2, 0); //tra ve OpenFileID
        delete[] tempWrite;
      }
      else // xu li stdout voi type quy uoc la 3
      {
        kernel->machine->WriteRegister(2, 1); //tra ve OpenFileID
        delete[] tempWrite;
      }
    }
  }
  kernel->machine->WriteRegister(2, -1);
  delete[] tempWrite;
}
void CloseSC()
{
  OpenFileId fid = kernel->machine->ReadRegister(4);
  if (fid >= 0 && fid <= 14) //Chi xu li khi fid nam trong [0, 14]
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
void ReadSC()
{
  int virtAddr = kernel->machine->ReadRegister(4);  // Lay dia chi cua tham so buffer tu thanh ghi so 4
  int charcount = kernel->machine->ReadRegister(5); // Lay charcount tu thanh ghi so 5
  int id = kernel->machine->ReadRegister(6);     
  int OldPos;
  int NewPos;
  char *buf;
  // Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
  if (id < 0 || id > 14)
  {
    printf("\nKhong the read vi id nam ngoai bang mo ta file.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  // Kiem tra file co ton tai khong
  if (kernel->fileSystem->openf[id] == NULL)
  {
    printf("\nKhong the read vi file nay khong ton tai.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  if (kernel->fileSystem->openf[id]->type == 3) // Xet truong hop doc file stdout (type quy uoc la 3) thi tra ve -1
  {
    printf("\nKhong the read file stdout.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  OldPos = kernel->fileSystem->openf[id]->GetCurrentPos(); // Kiem tra thanh cong thi lay vi tri OldPos
  buf = User2System(virtAddr, charcount);                  // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai charcount
  // Xet truong hop doc file stdin (type quy uoc la 2)
  if (kernel->fileSystem->openf[id]->type == 2)
  {
    // Su dung ham Read cua lop SynchConsole de tra ve so byte thuc su doc duoc
    //int size = gSynchConsole->Read(buf, charcount);
    int size = System2User(virtAddr, charcount, buf); // Copy chuoi tu vung nho System Space sang User Space voi bo dem buffer co do dai la so byte thuc su
    kernel->machine->WriteRegister(2, size);          // Tra ve so byte thuc su doc duoc
    delete buf;
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
  // Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
  if (id < 0 || id > 14)
  {
    printf("\nKhong the write vi id nam ngoai bang mo ta file.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  // Kiem tra file co ton tai khong
  if (kernel->fileSystem->openf[id] == NULL)
  {
    printf("\nKhong the write vi file nay khong ton tai.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  if (kernel->fileSystem->openf[id]->type == 1 || kernel->fileSystem->openf[id]->type == 2)
  {
    printf("\nKhong the write file stdin hoac file only read.");
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  OldPos = kernel->fileSystem->openf[id]->GetCurrentPos(); // Kiem tra thanh cong thi lay vi tri OldPos
  buf = User2System(virtAddr, charcount);  // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai charcount
  //Xet truong hop ghi file read & write (type quy uoc la 0) thi tra ve so byte thuc su
  if (kernel->fileSystem->openf[id]->type == 0)
  {
    if ((kernel->fileSystem->openf[id]->Write(buf, charcount)) > 0)
    {
      // So byte thuc su = NewPos - OldPos
      NewPos = kernel->fileSystem->openf[id]->GetCurrentPos();
      kernel->machine->WriteRegister(2, NewPos - OldPos);
      delete buf;
      return;
    }
  if (kernel->fileSystem->openf[id]->type == 3) // Xet truong hop con lai ghi file stdout (type quy uoc la 3)
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
  
}
void JoinSC()
{
}
void ExitSC()
{
}
void CreateSemaphoreSC()
{
}
void WaitSC()
{
}
void SignalSC()
{
}

#endif /* ! __USERPROG_KSYSCALL_H__ */
