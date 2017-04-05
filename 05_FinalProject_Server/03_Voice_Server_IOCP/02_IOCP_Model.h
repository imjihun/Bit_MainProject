#pragma once

#pragma warning(disable:4996)

#include <WinSock2.h>
#include <process.h>

#include "../DebugLog.h"
#include "05_Protocol.h"
#include "03_Voice_Server.h"

void Main_IOCP_Model(SOCKET &listenSock);
//void Req_WSASend(SOCKETINFO *ptr);