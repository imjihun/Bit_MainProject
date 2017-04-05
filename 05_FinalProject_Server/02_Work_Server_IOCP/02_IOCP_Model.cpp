#include "02_IOCP_Model.h"

char ThreadBuf[CNT_THREAD][SIZE_THREAD_BUF];
CRITICAL_SECTION del_sock_CS;

void IOCP_ViewMessage(SOCKET sock, int cmd, char *str)
{
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(sock, (SOCKADDR*)&clientaddr, &addrlen);

	ViewMessage("IOCP : [TCP/%s:%d] cmd=%x / %s",
		inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port), cmd, str);
}
void IOCP_ViewMessage(SOCKET sock, char *str)
{
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(sock, (SOCKADDR*)&clientaddr, &addrlen);

	ViewMessage("IOCP : [TCP/%s:%d] %s",
		inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port), str);
}
void IOCP_RecvViewMessage(SOCKETINFO *ptr, unsigned int len, int threadNum)
{
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);

	switch (*(unsigned short *)ptr->wsaSendBuf.buf)
	{
	//case CMD_SAVE_PROFILE_IMAGE:
	//	ViewMessage("IOCP[%d] : [TCP/%s:%d] [Recv : %c%c] [len = %d / %d]", threadNum,
	//		inet_ntoa(clientaddr.sin_addr),
	//		ntohs(clientaddr.sin_port), ptr->recvBuf[0], ptr->recvBuf[1], *(long*)(ptr->recvBuf + 2), len);
	//	break;
	//case CMD_KEY_PACKET:
	//	printf("___________Recv : ");
	//	for (int i = 0; i < ptr->wsaSendBuf.len; i++)
	//	{
	//		printf("%x ", ptr->wsaSendBuf.buf[i]);
	//	}
	//	printf("\n");
	//	break;
	//default:
	//	ViewMessage("IOCP[%d] : [TCP/%s:%d] [Recv : %c%c] [len = %u / %u]", threadNum,
	//		inet_ntoa(clientaddr.sin_addr),
	//		ntohs(clientaddr.sin_port), ptr->recvBuf[0], ptr->recvBuf[1], *(unsigned short*)(ptr->recvBuf + 2), len);
	//	break;
	}
}
void IOCP_SendViewMessage(SOCKETINFO *ptr, unsigned len, int threadNum)
{
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);
	switch (*(unsigned short *)ptr->wsaSendBuf.buf)
	{
	//case CMD_LOAD_PROFILE_IMAGE:
	//	ViewMessage("IOCP[%d] : [TCP/%s:%d] [WillSend : %c%c] [len = %d / %d]", threadNum,
	//		inet_ntoa(clientaddr.sin_addr),
	//		ntohs(clientaddr.sin_port), ptr->wsaSendBuf.buf[0], ptr->wsaSendBuf.buf[1], *(long*)(ptr->wsaSendBuf.buf + 2), len);
	//	break;
	//case CMD_KEY_PACKET:
	//	printf("___________WillSend : ");
	//	for (int i = 0; i < ptr->wsaSendBuf.len; i++)
	//	{
	//		printf("%x ", ptr->wsaSendBuf.buf[i]);
	//	}
	//	printf("\n");
	//	break;
	//default:
	//	ViewMessage("IOCP[%d] : [TCP/%s:%d] [WillSend : %c%c] [len = %u / %u]", threadNum,
	//		inet_ntoa(clientaddr.sin_addr),
	//		ntohs(clientaddr.sin_port), ptr->wsaSendBuf.buf[0], ptr->wsaSendBuf.buf[1], *(unsigned short*)(ptr->wsaSendBuf.buf + 2), len);
	//	break;
	}
}
void CloseSocketInfo(HANDLE hcp, char *threadBuf, SOCKETINFO *ptr, int threadNum)
{
	closesocket((ptr)->sock);
	(ptr)->Lock();
	DisConnectClient(hcp, threadBuf, (ptr), threadNum);
	(ptr)->UnLock();
	(ptr)->DelLock();
	delete (ptr->pptr);
	delete (ptr);
	::printf("__________________________________________Closed\n");
}

void DisConnect_pptr(HANDLE hcp, int threadNum, SOCKETINFO **pptr)
{
	EnterCriticalSection(&del_sock_CS);
	if ((*pptr) != NULL)
	{
		IOCP_ViewMessage((*pptr)->sock, "Closed!!");
		CloseSocketInfo(hcp, ThreadBuf[threadNum], (*pptr), threadNum);
		(*pptr) = NULL;
	}
	LeaveCriticalSection(&del_sock_CS);
}
void Req_WSASend(SOCKETINFO *ptr)
{
	int retval;

	ZeroMemory(&(ptr->overlappedSend),
		sizeof(ptr->overlappedSend));
	ptr->wsaSendBuf.buf = ptr->sendBuf + ptr->sentbytes;
	ptr->wsaSendBuf.len = ptr->willSendbytes - ptr->sentbytes;
	retval = WSASend(ptr->sock, &ptr->wsaSendBuf, 1,
		NULL, 0, &(ptr->overlappedSend), NULL);
	if (retval == SOCKET_ERROR)
	{
		int a = WSAGetLastError();
		if (a != WSA_IO_PENDING)
		{
			ViewMessage("IOCP : WSASend() error (%d)", a);
			ViewError(a);
		}
	}
	PrintPacket(ptr, ptr->sendBuf, *(TYPE_HEADER_LENGTH *)(ptr->sendBuf + SIZE_HEADER_LENGTH), "Send");
}
void Req_WSARecv(SOCKETINFO *ptr, HANDLE hcp, int threadNum)
{
	int retval;

	ZeroMemory(&(ptr->overlappedRecv),
		sizeof(ptr->overlappedRecv));
	ptr->wsaRecvBuf.buf = ptr->recvBuf + ptr->recievedbytes;
	ptr->wsaRecvBuf.len = SIZE_BUF - 1 - ptr->recievedbytes;

	DWORD flags = 0;
	retval = WSARecv(ptr->sock, &(ptr->wsaRecvBuf), 1,
		NULL, &flags, &(ptr->overlappedRecv), NULL);
	if (retval == SOCKET_ERROR)
	{
		int a = WSAGetLastError();
		if (a != WSA_IO_PENDING)
		{
			ViewMessage("IOCP : WSARecv() error (%d)", a);
			ViewError(a);
			//DisConnect_pptr(hcp, threadNum, ptr->pptr);
		}
	}
}
void Req_WSARecv(SOCKETINFO *ptr)
{
	int retval;

	ZeroMemory(&(ptr->overlappedRecv),
		sizeof(ptr->overlappedRecv));
	ptr->wsaRecvBuf.buf = ptr->recvBuf + ptr->recievedbytes;
	ptr->wsaRecvBuf.len = SIZE_BUF - 1 - ptr->recievedbytes;

	DWORD flags = 0;
	retval = WSARecv(ptr->sock, &(ptr->wsaRecvBuf), 1,
		NULL, &flags, &(ptr->overlappedRecv), NULL);
	if (retval == SOCKET_ERROR)
	{
		int a = WSAGetLastError();
		if (a != WSA_IO_PENDING)
		{
			ViewMessage("IOCP : WSARecv() error (%d)", a);
			ViewError(a);
		}
	}
}

void DataProcess(char *threadBuf, HANDLE hcp, DWORD cbTransferred, SOCKETINFO* ptr, LPOVERLAPPED lpResultOverlapped, int threadNum)
{
	int retval = 0;
	unsigned short cmd = 0;
	ptr->Lock();
	if (&ptr->overlappedRecv == lpResultOverlapped)
	{
		IOCP_RecvViewMessage(ptr, cbTransferred, threadNum);


		Main_WorkServerLogic(threadBuf, hcp, cbTransferred, ptr, threadNum);

		Req_WSARecv(ptr, hcp, threadNum);
	}
	else
	{
		ptr->sentbytes += cbTransferred;
		//if (ptr->GetWillSendbytes() != ptr->GetSentbytes())
		if (ptr->willSendbytes != ptr->sentbytes)
		{
			// 데이터 더 보내기
			Req_WSASend(ptr);
			IOCP_SendViewMessage(ptr, ptr->willSendbytes - ptr->sentbytes, threadNum);
		}
		else
		{
			ptr->willSendbytes = SIZE_BUF;
			ptr->sentbytes = SIZE_BUF;
		}
	}
	ptr->UnLock();
}


unsigned int CALLBACK WorkerThread(LPVOID arg)
{
	HANDLE hcp = (HANDLE)arg;
	int retval;
	static int threadCnt = 0;
	int ThreadNum = threadCnt++;
	while (1)
	{
		DWORD cbTransferred;
		LPOVERLAPPED lpResultOverlapped;
		SOCKETINFO** pptr = NULL;

		retval = GetQueuedCompletionStatus(
			hcp, &cbTransferred,
			(LPDWORD)&pptr,
			&lpResultOverlapped, INFINITE);

		if ((retval == false || cbTransferred == 0) && lpResultOverlapped != NULL)
		{
			//EnterCriticalSection(&del_sock_CS);
			//if ((*pptr) != NULL)
			//{
			//	IOCP_ViewMessage((*pptr)->sock, "Closed!!");
			//	if (retval == false && pptr != NULL)
			//	{
			//		DWORD temp1, temp2;
			//		WSAGetOverlappedResult((*pptr)->sock,
			//			lpResultOverlapped, &temp1, FALSE, &temp2);
			//		::printf("\t");
			//		ViewError(WSAGetLastError());
			//	}
			//	CloseSocketInfo(hcp, ThreadBuf[ThreadNum], (*pptr), ThreadNum);
			//	(*pptr) = NULL;
			//}
			//LeaveCriticalSection(&del_sock_CS);
			DisConnect_pptr(hcp, ThreadNum, pptr);
			continue;
		}
		else if (lpResultOverlapped == NULL)
		{
			::printf("Out OverlappedIO* is Null\n");
			continue;
		}
		DataProcess(ThreadBuf[ThreadNum], hcp, cbTransferred, (*pptr), lpResultOverlapped, ThreadNum);
	}
	return 0;
}
void Init_IOCP_Model(HANDLE &hcp)
{
	// Completion Queue 생성
	hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hcp == NULL)
		ViewError("IOCP : CreateIoCompletionPort() Error");

	// CPU 개수 확인
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	HANDLE hThread;
	unsigned int ThreadId;

	// CPU에 비례해서 작업Thread 생성
	for (int i = 0; i<(int)si.dwNumberOfProcessors * 2 + 1; i++)
	{
		// thread 들이 hcp 를 감시하고 있으라고 인자로 포트핸들러를 준다.
		hThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread,
			hcp, 0, &ThreadId);
		if (hThread == NULL)
			ViewError("IOCP : _beginthreadex() Error");
		// 더이상 Thread handle 로 접근을 안하겠다.
		// Thread Routine 이 종료되면 종료해라
		CloseHandle(hThread);
	}
	ViewMessage("IOCP : Thread Count = %d", (int)si.dwNumberOfProcessors * 2 + 1);

	Init_WorkServer();
	InitializeCriticalSection(&del_sock_CS);
}
void Run_IOCP_Model(SOCKET &listenSock, HANDLE &hcp)
{
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	SOCKET client_sock;

	ViewMessage("IOCP : IOCP_Model Run!!!");

	while (1)
	{
		client_sock = accept(listenSock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
		{
			ViewMessage("IOCP : accept() error");
			continue;
		}
		IOCP_ViewMessage(client_sock, "Connected!!");

		// 소켓 정보 구조체 할당
		SOCKETINFO **pptr = new SOCKETINFO*();
		*pptr = new SOCKETINFO(client_sock, pptr);
		if (*pptr == NULL)
		{
			ViewMessage("IOCP : [오류] 메모리가 부족합니다");
			break;
		}

		// 1번 인자와 2번 인자를 결합하고, 3번 인자를 주고받는다.
		HANDLE hResult = CreateIoCompletionPort((HANDLE)client_sock, hcp, (DWORD)pptr, 0);
		if (hResult == NULL)
			return;

		Req_WSARecv(*pptr);

		//ptr->sentbytes = SIZE_BUF - 1;
		//PostQueuedCompletionStatus(hcp, 1, (DWORD)ptr, &ptr->overlappedSend);
	}
}

void Main_IOCP_Model(SOCKET &listenSock)
{
	ViewMessage("IOCP : IOCP_Model_Start");

	HANDLE hcp = NULL;
	Init_IOCP_Model(hcp);
	Run_IOCP_Model(listenSock, hcp);
	DeleteCriticalSection(&del_sock_CS);
}