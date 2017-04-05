#include "02_IOCP_Model.h"

char ThreadBuf[17][SIZE_THREAD_BUF];
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
void IOCP_RecvViewMessage_voice(SOCKETINFO *ptr, int len, int threadNum)
{
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);

	ViewMessage("IOCP[%d] : [TCP/%s:%d] [Recv : %c%c] [len = %u / %u]", threadNum,
		inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port), ptr->recvBuf[0], ptr->recvBuf[1], *(int*)(ptr->recvBuf + 2), len);
}
void IOCP_SendViewMessage_voice(SOCKETINFO *ptr, int len, int threadNum)
{
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);

	ViewMessage("IOCP[%d] : [TCP/%s:%d] [WillSend : %c%c] [len = %u / %u]", threadNum,
		inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port), ptr->wsaSendBuf.buf[0], ptr->wsaSendBuf.buf[1], *(int*)(ptr->wsaSendBuf.buf + 2), len);
}
void CloseSocketInfo(HANDLE hcp, char *threadBuf, SOCKETINFO *ptr, int threadNum)
{
	closesocket((ptr)->sock);
	(ptr)->Lock();
	DeleteVoiceUser(ptr);
	(ptr)->UnLock();
	(ptr)->DelLock();
	delete (ptr->pptr);
	delete (ptr);
	::printf("__________________________________________Closed\n");
}
//
//void Req_WSASend(SOCKETINFO *ptr)
//{
//	int retval;
//
//	ZeroMemory(&(ptr->overlappedSend),
//		sizeof(ptr->overlappedSend));
//	ptr->wsaSendBuf.buf = ptr->sendBuf + ptr->sentbytes;
//	ptr->wsaSendBuf.len = ptr->willSendbytes - ptr->sentbytes;
//	retval = WSASend(ptr->sock, &ptr->wsaSendBuf, 1,
//		NULL, 0, &(ptr->overlappedSend), NULL);
//	if (retval == SOCKET_ERROR)
//	{
//		int a = WSAGetLastError();
//		if (a != WSA_IO_PENDING)
//		{
//			ViewMessage("IOCP : WSASend() error (%d)", a);
//			ViewError(a);
//		}
//	}
//}
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
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			ViewMessage("IOCP : WSARecv() error");
		}
	}
}

void DataProcess(char *threadBuf, HANDLE hcp, DWORD cbTransferred, SOCKETINFO* ptr, LPOVERLAPPED lpResultOverlapped, int threadNum)
{
	int retval = 0;
	unsigned short cmd = 0;
	if (&ptr->overlappedRecv == lpResultOverlapped)
	{
		IOCP_RecvViewMessage_voice(ptr, cbTransferred, threadNum);


		Main_VoiceServerLogic(threadBuf, hcp, cbTransferred, ptr, threadNum);
		Req_WSARecv(ptr);
	}
	else
	{
		ptr->Lock();
		ptr->sentbytes += cbTransferred;
		//if (ptr->GetWillSendbytes() != ptr->GetSentbytes())
		if (ptr->willSendbytes != ptr->sentbytes)
		{
			// ������ �� ������
			Req_WSASend(ptr, threadNum);
			IOCP_SendViewMessage_voice(ptr, ptr->willSendbytes - ptr->sentbytes, threadNum);
		}
		else
		{
			ptr->willSendbytes = SIZE_BUF;
			ptr->sentbytes = SIZE_BUF;
		}
		ptr->UnLock();
	}
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
			EnterCriticalSection(&del_sock_CS);
			if ((*pptr) != NULL)
			{
				IOCP_ViewMessage((*pptr)->sock, "Closed!!");
				if (retval == false && pptr != NULL)
				{
					DWORD temp1, temp2;
					WSAGetOverlappedResult((*pptr)->sock,
						lpResultOverlapped, &temp1, FALSE, &temp2);
					::printf("\t");
					ViewError(WSAGetLastError());
				}
				CloseSocketInfo(hcp, ThreadBuf[ThreadNum], (*pptr), ThreadNum);
				(*pptr) = NULL;
			}
			LeaveCriticalSection(&del_sock_CS);
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
	// Completion Queue ����
	hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hcp == NULL)
		ViewError("IOCP : CreateIoCompletionPort() Error");

	// CPU ���� Ȯ��
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	HANDLE hThread;
	unsigned int ThreadId;

	// CPU�� ����ؼ� �۾�Thread ����
	const int CntThread = (int)si.dwNumberOfProcessors * 2 + 1;
	for (int i = 0; i<CntThread; i++)
	{
		// thread ���� hcp �� �����ϰ� ������� ���ڷ� ��Ʈ�ڵ鷯�� �ش�.
		hThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread,
			hcp, 0, &ThreadId);
		if (hThread == NULL)
			ViewError("IOCP : _beginthreadex() Error");
		// ���̻� Thread handle �� ������ ���ϰڴ�.
		// Thread Routine �� ����Ǹ� �����ض�
		CloseHandle(hThread);
	}
	ViewMessage("IOCP : Thread Count = %d", (int)si.dwNumberOfProcessors * 2 + 1);

	InitVoiceServer();
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

		// ���� ���� ����ü �Ҵ�
		SOCKETINFO **pptr = new SOCKETINFO*();
		*pptr = new SOCKETINFO(client_sock, pptr);
		if (*pptr == NULL)
		{
			ViewMessage("IOCP : [����] �޸𸮰� �����մϴ�");
			break;
		}

		// 1�� ���ڿ� 2�� ���ڸ� �����ϰ�, 3�� ���ڸ� �ְ�޴´�.
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