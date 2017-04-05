#pragma once

#pragma warning(disable:4996)

#include <WinSock2.h>
#include <process.h>

#include "../DebugLog.h"
#include "02_ConnectDB.h"
#include "05_Protocol.h"

struct SOCKETINFO
{
	SOCKET sock;

	WSAOVERLAPPED overlappedRecv;
	char recvBuf[SIZE_BUF + 1];
	WSABUF wsaRecvBuf;
	int recievedbytes;

	WSAOVERLAPPED overlappedSend;
	char sendBuf[SIZE_BUF + 1];
	WSABUF wsaSendBuf;
	int willSendbytes;
	int sentbytes;
};

void Main_IOCP_Model(SOCKET &listenSock);
void IOCP_SendViewMessage(SOCKET sock, char *str);
void IOCP_RecvViewMessage(SOCKET sock, char *str);