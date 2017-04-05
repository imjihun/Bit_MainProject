#include <afxdb.h>
#include <WinSock2.h>
#include <stdio.h>

#include "../DebugLog.h"
#include "02_ConnectDB.h"
#include "03_IOCP_Model.h"

#pragma comment(lib, "ws2_32")


void ServerInit(SOCKET &listenSock);
inline void ServerRun(SOCKET &listenSock);

int PORT = 8000;

void Main_TCP_Server(int argc, char **argv)
{
	ViewMessage("TCP_Server Start");
	if (argc == 2)
		PORT = atoi(argv[1]);

	WSADATA wsadata;
	SOCKET listenSock;

	if (WSAStartup(MAKEWORD(2, 2), &wsadata))
		ViewError("WSAStartup() error");

	ServerInit(listenSock);

	ServerRun(listenSock);

	WSACleanup();
}
inline void ServerRun(SOCKET &listenSock)
{
	ViewMessage("TCP_Server Run!!");
	Main_IOCP_Model(listenSock);
	closesocket(listenSock);
}

void ServerInit(SOCKET &listenSock)
{
	SOCKADDR_IN serverAddr;
	BOOL optVal = TRUE;
	if ((listenSock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		ViewError("socket() error");

	if (setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (char *)&optVal, sizeof(optVal)) == SOCKET_ERROR)
		ViewError("setsockopt() error");

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(PORT);
	if (bind(listenSock, (SOCKADDR *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		ViewError("bind() error");

	if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR)
		ViewError("listen() error");
}