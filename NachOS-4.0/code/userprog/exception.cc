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
				case SC_RandomNum:
						int r;
						srand(time(NULL));
						r = rand();
						printf("%d",r);
						kernel->machine->WriteRegister(2,(int)r);
						increasePC();
						return;
						ASSERTNOTREACHED();
						break;
  				default:
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
