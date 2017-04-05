#pragma once

#include <afxdb.h>
#include <WinSock2.h>
#include <stdio.h>

#include "../DebugLog.h"

#pragma comment(lib, "ws2_32")

void Main_TCP_Server(int argc, char **argv); 