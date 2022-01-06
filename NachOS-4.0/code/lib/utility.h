// utility.h 
//	Miscellaneous useful definitions.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef UTILITY_H
#define UTILITY_H

#include "copyright.h"

// Miscellaneous useful routines

#define NULL 0
#define TRUE  true
#define FALSE  false
// #define bool int		// necessary on the Mac?

#define min(a,b)  (((a) < (b)) ? (a) : (b))
#define max(a,b)  (((a) > (b)) ? (a) : (b))

// Divide and either round up or down 
#define divRoundDown(n,s)  ((n) / (s))
#define divRoundUp(n,s)    (((n) / (s)) + ((((n) % (s)) > 0) ? 1 : 0))

// This declares the type "VoidFunctionPtr" to be a "pointer to a
// function taking an arbitrary pointer argument and returning nothing".  With
// such a function pointer (say it is "func"), we can call it like this:
//
//	(*func) ("help!");
//
// This is used by Thread::Fork as well as a couple of other places.

typedef void (*VoidFunctionPtr)(void *arg); 
typedef void (*VoidNoArgFunctionPtr)(); 
 

// Include interface that isolates us from the host machine system library.
// Requires definition of bool, and VoidFunctionPtr
#include "sysdep.h"

// Interface to debugging routines.

extern void DebugInit(char *flags); // enable printing debug messages

extern bool DebugIsEnabled(char flag); // Is this debug flag enabled?

extern void DEBUG(char flag, char *format, ...); // Print debug message
                                                 // if flag is enabled

//----------------------------------------------------------------------
// ASSERT
//      If condition is false,  print a message and dump core.
//	Useful for documenting assumptions in the code.
//
//	NOTE: needs to be a #define, to be able to print the location 
//	where the error occurred.
//----------------------------------------------------------------------
#define ASSERT(condition)                                           \
    if (!(condition))                                               \
    {                                                               \
        fprintf(stderr, "Assertion failed: line %d, file \"%s\"\n", \
                __LINE__, __FILE__);                                \
        fflush(stderr);                                             \
        Abort();                                                    \
    }

#endif // UTILITY_H