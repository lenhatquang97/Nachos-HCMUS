// main.h 
//	This file defines the Nachos global variables
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef MAIN_H
#define MAIN_H

#include "stable.h"
#include "copyright.h"
#include "debug.h"
#include "kernel.h"
#include "ptable.h"
#include "bitmap.h"

class PTable;

extern Kernel *kernel;
extern Debug *debug;
extern Bitmap *gPhysPageBitMap;
extern Semaphore* addrLock;
extern PTable* pTab;
extern STable *semTab;
#endif // MAIN_H
