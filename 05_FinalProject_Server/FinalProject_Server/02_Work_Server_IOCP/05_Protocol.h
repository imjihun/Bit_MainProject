#pragma once
#include "../CMD.h"

#define CNT_THREAD				17
#define SIZE_HEADER_CMD			2
#define SIZE_HEADER_LENGTH		2
#define SIZE_HEADER				SIZE_HEADER_CMD + SIZE_HEADER_LENGTH
#define TYPE_HEADER_LENGTH		unsigned short
#define TYPE_HEADER_CMD			unsigned short

#define SIZE_BUF	(unsigned int)1024 * 1024 * 4
#define SIZE_THREAD_BUF 1024 * 1024 * 4

#define DB_SUCCESS		100
#define DB_FAIL			-100
#define FAIL_CMD			-101

#define EXSIST_SEND_BUF			1
#define NOT_EXSIST_SEND_BUF		0
// CMD(2bytes) : Length(2bytes) : Data(Length(1bytes) : Data(0~255bytes) :.....)


#define MAX_ROOMCNT 60
#define MAX_MEMBERCNT_INROOM 16

class SOCKETINFO
{
private:
	class uint
	{
	private:
		unsigned int value;
	public:
		uint()
		{
			value = 0;
		}
		uint(unsigned int u)
		{
			(*this) = u;
		}
		void operator=(unsigned int u)
		{
			value = u;
			while (value > SIZE_BUF)
				value -= SIZE_BUF;
		}
		unsigned int operator+(unsigned int u)
		{
			unsigned int tmp = value + u;
			while (tmp > SIZE_BUF)
				tmp -= SIZE_BUF;
			return tmp;
		}
		unsigned int operator+(uint U)
		{
			return U + value;
		}
		void operator+=(unsigned int u)
		{
			(*this) = (*this) + u;
		}
		friend unsigned int operator+(unsigned int u, uint U)
		{
			return U + u;
		}
		friend char* operator+(char *ptr, uint U)
		{
			return (U.value >= SIZE_BUF ? U.value - SIZE_BUF : U.value) + ptr;
		}

		unsigned int operator-(unsigned int u)
		{
			unsigned int tmp = value - u;
			while (tmp > SIZE_BUF)
				tmp += SIZE_BUF;
			return tmp;
		}
		unsigned int operator-(unsigned short us)
		{
			return (*this) - (unsigned int)us;
		}
		unsigned int operator-(uint U)
		{
			return (*this) - U.value;
		}
		void operator-=(unsigned int u)
		{
			(*this) = (*this) - u;
		}
		friend unsigned int operator-(unsigned int u, uint U)
		{
			unsigned int tmp = u - U.value;
			while (tmp > SIZE_BUF)
				tmp += SIZE_BUF;
			return tmp;
		}
		friend char* operator-(char *ptr, uint U)
		{
			return ptr - (U.value >= SIZE_BUF ? U.value - SIZE_BUF : U.value);
		}

		bool operator!=(uint U)
		{
			return (this->value != U.value);
		}
		operator unsigned int()
		{
			return value;
		}
	};

	HANDLE hMutex;
public:
	SOCKET sock;

	WSAOVERLAPPED overlappedRecv;
	char recvBuf[SIZE_BUF];
	WSABUF wsaRecvBuf;
	uint recievedbytes;

	WSAOVERLAPPED overlappedSend;
	char sendBuf[SIZE_BUF];
	WSABUF wsaSendBuf;
	uint willSendbytes;
	uint sentbytes;

	SOCKETINFO **pptr;

	SOCKETINFO(SOCKET client_sock, SOCKETINFO **ptr)
	{
		this->sock = client_sock;

		ZeroMemory(&(this->overlappedRecv), sizeof(this->overlappedRecv));
		this->wsaRecvBuf.buf = this->recvBuf;
		this->wsaRecvBuf.len = SIZE_BUF - 1;
		this->recievedbytes = 0;

		ZeroMemory(&(this->overlappedSend), sizeof(this->overlappedSend));

		InitLock();
		Lock();
		this->wsaSendBuf.buf = this->sendBuf;
		this->wsaSendBuf.len = SIZE_BUF - 1;
		this->willSendbytes = SIZE_BUF;
		this->sentbytes = SIZE_BUF;
		UnLock();
		pptr = ptr;
	}
	void Update_SendBuf_FromOthreThread(HANDLE hcp, char *BufAdding, unsigned int LenAdding)
	{
		Lock();
		memcpy(sendBuf + willSendbytes, BufAdding, LenAdding);
		willSendbytes += LenAdding;
		//PostQueuedCompletionStatus(hcp, SIZE_BUF, (DWORD)this, &this->overlappedSend);
		UnLock();
	}
	void Update_SendBuf(HANDLE hcp, char *BufAdding, unsigned int LenAdding)
	{
		Lock();
		memcpy(sendBuf + willSendbytes, BufAdding, LenAdding);
		willSendbytes += LenAdding;
		PostQueuedCompletionStatus(hcp, SIZE_BUF, (DWORD)this->pptr, &this->overlappedSend);
		UnLock();
	}
	void InitLock()
	{
		hMutex = CreateMutex(NULL, FALSE, NULL);
	}
	void DelLock()
	{
		CloseHandle(hMutex);
	}
	void Lock()
	{
		//printf("______[%x]Lock()\n", this);
		WaitForSingleObject(hMutex, 3000);
		//printf("____[%x]Locked()\n", this);
	}
	void UnLock()
	{
		//printf("######[%x]UnLock()\n", this);
		ReleaseMutex(hMutex);
		//printf("####[%x]UnLocked()\n", this);
	}
};
