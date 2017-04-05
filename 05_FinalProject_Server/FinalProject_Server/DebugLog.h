
#ifndef _DEBUG_LOG_
#define _DEBUG_LOG_

#include <time.h>
#include <stdio.h>
#include <Windows.h>

#define _DEBUG_OUTPUT_
//#define _DEBUG_LOGFILE_

void DebugMessage(char *buf);

void ViewMessage(char *format, ...);

void ViewError(char *format, ...);

void ViewError(int errorcode);

#endif /*_DEBUG_LOG_*/