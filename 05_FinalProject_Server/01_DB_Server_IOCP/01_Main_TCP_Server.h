#pragma once
//#pragma warning(disable:4996)

#include <afxdb.h>
#include <WinSock2.h>
#include <stdio.h>
#include <direct.h>
#include <errno.h>

#include "../DebugLog.h"

#pragma comment(lib, "ws2_32")

void Main_TCP_Server(int argc, char **argv);