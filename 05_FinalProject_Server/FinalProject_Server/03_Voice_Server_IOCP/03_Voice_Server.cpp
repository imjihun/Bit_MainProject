#include "03_Voice_Server.h"
#pragma region work source

#pragma endregion

#define SIZE_HEADER_VOICE_CMD		2
#define TYPE_HEADER_VOICE_CMD		unsigned short
#define SIZE_HEADER_VOICE_LENGTH	4	
#define TYPE_HEADER_VOICE_LENGTH	int
#define SIZE_HEADER_VOICE			SIZE_HEADER_VOICE_CMD + SIZE_HEADER_VOICE_LENGTH

#define VOICE_FREQUENCY				44100
#define SIZE_VOICE_BUFFER_FLOAT		VOICE_FREQUENCY / 2
#define SIZE_VOICE_BUFFER			SIZE_VOICE_BUFFER_FLOAT * 4

#define TIME_VOICE_BUFFER_MS		(SIZE_VOICE_BUFFER_FLOAT * 1000) / VOICE_FREQUENCY

#pragma comment(lib, "Winmm.lib")
class VoiceData
{
private:
	char Voice_Buffer[SIZE_VOICE_BUFFER];
	CRITICAL_SECTION CS;
public:
	char Id[17];
	int LenId;
	SOCKETINFO *ptr;
	DWORD curr_time;

	VoiceData(char *id, int lenid, SOCKETINFO *p)
	{
		memcpy_s(Id, 16, id, lenid);
		Id[lenid] = 0;
		LenId = lenid;
		curr_time = timeGetTime();
		InitializeCriticalSection(&CS);

		ptr = p;
	}
	~VoiceData()
	{
		DeleteCriticalSection(&CS);
	}
	void SetVoiceData(char *src, int len_src)
	{
		//EnterCriticalSection(&CS);
		memcpy_s(Voice_Buffer, SIZE_VOICE_BUFFER, src, len_src);
		curr_time = timeGetTime();
		//LeaveCriticalSection(&CS);
	}
	int GetVoiceData(char *dst, int len_dst)
	{
		//EnterCriticalSection(&CS);
		if (timeGetTime() - curr_time > TIME_VOICE_BUFFER_MS)
		{
			//LeaveCriticalSection(&CS);
			return -1;
		}

		memcpy_s(dst, len_dst, Voice_Buffer, SIZE_VOICE_BUFFER);
		//LeaveCriticalSection(&CS);
		return 0;
	}
};

char DBIP[20] = "127.0.0.1";
//char DBIP[20] = "14.32.7.30";
int DBPORT = 8000;
SOCKET DBSocket[CNT_THREAD];


std::map<SOCKETINFO*, VoiceData*> mapVoice_ptr;
std::map<string, VoiceData*> mapVoice_id;

void InitVoiceServer()
{
	SOCKADDR_IN DBAddr;
	BOOL optVal = TRUE;
	for (int i = 0; i < CNT_THREAD; i++)
	{
		if ((DBSocket[i] = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
			ViewError("DBsocket() error");

		if (setsockopt(DBSocket[i], SOL_SOCKET, SO_REUSEADDR, (char *)&optVal, sizeof(optVal)) == SOCKET_ERROR)
			ViewError("DBsetsockopt() error");

		memset(&DBAddr, 0, sizeof(DBAddr));
		DBAddr.sin_family = AF_INET;
		DBAddr.sin_addr.s_addr = inet_addr(DBIP);
		DBAddr.sin_port = htons(DBPORT);
		if (connect(DBSocket[i], (SOCKADDR *)&DBAddr, sizeof(DBAddr)) != 0)
			ViewError("DBconnect() error");
	}
}
void InsertVoiceUser(SOCKETINFO *ptr, char *id, int lenid)
{
	if (mapVoice_id.find(id) != mapVoice_id.end())
	{
		mapVoice_ptr.erase(mapVoice_id[id]->ptr);
		mapVoice_id.erase(id);

		printf("__________[Debug : Deleted] Connected User Cnt = %d:%d\n", mapVoice_ptr.size(), mapVoice_id.size());
	}

	mapVoice_ptr[ptr] = new VoiceData(id, lenid, ptr);
	id[lenid] = 0;
	mapVoice_id[id] = mapVoice_ptr[ptr];

	printf("__________[Debug : Inserted] Connected User Cnt = %d:%d //[id=%s, ptr=%x]\n", mapVoice_ptr.size(), mapVoice_id.size(), id, ptr);
}
void DeleteVoiceUser(SOCKETINFO *ptr)
{
	if (mapVoice_ptr.find(ptr) != mapVoice_ptr.end())
	{
		mapVoice_id.erase(mapVoice_ptr[ptr]->Id);
		mapVoice_ptr.erase(ptr);

		printf("__________[Debug : Deleted] Connected User Cnt = %d:%d\n", mapVoice_ptr.size(), mapVoice_id.size());
	}
}
void SetVoiceData(SOCKETINFO *ptr, int threadNum)
{
	if (mapVoice_ptr.find(ptr) != mapVoice_ptr.end())
	{
		int idx_start = SIZE_HEADER_VOICE + mapVoice_ptr[ptr]->LenId + 1;
		mapVoice_ptr[ptr]->SetVoiceData(ptr->recvBuf + idx_start, SIZE_VOICE_BUFFER);
	}
}
void Req_WSASend(SOCKETINFO *ptr, int threadNum)
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

	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);

	ViewMessage("IOCP[%d] : [TCP/%s:%d] [WillSend : %c%c] [len = %u] [s = %u / w = %u]", threadNum,
		inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port), ptr->wsaSendBuf.buf[0], ptr->wsaSendBuf.buf[1], *(TYPE_HEADER_VOICE_LENGTH*)(ptr->wsaSendBuf.buf + SIZE_HEADER_CMD), ptr->sentbytes, ptr->willSendbytes);

}

void Req_DB(char *src, int len, char *dst, int dstSize, int threadNum)
{
	send(DBSocket[threadNum], src, len, 0);
	recv(DBSocket[threadNum], dst, dstSize, 0);
}
int ParseRecvBuf_DBRecv(char *src, char **dst)
{
	int idx_buf = SIZE_HEADER;
	TYPE_HEADER_LENGTH lenth = *(TYPE_HEADER_LENGTH *)(src + 2);
	int len = 0;
	int i = 0;
	for (i = 0; idx_buf < lenth; idx_buf += len + 1)
	{
		len = (unsigned char)src[idx_buf];
		src[idx_buf] = '\0';
		dst[i++] = &src[idx_buf + 1];
	}
	src[idx_buf] = '\0';
	return i;
}
void Send_Voice_Data(SOCKETINFO *ptr, int threadNum, char *threadBuf)
{
	TYPE_HEADER_CMD cmd = CMD_GET_USER_LIST_INROOM;
	memcpy_s(threadBuf, SIZE_THREAD_BUF, &cmd, 2);

	TYPE_HEADER_LENGTH idx = SIZE_HEADER;
	threadBuf[idx++] = mapVoice_ptr[ptr]->LenId;
	memcpy_s(threadBuf + idx, SIZE_THREAD_BUF, mapVoice_ptr[ptr]->Id, mapVoice_ptr[ptr]->LenId);
	idx += mapVoice_ptr[ptr]->LenId;

	memcpy_s(threadBuf + 2, SIZE_THREAD_BUF, &idx, 2);

	Req_DB(threadBuf, idx, threadBuf, SIZE_THREAD_BUF, threadNum);
	char *id_arr[MAX_MEMBERCNT_INROOM];
	int cntId = ParseRecvBuf_DBRecv(threadBuf, id_arr);
	//////////////////////////////////////////////////////////////
	// 해당 방안에 유저아이디 얻어옴. : id_arr / cntId
	//////////////////////////////////////////////////////////////
	{
		char *sendptr = ptr->sendBuf + ptr->willSendbytes;
		ptr->Lock();
		TYPE_HEADER_VOICE_CMD cmd = CMD_GET_VOICE_PACKET;
		memcpy(sendptr, &cmd, SIZE_HEADER_VOICE_CMD);

		TYPE_HEADER_VOICE_LENGTH idx_SendBuf = ptr->sentbytes + (unsigned int)SIZE_HEADER_VOICE;
		for (int i = 0; i < cntId; i++)
		{
			if (mapVoice_id.find(id_arr[i]) == mapVoice_id.end()
				|| (ptr == mapVoice_id[id_arr[i]]->ptr || mapVoice_id[id_arr[i]]->GetVoiceData(sendptr + idx_SendBuf + 1 + mapVoice_id[id_arr[i]]->LenId, SIZE_VOICE_BUFFER) == -1))
				continue;

			sendptr[idx_SendBuf] = mapVoice_id[id_arr[i]]->LenId;
			idx_SendBuf += 1;
			memcpy(sendptr + idx_SendBuf, mapVoice_id[id_arr[i]]->Id, mapVoice_id[id_arr[i]]->LenId);
			idx_SendBuf += mapVoice_id[id_arr[i]]->LenId;

			idx_SendBuf += SIZE_VOICE_BUFFER;
		}

		memcpy(sendptr + SIZE_HEADER_VOICE_CMD, &idx_SendBuf, SIZE_HEADER_VOICE_LENGTH);

		ptr->willSendbytes += idx_SendBuf;

		Req_WSASend(ptr, threadNum);
		ptr->UnLock();
	}
}

void StartVoice(SOCKETINFO *ptr, int threadNum)
{
	char id[17];
	int lenid = ptr->recvBuf[SIZE_HEADER_VOICE];
	memcpy(id, ptr->recvBuf + SIZE_HEADER_VOICE + 1, lenid);
	id[lenid] = 0;

	InsertVoiceUser(ptr, id, lenid);

	TYPE_HEADER_VOICE_CMD cmd = CMD_START_VOICE;
	TYPE_HEADER_VOICE_LENGTH len_packet = SIZE_HEADER_VOICE;

	ptr->Lock();
	memcpy(ptr->sendBuf + ptr->willSendbytes, &cmd, SIZE_HEADER_VOICE_CMD);
	ptr->willSendbytes += SIZE_HEADER_VOICE_CMD;
	memcpy(ptr->sendBuf + ptr->willSendbytes, &len_packet, SIZE_HEADER_VOICE_LENGTH);
	ptr->willSendbytes += SIZE_HEADER_VOICE_LENGTH;
	
	Req_WSASend(ptr, threadNum);
	ptr->UnLock();
}


#define CMD_VOICE_GIVE_TO_SAY		'S' << 8 | 'G'
#define CMD_VOICE_STOP_TO_SAY		'S' << 8 | 'S'
void Send_CMD_To_Other_Voice(char *threadBuf, HANDLE hcp, SOCKETINFO *Src, int threadNum, TYPE_HEADER_VOICE_CMD cmd)
{
	char id[16];
	char lenid = Src->recvBuf[SIZE_HEADER_VOICE];
	memcpy(id, Src->recvBuf + SIZE_HEADER_VOICE + 1, lenid);
	id[lenid] = 0;

	TYPE_HEADER_VOICE_LENGTH len = SIZE_HEADER_VOICE;
	memcpy(threadBuf, &cmd, SIZE_HEADER_VOICE_CMD);
	memcpy(threadBuf + SIZE_HEADER_VOICE_CMD, &len, SIZE_HEADER_VOICE_LENGTH);

	if (mapVoice_id.find(id) != mapVoice_id.end())
	{
		mapVoice_id[id]->ptr->Update_SendBuf_FromOthreThread(hcp, threadBuf, SIZE_HEADER_VOICE);
	}
}
void Main_VoiceServerLogic(char *threadBuf, HANDLE hcp, unsigned int cbTransferred, SOCKETINFO *ptr, int threadNum)
{
	ptr->recievedbytes += cbTransferred;

	TYPE_HEADER_VOICE_LENGTH length = *(TYPE_HEADER_VOICE_LENGTH *)&ptr->recvBuf[2];
	while (ptr->recievedbytes >= SIZE_HEADER_VOICE && length <= (TYPE_HEADER_VOICE_LENGTH)ptr->recievedbytes)
	{
		switch (*(TYPE_HEADER_VOICE_CMD *)&ptr->recvBuf)
		{
		case CMD_START_VOICE:
			StartVoice(ptr, threadNum);
			break;
		case CMD_SET_VOICE_PACKET:
			SetVoiceData(ptr, threadNum);
			break;
		case CMD_GET_VOICE_PACKET:
			Send_Voice_Data(ptr, threadNum, threadBuf);
			break;
		case CMD_VOICE_GIVE_TO_SAY:
			Send_CMD_To_Other_Voice(threadBuf, hcp, ptr, threadNum, CMD_VOICE_GIVE_TO_SAY);
			break;
		case CMD_VOICE_STOP_TO_SAY:
			Send_CMD_To_Other_Voice(threadBuf, hcp, ptr, threadNum, CMD_VOICE_STOP_TO_SAY);
			break;
		default:
			break;
		}
		ptr->recievedbytes = ptr->recievedbytes - (unsigned int)length;
		memcpy(ptr->recvBuf, ptr->recvBuf + length, ptr->recievedbytes);

		length = *(TYPE_HEADER_VOICE_LENGTH *)&ptr->recvBuf[2];
	}
}