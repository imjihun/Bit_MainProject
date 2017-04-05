#include "03_IOCP_Model.h"
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
void IOCP_RecvViewMessage(SOCKET sock, char *str)
{
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(sock, (SOCKADDR*)&clientaddr, &addrlen);

	switch (*(unsigned short *)str)
	{
	case '0' << 8 | 'M':
		ViewMessage("IOCP : [TCP/%s:%d] [Recv : %c%c] p=(%f, %f, %f)",
			inet_ntoa(clientaddr.sin_addr),
			ntohs(clientaddr.sin_port), str[0], str[1], *(float*)(str + 4), *(float*)(str + 8), *(float*)(str + 12));
		break;
	case 'I' << 8 | 'S':
		char id[17];
		memcpy_s(id, 16, str + 7, str[6]);
		id[str[6]] = 0;
		ViewMessage("IOCP : [TCP/%s:%d] [Recv : %c%c] [Len : %d] [ID = %s]",
			inet_ntoa(clientaddr.sin_addr),
			ntohs(clientaddr.sin_port), str[0], str[1], *(long*)(str + 2), id);
		break;
	default:
		//ViewMessage("IOCP : [TCP/%s:%d] [Recv : %c%c] [Len : %d]",
		//	inet_ntoa(clientaddr.sin_addr),
		//	ntohs(clientaddr.sin_port), str[0], str[1], *(unsigned short *)(str + 2));
		ViewMessage("IOCP : [TCP/%s:%d] [Recv : %c%c] [%d/%d]",
			inet_ntoa(clientaddr.sin_addr),
			ntohs(clientaddr.sin_port), str[0], str[1], str[2], str[3]);
		printf("\t\t\t");
		for (int i = 0; i < *(unsigned short *)(str + 2); i++)
			printf(" %x", str[i]);
		printf("\n");
		//char tmp[1024];

		//int idx_buf = SIZE_HEADER;
		//unsigned short lenth = *(unsigned short *)(str + 2);
		//int len = 0;
		//for (int i = 0; idx_buf < lenth; idx_buf += len + 1, i++)
		//{
		//	len = (unsigned char)str[idx_buf];
		//	char *t = str + idx_buf + 1;
		//	memcpy_s(tmp, len, str + idx_buf + 1, len);
		//	tmp[len] = '\0';
		//	printf("[%d:%s]", i, tmp);
		//}
		//printf("\n");
		break;
	}
}
void IOCP_SendViewMessage(SOCKET sock, char *str)
{
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(sock, (SOCKADDR*)&clientaddr, &addrlen);

	switch (*(unsigned short *)str)
	{
	case 'I' << 8 | 'L':
		char id[17];
		memcpy_s(id, 16, str + 7, str[6]);
		id[str[6]] = 0;
		ViewMessage("IOCP : [TCP/%s:%d] [WillSend : %c%c] [Len : %d] [ID = %s]",
			inet_ntoa(clientaddr.sin_addr),
			ntohs(clientaddr.sin_port), str[0], str[1], *(long*)(str + 2), id);
		break;
	case '0' << 8 | 'M':
		ViewMessage("IOCP : [TCP/%s:%d] [WillSend : %c%c] p=(%f, %f, %f)",
			inet_ntoa(clientaddr.sin_addr),
			ntohs(clientaddr.sin_port), str[0], str[1], *(float*)(str + 4), *(float*)(str + 8), *(float*)(str + 12));
		break;
	default:
		//ViewMessage("IOCP : [TCP/%s:%d] [WillSend : %c%c] [Len : %d]",
		//	inet_ntoa(clientaddr.sin_addr),
		//	ntohs(clientaddr.sin_port), str[0], str[1], *(unsigned short *)(str + 2));
		ViewMessage("IOCP : [TCP/%s:%d] [WillSend : %c%c] [%d/%d]",
			inet_ntoa(clientaddr.sin_addr),
			ntohs(clientaddr.sin_port), str[0], str[1], str[2], str[3]);
		printf("\t\t\t");
		for (int i = 0; i < *(unsigned short *)(str + 2); i++)
			printf(" %x", str[i]);
		printf("\n");
		break;
	}
}

void CloseSocketInfo(SOCKETINFO *ptr)
{
	IOCP_ViewMessage(ptr->sock, "Closed!!");
	closesocket(ptr->sock);
	delete ptr;
}
int Req_WSASend(SOCKETINFO *dst, char *pSrcBuf, int lenSrcBuf)
{
	int retval = 0;

	ZeroMemory(&(dst->overlappedSend),
		sizeof(dst->overlappedSend));
	memcpy(dst->sendBuf, pSrcBuf, lenSrcBuf);
	dst->willSendbytes = lenSrcBuf;
	dst->sentbytes = 0;
	dst->wsaSendBuf.buf = dst->sendBuf;
	dst->wsaSendBuf.len = lenSrcBuf;

	//PostQueuedCompletionStatus(hcp, 0, ptr->sock, &ptr->overlappedSend);
	retval = WSASend(dst->sock, &(dst->wsaSendBuf), 1,
		NULL, 0, &(dst->overlappedSend), NULL);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			ViewMessage("IOCP : WSASend() error (Thread)");
		return -1;
	}
	dst->sendBuf[lenSrcBuf] = 0;
	IOCP_SendViewMessage(dst->sock, dst->sendBuf);
	return 0;
}
void PacketProcess(int cbTransferred, SOCKETINFO *ptr)
{
	ptr->recievedbytes += cbTransferred;
	// 받은 데이터 출력
	ptr->recvBuf[ptr->recievedbytes] = 0;
	IOCP_RecvViewMessage(ptr->sock, ptr->recvBuf);
	//cmd = *(unsigned short*)ptr->recvBuf;
	if (ptr->recievedbytes >= SIZE_HEADER)
	{
		unsigned short t1 = *(unsigned short *)ptr->recvBuf;
		unsigned short t2 = CMD_SAVE_PROFILE_IMAGE;
		unsigned short t3 = CMD_LOAD_PROFILE_IMAGE;
		if (t2 == t1)
		{
			if (ptr->recievedbytes >= 6)
			{
				long Len_Pack = *(long *)(ptr->recvBuf + 2);
				if (Len_Pack <= ptr->recievedbytes)
				{
					unsigned short Len_SendBuf;
					if (SaveProfileImage(ptr->recvBuf, Len_Pack, ptr->sendBuf, &Len_SendBuf) == EXSIST_SEND_BUF)
					{
						if (Req_WSASend(ptr, ptr->sendBuf, Len_SendBuf) != 0)
							CloseSocketInfo(ptr);
					}
					ptr->recievedbytes = ptr->recievedbytes - Len_Pack;
					memcpy(ptr->recvBuf, ptr->recvBuf + Len_Pack, ptr->recievedbytes);
				}
			}
		}
		else if (t3 == t1)
		{
			if (ptr->recievedbytes >= 4)
			{
				unsigned short Len_Pack = *(unsigned short *)(ptr->recvBuf + 2);
				if (Len_Pack <= ptr->recievedbytes)
				{
					int Len_SendBuf;
					int retval = GetUserProfileImage(ptr->recvBuf, Len_Pack, ptr->sendBuf, &Len_SendBuf);
					if (retval == EXSIST_SEND_BUF)
					{
						if (Req_WSASend(ptr, ptr->sendBuf, Len_SendBuf) != 0)
							CloseSocketInfo(ptr);
					}
					ptr->recievedbytes = ptr->recievedbytes - Len_Pack;
					memcpy(ptr->recvBuf, ptr->recvBuf + Len_Pack, ptr->recievedbytes);
				}
			}
		}
		else
		{
			unsigned short Len_Pack = *(unsigned short *)(ptr->recvBuf + 2);
			if (Len_Pack <= ptr->recievedbytes)
			{
				unsigned short Len_SendBuf;
				if (MAIN_DB(ptr->recvBuf, Len_Pack, ptr->sendBuf, &Len_SendBuf) == EXSIST_SEND_BUF)
				{
					if (Req_WSASend(ptr, ptr->sendBuf, Len_SendBuf) != 0)
						CloseSocketInfo(ptr);
				}
				ptr->recievedbytes = ptr->recievedbytes - Len_Pack;
				memcpy(ptr->recvBuf, ptr->recvBuf + Len_Pack, ptr->recievedbytes);
			}
		}
	}
}
void DataProcess(DWORD cbTransferred, SOCKETINFO* ptr, LPOVERLAPPED lpResultOverlapped)
{
	int retval = 0;
	unsigned short cmd = 0;
	if (&ptr->overlappedRecv == lpResultOverlapped)
	{
		PacketProcess(cbTransferred, ptr);
		//데이터 받기
		ZeroMemory(&(ptr->overlappedRecv),
			sizeof(ptr->overlappedRecv));
		ptr->wsaRecvBuf.buf = ptr->recvBuf + ptr->recievedbytes;
		ptr->wsaRecvBuf.len = SIZE_BUF - ptr->recievedbytes;

		DWORD flags = 0;
		retval = WSARecv(ptr->sock, &(ptr->wsaRecvBuf), 1,
			NULL, &flags, &(ptr->overlappedRecv), NULL);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
				ViewMessage("IOCP : WSARecv() error");
			return;
		}
	}
	else
	{
		ptr->sentbytes += cbTransferred;
		if (ptr->willSendbytes > ptr->sentbytes)
		{
			// 데이터 더보내기
			ZeroMemory(&(ptr->overlappedSend),
				sizeof(ptr->overlappedSend));
			ptr->wsaSendBuf.buf = ptr->sendBuf + ptr->sentbytes;
			ptr->wsaSendBuf.len = ptr->willSendbytes - ptr->sentbytes;

			retval = WSASend(ptr->sock, &(ptr->wsaSendBuf), 1,
				NULL, 0, &(ptr->overlappedSend), NULL);
			if (retval == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
					ViewMessage("IOCP : WSASend() error");
				return;
			}
			IOCP_SendViewMessage(ptr->sock, ptr->wsaSendBuf.buf);
		}
		else
		{
			ptr->willSendbytes = 0;
			ptr->sentbytes = 0;
		}
	}
}
unsigned int CALLBACK WorkerThread(LPVOID arg)
{
	HANDLE hcp = (HANDLE)arg;
	int retval;

	while (1)
	{
		DWORD cbTransferred;
		LPOVERLAPPED lpResultOverlapped;
		SOCKETINFO* ptr = NULL;

		retval = GetQueuedCompletionStatus(
			hcp, &cbTransferred,
			(LPDWORD)&ptr,
			&lpResultOverlapped, INFINITE);

		if ((retval == false || cbTransferred == 0) && lpResultOverlapped != NULL)
		{
			if (retval == 0)
			{
				DWORD temp1, temp2;
				WSAGetOverlappedResult(ptr->sock,
					lpResultOverlapped, &temp1, FALSE, &temp2);
				printf("\t");
				ViewError(WSAGetLastError());
			}
			CloseSocketInfo(ptr);
			continue;
		}
		else if (lpResultOverlapped == NULL)
		{
			printf("Out OverlappedIO* is Null\n");
			continue;
		}
		DataProcess(cbTransferred, ptr, lpResultOverlapped);
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
}
void Run_IOCP_Model(SOCKET &listenSock, HANDLE &hcp)
{
	int retval;
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
		SOCKETINFO* ptr = new SOCKETINFO;

		// 1번 인자와 2번 인자를 결합하고, 3번 인자를 주고받는다.
		HANDLE hResult = CreateIoCompletionPort((HANDLE)client_sock, hcp, (DWORD)ptr, 0);
		if (hResult == NULL)
			return;


		if (ptr == NULL)
		{
			ViewMessage("IOCP : [오류] 메모리가 부족합니다");
			break;
		}

		ZeroMemory(&(ptr->overlappedRecv), sizeof(ptr->overlappedRecv));
		ZeroMemory(&(ptr->overlappedSend), sizeof(ptr->overlappedSend));
		ptr->sock = client_sock;

		ptr->wsaSendBuf.buf = ptr->sendBuf;
		ptr->wsaSendBuf.len = SIZE_BUF;
		ptr->willSendbytes = 0;
		ptr->sentbytes = 0;

		ptr->wsaRecvBuf.buf = ptr->recvBuf;
		ptr->wsaRecvBuf.len = SIZE_BUF;
		ptr->recievedbytes = 0;

		//비동기 입출력 시작
		DWORD flags = 0;
		// WSARecv() 를 한번 호출해 준다.
		retval = WSARecv(client_sock, &(ptr->wsaRecvBuf), 1, NULL,
			&flags, &(ptr->overlappedRecv), NULL);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
				ViewMessage("IOCP : WSARecv() error (%s)", "MainRoutine");
			continue;
		}
	}
}

void Main_IOCP_Model(SOCKET &listenSock)
{
	ViewMessage("IOCP : IOCP_Model_Start");

	HANDLE hcp = NULL;
	Init_IOCP_Model(hcp);
	Run_IOCP_Model(listenSock, hcp);
}