#pragma once

#ifndef __MACRO_H__
#define __MACRO_H__

#include <stdio.h>

#define __null ((void*)0)

// Color macros (code-style)
#define __red       printf("\033[31m")
#define __yellow    printf("\033[33m")
#define __green     printf("\033[32m")
#define __blue      printf("\033[34m")
#define __cyan      printf("\033[36m")
#define __magenta   printf("\033[35m")
#define __white     printf("\033[37m")
#define __reset     printf("\033[0m")
#define __bold      printf("\033[1m")
#define __underline printf("\033[4m")

// Status macros
#define __ok   printf("\033[32m[  OK  ]\033[0m ")
//#define __info printf("\033[34m[ INFO ]\033[0m ")
#define __warn printf("\033[33m[ WARN ]\033[0m ")
#define __fail printf("\033[31m[FAILED]\033[0m ")

// Clear screen macro
#ifndef __clear
#define __clear printf("\033[2J\033[H")
#endif

// HOS information
#define HOS_VERSION "Alpha v1.2"
#define __AUTHOR__ "Brian Riff"
#define __RELEASE_DATE__ "May 18, 2025"

#endif