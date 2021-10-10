// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "synchconsole.h"
#include "machine.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------

char* User2System(int virtAddr, int limit)
{
	int i;// index
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit + 1];//need for terminal string
	if (kernelBuf == NULL)
		return kernelBuf;
	memset(kernelBuf, 0, limit + 1);
	//printf("\n Filename u2s:");
	for (i = 0; i < limit; i++)
	{
		kernel->machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		//printf("%c",kernelBuf[i]);
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

int System2User(int virtAddr, int len, char* buffer)
{
	if (len < 0) return -1;
	if (len == 0)return len;
	int i = 0;
	int oneChar = 0;
	do {
		oneChar = (int)buffer[i];
		kernel->machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}

//Increase Program Counter
void increasePC(){
	/* set previous programm counter (debugging only)*/
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
	
	/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
						  
	/* set next programm counter for brach execution */
	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
}

void
ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
			case NoException:
				SysHalt();
				return;
			
			case PageFaultException:
				printf("\nPage Fault Exception.\n");
				ASSERT(false);
				break;

			case ReadOnlyException:
				printf("\nReadOnly Exception.\n");
				ASSERT(false);
				break;

			case BusErrorException:
				printf("\nBus error exception.\n");
				ASSERT(false);
				break;

		  case AddressErrorException:
				printf("\nAddress error exception.\n");
				ASSERT(false);
				break;
			
			case OverflowException:
				printf("\nOverflow Exception.\n");
				ASSERT(false);
				break;

			case IllegalInstrException:
				printf("\nIllegal Instr Exception.\n");
				ASSERT(false);
				break;
			
			case NumExceptionTypes:
				printf("\nNum Exception Types.\n");
				ASSERT(false);
				break;

    	case SyscallException:
    	  switch(type) {
    	  	case SC_Halt:
						DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

						SysHalt();

						ASSERTNOTREACHED();
						break;

  				case SC_Add:
						DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
						printf("%d + %d", kernel->machine->ReadRegister(4), kernel->machine->ReadRegister(5));
						/* Process SysAdd Systemcall*/
						int result;
						result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
								/* int op2 */(int)kernel->machine->ReadRegister(5));
	
						DEBUG(dbgSys, "Add returning with " << result << "\n");
						/* Prepare Result */
						kernel->machine->WriteRegister(2, (int)result);
						
						/* Modify return point */
						increasePC();
	
						return;
						
						ASSERTNOTREACHED();
	
						break;
				
				case SC_ReadNum: {
						char ch = 0; 
						int ret = 0;
						bool flag = true;
						bool sign = false;
						for (int i = 0; i < 10; i++) {
							ch = (char)kernel->synchConsoleIn->GetChar();
							if (i == 0 && ch == '-') {
								sign = true;
								continue;
							}
							if (ch == 10) break;
							if ('1' <= ch && ch <= '9') {
								int temp = ret;
								ret = ret * 10 + ch - '0';
								if (temp > ret) {
									printf("exception");
									ExceptionHandler(OverflowException);
								}
							}
							else flag = false;
						}
						if (sign == true) ret *= -1;
						kernel->machine->WriteRegister(2,(flag)?(int)ret:0);
						increasePC();
				
						return;

						ASSERTNOTREACHED();
						break;
				}

				case SC_PrintNum: {
					int num = (int)kernel->machine->ReadRegister(4);
					bool sign = (num < 0);
					char a[10];
					if (num == 0) kernel->synchConsoleOut->PutChar('0');
					else {
						size_t len = 0;
						if (num < 0) num *= -1;
						while (num != 0) {
							a[len++] = (num % 10 + '0');
							num /= 10;
						}
						if (sign) kernel->synchConsoleOut->PutChar('-');
						for (int i = (int)len - 1; i >= 0; i--) {
							kernel->synchConsoleOut->PutChar(char(a[i]));
						}
					}
					increasePC();
					return;

					ASSERTNOTREACHED();
					break;
				}

				case SC_ReadChar:
						char ch;
						ch = (char)kernel->synchConsoleIn->GetChar();
						kernel->machine->WriteRegister(2,(int)ch);
						increasePC();
						return;
						
						ASSERTNOTREACHED();
						break;
				case SC_PrintChar:
						char ch_print;
						ch_print = (char)kernel->machine->ReadRegister(4);
						kernel->synchConsoleOut->PutChar(ch_print);
						increasePC();
						return;
						
						ASSERTNOTREACHED();
						break;
						//Day la vi du Random Number
				case SC_RandomNum:
						int r;
						RandomInit((unsigned)time(NULL));
						r = RandomNumber();
						DEBUG(dbgSys, "Random with number " << r << "\n");
						kernel->machine->WriteRegister(2,(int)r);
						increasePC();
						return;

						ASSERTNOTREACHED();
						break;
				case SC_ReadString:
				{
					int virtAddrRead, lengthRead;
					char* bufferRead;
					virtAddrRead = kernel->machine->ReadRegister(4);
					lengthRead = kernel->machine->ReadRegister(5);
					bufferRead = User2System(virtAddrRead, lengthRead);
					//gSynchConsole->Read(buffer, length);
					System2User(virtAddrRead, lengthRead, bufferRead);
					delete[] bufferRead; 
					increasePC();
					return;
				}
				case SC_PrintString:
				{
					int virtAddrWrite;
					char* bufferWrite;
					virtAddrWrite = kernel->machine->ReadRegister(4);
					bufferWrite = User2System(virtAddrWrite, 255);
					int lengthWrite = 0;
					while (bufferWrite[lengthWrite] != 0) lengthWrite++;
					//gSynchConsole->Write(buffer, lengthWrite + 1);
					delete[] bufferWrite; 
					increasePC();
					break;
				}  				default:
						cerr << "Unexpected system call " << type << "\n";
						break;
  			}
  			break;
  		default:
  			cerr << "Unexpected user mode exception" << (int)which << "\n";
  			break;
  	}
		ASSERTNOTREACHED();
}
