#pragma once
//#pragma warning(disable:4996)

#include <afxdb.h>
#include <WinSock2.h>
#include <stdio.h>

#include "../DebugLog.h"
//#include "04_EventSelect_Model.h"

#pragma comment(lib, "ws2_32")

void Main_TCP_Server(int argc, char **argv); 