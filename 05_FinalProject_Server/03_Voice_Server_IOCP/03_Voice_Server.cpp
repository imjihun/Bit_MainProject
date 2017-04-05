#include "03_Voice_Server.h"
#pragma region work source
//std::map<string, UserData*> mapUserInfo_InRoom_id;
//std::map<SOCKETINFO*, UserData*> mapUserInfo_InRoom_ptr;
//CRITICAL_SECTION g_CS;
//
//char DBIP[20] = "192.168.0.25";
//int DBPORT = 8000;
//SOCKET DBSocket[17];
//
//
//static int Room_Number = 1;
//int ParseRecvBuf_DBRecv(char *src, char **dst)
//{
//	int idx_buf = SIZE_HEADER;
//	TYPE_HEADER_VOICE_CMD lenth = *(TYPE_HEADER_VOICE_CMD *)(src + 2);
//	int len = 0;
//	int i = 0;
//	for (i = 0; idx_buf < lenth; idx_buf += len + 1)
//	{
//		len = (unsigned char)src[idx_buf];
//		src[idx_buf] = '\0';
//		dst[i++] = &src[idx_buf + 1];
//	}
//	src[idx_buf] = '\0';
//	return i;
//}
//void Get_mapUserId(SOCKETINFO *ptr, char *retId, int sizeret, unsigned char *retLenid)
//{
//	EnterCriticalSection(&g_CS);
//	if (mapUserInfo_InRoom_ptr.find(ptr) != mapUserInfo_InRoom_ptr.end())
//	{
//		*retLenid = mapUserInfo_InRoom_ptr[ptr]->LenId;
//		memcpy_s(retId, sizeret, mapUserInfo_InRoom_ptr[ptr]->Id, *retLenid);
//		retId[*retLenid] = 0;
//	}
//	LeaveCriticalSection(&g_CS);
//}
//void Remove_mapUserInfo(SOCKETINFO *ptr)
//{
//	EnterCriticalSection(&g_CS);
//	if (mapUserInfo_InRoom_ptr.find(ptr) != mapUserInfo_InRoom_ptr.end())
//	{
//		char id[16];
//		int lenid = mapUserInfo_InRoom_ptr[ptr]->LenId;
//		memcpy_s(id, 16, mapUserInfo_InRoom_ptr[ptr]->Id, lenid);
//		id[lenid] = 0;
//
//		//delete(mapUserInfo_InRoom_ptr[ptr]);
//		mapUserInfo_InRoom_ptr.erase(ptr);
//		mapUserInfo_InRoom_id.erase(id);
//	}
//	LeaveCriticalSection(&g_CS);
//}
//void Insert_mapUserInfo(SOCKETINFO *ptr, char *id, int lenid)
//{
//	EnterCriticalSection(&g_CS);
//	id[lenid] = 0;
//	mapUserInfo_InRoom_id[id] = new UserData(ptr, id, lenid);
//	mapUserInfo_InRoom_ptr[ptr] = mapUserInfo_InRoom_id[id];
//	LeaveCriticalSection(&g_CS);
//}
//void Update_mapUserInfo(SOCKETINFO *ptr, char key_packet)
//{
//	EnterCriticalSection(&g_CS);
//	if (mapUserInfo_InRoom_ptr.find(ptr) != mapUserInfo_InRoom_ptr.end())
//	{
//		mapUserInfo_InRoom_ptr[ptr]->Curkey = key_packet;
//	}
//	LeaveCriticalSection(&g_CS);
//}
//void Update_mapUserInfo(SOCKETINFO *ptr, Vector3 pos, Vector3 rot)
//{
//	EnterCriticalSection(&g_CS);
//	if (mapUserInfo_InRoom_ptr.find(ptr) != mapUserInfo_InRoom_ptr.end())
//	{
//		mapUserInfo_InRoom_ptr[ptr]->Position = pos;
//		mapUserInfo_InRoom_ptr[ptr]->Rotation = rot;
//	}
//	LeaveCriticalSection(&g_CS);
//}
//void Req_AllUserKey_InRoom(HANDLE hcp, SOCKETINFO *dst, char **id, int cntId)
//{
//	char buf[1024];
//	int lenbuf = 1024;
//	TYPE_HEADER_VOICE_CMD cmd = CMD_KEY_PACKET;
//	memcpy_s(buf, lenbuf, &cmd, 2);
//	TYPE_HEADER_VOICE_CMD idx = SIZE_HEADER;
//
//	EnterCriticalSection(&g_CS);
//	for (int i = 0; i < cntId; i++)
//	{
//		memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->LenId, 1);
//		idx++;
//		memcpy_s(buf + idx, lenbuf - idx, id[i], mapUserInfo_InRoom_id[id[i]]->LenId);
//		idx += mapUserInfo_InRoom_id[id[i]]->LenId;
//		memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->Curkey, 1);
//		idx++;
//	}
//	memcpy_s(buf + 2, lenbuf - 2, &idx, 2);
//	LeaveCriticalSection(&g_CS);
//
//	dst->Update_SendBuf(hcp, buf, idx);
//}
//void Req_AllUserPos_InRoom(HANDLE hcp, SOCKETINFO *dst, char **id, int cntId)
//{
//	char buf[1024];
//	int lenbuf = 1024;
//	TYPE_HEADER_VOICE_CMD cmd = CMD_GET_POSITION_PACKET;
//	memcpy_s(buf, lenbuf, &cmd, 2);
//	TYPE_HEADER_VOICE_CMD idx = SIZE_HEADER;
//
//	EnterCriticalSection(&g_CS);
//	for (int i = 0; i < cntId; i++)
//	{
//		memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->LenId, 1);
//		idx++;
//		memcpy_s(buf + idx, lenbuf - idx, id[i], mapUserInfo_InRoom_id[id[i]]->LenId);
//		idx += mapUserInfo_InRoom_id[id[i]]->LenId;
//
//		memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->Position.X, 4);
//		idx += 4;
//		memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->Position.Y, 4);
//		idx += 4;
//		memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->Position.Z, 4);
//		idx += 4;
//
//		memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->Rotation.X, 4);
//		idx += 4;
//		memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->Rotation.Y, 4);
//		idx += 4;
//		memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->Rotation.Z, 4);
//		idx += 4;
//	}
//	memcpy_s(buf + 2, lenbuf - 2, &idx, 2);
//	LeaveCriticalSection(&g_CS);
//
//	dst->Update_SendBuf(hcp, buf, idx);
//}
//void Scatter_Buf_InRoom(HANDLE hcp, char **id, int cntId, char *SendBuf, int lenSendBuf)
//{
//	EnterCriticalSection(&g_CS);
//	for (int i = 0; i < cntId; i++)
//	{
//		mapUserInfo_InRoom_id[id[i]]->Ptr->Update_SendBuf_FromOthreThread(hcp, SendBuf, lenSendBuf);
//	}
//	LeaveCriticalSection(&g_CS);
//}
//
//void Init_WorkServer()
//{
//	SOCKADDR_IN DBAddr;
//	BOOL optVal = TRUE;
//	for (int i = 0; i < 17; i++)
//	{
//		if ((DBSocket[i] = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
//			ViewError("DBsocket() error");
//
//		if (setsockopt(DBSocket[i], SOL_SOCKET, SO_REUSEADDR, (char *)&optVal, sizeof(optVal)) == SOCKET_ERROR)
//			ViewError("DBsetsockopt() error");
//
//		memset(&DBAddr, 0, sizeof(DBAddr));
//		DBAddr.sin_family = AF_INET;
//		DBAddr.sin_addr.s_addr = inet_addr(DBIP);
//		DBAddr.sin_port = htons(DBPORT);
//		if (connect(DBSocket[i], (SOCKADDR *)&DBAddr, sizeof(DBAddr)) != 0)
//			ViewError("DBconnect() error");
//	}
//
//	InitializeCriticalSection(&g_CS);
//}
//
//void Req_DB(char *src, int len, char *dst, int dstSize, int threadNum)
//{
//	send(DBSocket[threadNum], src, len, 0);
//	recv(DBSocket[threadNum], dst, dstSize, 0);
//}
//void ConnectClient(SOCKETINFO *ptr, char *recv)
//{
//	char id[16];
//	int len_id = recv[SIZE_HEADER];
//	memcpy_s(id, 16, recv + SIZE_HEADER + 1, len_id);
//	id[len_id] = 0;
//	Insert_mapUserInfo(ptr, id, len_id);
//
//	//memcpy_s(retBuf, SIZE_THREAD_BUF, ptr->recvBuf, 2);
//	//TYPE_HEADER_VOICE_CMD len = 4;
//	//memcpy_s(retBuf + 2, SIZE_THREAD_BUF - 2, &len, 2);
//}
//void DisConnectClient(HANDLE hcp, char *threadBuf, SOCKETINFO *ptr, int threadNum)
//{
//	unsigned char lenid = 0;
//	char id[16];
//	Get_mapUserId(ptr, id, 16, &lenid);
//
//	TYPE_HEADER_VOICE_CMD cmd = CMD_LEAVE_ROOM;
//	memcpy_s(threadBuf, 1024, &cmd, 2);
//	TYPE_HEADER_VOICE_CMD idx = SIZE_HEADER;
//	threadBuf[idx++] = lenid;
//	memcpy_s(threadBuf + idx, 1024 - idx, id, lenid);
//	idx += lenid;
//	memcpy_s(threadBuf + 2, 1024 - 2, &idx, 2);
//	Req_DB(threadBuf, idx, threadBuf, SIZE_THREAD_BUF, threadNum);
//
//
//	char buf[1024];
//	cmd = CMD_LEAVE_ROOM;
//	memcpy_s(buf, 1024, &cmd, 2);
//	idx = SIZE_HEADER;
//	memcpy_s(buf + idx, 1024 - idx, ptr->recvBuf + SIZE_HEADER, 1 + ptr->recvBuf[SIZE_HEADER]);
//	idx += 1 + ptr->recvBuf[SIZE_HEADER];
//	memcpy_s(buf + 2, 1024 - 2, &idx, 2);
//	char *split_buf[MAX_MEMBERCNT_INROOM];
//	int cnt = ParseRecvBuf_DBRecv(threadBuf, split_buf);
//	Scatter_Buf_InRoom(hcp, split_buf, cnt, buf, idx);
//
//	Remove_mapUserInfo(ptr);
//}
//void InsertPacket_ForCreateRoom(char *src, char *dst, int SizeDst)
//{
//	char tmp[16];
//	unsigned char len;
//	static int Room_CurMemberCount = 1;
//	static int Room_State = 0;
//	// Recv
//	// User_Id, RoomMode_Id, Room_Subject, Room_Title, Room_MaxMemberCount
//	// Send
//	// User_Id, Room_Number, RoomMode_Id, Room_Subject, Room_Title, Room_MaxMemberCount, Room_CurMemberCount, Room_State
//	memcpy_s(dst, SizeDst, src, 2);
//
//	TYPE_HEADER_VOICE_CMD idxDst = SIZE_HEADER;
//	TYPE_HEADER_VOICE_CMD idxSrc = SIZE_HEADER;
//	memcpy_s(dst + idxDst, SizeDst - idxDst, src + idxSrc, src[idxSrc] + 1);
//	idxDst += src[idxSrc] + 1;
//	idxSrc += src[idxSrc] + 1;
//
//	itoa(Room_Number++, tmp, 10);
//	len = strlen(tmp);
//	memcpy_s(dst + idxDst, SizeDst - idxDst, &len, 1);
//	idxDst += 1;
//	memcpy_s(dst + idxDst, SizeDst - idxDst, tmp, len);
//	idxDst += len;
//
//	memcpy_s(dst + idxDst, SizeDst - idxDst, src + idxSrc, src[idxSrc] + 1);
//	idxDst += src[idxSrc] + 1;
//	idxSrc += src[idxSrc] + 1;
//
//
//	memcpy_s(dst + idxDst, SizeDst - idxDst, src + idxSrc, src[idxSrc] + 1);
//	idxDst += src[idxSrc] + 1;
//	idxSrc += src[idxSrc] + 1;
//
//
//	memcpy_s(dst + idxDst, SizeDst - idxDst, src + idxSrc, src[idxSrc] + 1);
//	idxDst += src[idxSrc] + 1;
//	idxSrc += src[idxSrc] + 1;
//
//
//	memcpy_s(dst + idxDst, SizeDst - idxDst, src + idxSrc, src[idxSrc] + 1);
//	idxDst += src[idxSrc] + 1;
//	idxSrc += src[idxSrc] + 1;
//
//
//	itoa(Room_CurMemberCount, tmp, 10);
//	len = strlen(tmp);
//	memcpy_s(dst + idxDst, SizeDst - idxDst, &len, 1);
//	idxDst += 1;
//	memcpy_s(dst + idxDst, SizeDst - idxDst, tmp, len);
//	idxDst += len;
//
//
//	itoa(Room_State, tmp, 10);
//	len = strlen(tmp);
//	memcpy_s(dst + idxDst, SizeDst - idxDst, &len, 1);
//	idxDst += 1;
//	memcpy_s(dst + idxDst, SizeDst - idxDst, tmp, len);
//	idxDst += len;
//
//	memcpy_s(dst + 2, SizeDst - 2, &idxDst, 2);
//}
#pragma endregion

#define SIZE_HEADER_VOICE_CMD		2
#define TYPE_HEADER_VOICE_CMD		unsigned short
#define SIZE_HEADER_VOICE_LENGTH	4	
#define TYPE_HEADER_VOICE_LENGTH	int
#define SIZE_HEADER_VOICE			SIZE_HEADER_VOICE_CMD + SIZE_HEADER_VOICE_LENGTH
#define SIZE_VOICE_BUFFER_FLOAT		4096
#define SIZE_VOICE_BUFFER			SIZE_VOICE_BUFFER_FLOAT * 4

#define VOICE_FREQUENCY				44100
#define TIME_VOICE_BUFFER_MS		(SIZE_VOICE_BUFFER_FLOAT * 1000) / VOICE_FREQUENCY

#define CNT_THREAD					17
#pragma comment(lib, "Winmm.lib")
class VoiceData
{
private:
	char Voice_Buffer[SIZE_VOICE_BUFFER];

public:
	char Id[17];
	int LenId;
	DWORD curr_time;

	VoiceData(char *id, int lenid)
	{
		memcpy_s(Id, 16, id, lenid);
		Id[lenid] = 0;
		LenId = lenid;
		curr_time = timeGetTime();
	}
	~VoiceData()
	{
	}
	void SetVoiceData(char *src, int len_src)
	{
		memcpy_s(Voice_Buffer, SIZE_VOICE_BUFFER, src, len_src);
		curr_time = timeGetTime();
	}
	int GetVoiceData(char *dst, int len_dst)
	{
		if (timeGetTime() - curr_time > TIME_VOICE_BUFFER_MS)
		{
			return -1;
		}

		memcpy_s(dst, len_dst, Voice_Buffer, SIZE_VOICE_BUFFER);
		return 0;
	}
};

std::map<SOCKETINFO*, VoiceData*> mapVoice_ptr;
int Cnt_Voice = 0;
CRITICAL_SECTION voice_CS[CNT_THREAD];
void init_voice_CS()
{
	for (int i = 0; i < CNT_THREAD; i++)
	{
		InitializeCriticalSection(&voice_CS[i]);
	}
}
void del_voice_CS()
{
	for (int i = 0; i < CNT_THREAD; i++)
	{
		DeleteCriticalSection(&voice_CS[i]);
	}
}
void Enter_voice_CS(int num)
{
	EnterCriticalSection(&voice_CS[num]);
}
void Leave_voice_CS(int num)
{
	LeaveCriticalSection(&voice_CS[num]);
}
void Enter_voice_CS()
{
	for (int i = 0; i < CNT_THREAD; i++)
	{
		EnterCriticalSection(&voice_CS[i]);
	}
}
void Leave_voice_CS()
{
	for (int i = 0; i < CNT_THREAD; i++)
	{
		LeaveCriticalSection(&voice_CS[i]);
	}
}

void InitVoiceServer()
{
	init_voice_CS();
	Cnt_Voice = 0;
}
void InsertVoiceUser(SOCKETINFO *ptr, char *id, int lenid)
{
	Enter_voice_CS();
	Cnt_Voice++;
	mapVoice_ptr[ptr] = new VoiceData(id, lenid);
	Leave_voice_CS();
}
void DeleteVoiceUser(SOCKETINFO *ptr)
{
	Enter_voice_CS();
	mapVoice_ptr.erase(ptr);
	Cnt_Voice--;
	Leave_voice_CS();
}
void SetVoiceData(SOCKETINFO *ptr, int threadNum)
{
	Enter_voice_CS(threadNum);
	int idx_start = SIZE_HEADER_VOICE + mapVoice_ptr[ptr]->LenId + 1;
	mapVoice_ptr[ptr]->SetVoiceData(ptr->recvBuf + idx_start, SIZE_VOICE_BUFFER);
	Leave_voice_CS(threadNum);
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

	ViewMessage("IOCP[%d] : [TCP/%s:%d] [WillSend : %c%c] [len = %u]", threadNum,
		inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port), ptr->wsaSendBuf.buf[0], ptr->wsaSendBuf.buf[1], *(TYPE_HEADER_VOICE_LENGTH*)(ptr->wsaSendBuf.buf + 2));

}
void Send_Voice_Data(SOCKETINFO *ptr, int threadNum)
{
	TYPE_HEADER_VOICE_CMD cmd = CMD_GET_VOICE_PACKET;
	memcpy(ptr->sendBuf, &cmd, SIZE_HEADER_VOICE_CMD);

	TYPE_HEADER_VOICE_LENGTH idx_SendBuf = SIZE_HEADER_VOICE;
	Enter_voice_CS(threadNum);
	for (map<SOCKETINFO*, VoiceData*>::iterator it = mapVoice_ptr.begin(); it != mapVoice_ptr.end(); it++)
	{
		if (ptr == it->first || it->second->GetVoiceData(ptr->sendBuf + idx_SendBuf + 1 + it->second->LenId, SIZE_VOICE_BUFFER) == -1)
			continue;

		ptr->sendBuf[idx_SendBuf] = it->second->LenId;
		idx_SendBuf += 1;
		memcpy(ptr->sendBuf + idx_SendBuf, it->second->Id, it->second->LenId);
		idx_SendBuf += it->second->LenId;

		idx_SendBuf += SIZE_VOICE_BUFFER;
	}
	Leave_voice_CS(threadNum);
	memcpy(ptr->sendBuf + SIZE_HEADER_VOICE_CMD, &idx_SendBuf, SIZE_HEADER_VOICE_LENGTH);

	ptr->willSendbytes += idx_SendBuf;

	Req_WSASend(ptr, threadNum);
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

	memcpy(ptr->sendBuf + ptr->willSendbytes, &cmd, SIZE_HEADER_VOICE_CMD);
	ptr->willSendbytes += SIZE_HEADER_VOICE_CMD;
	memcpy(ptr->sendBuf + ptr->willSendbytes, &len_packet, SIZE_HEADER_VOICE_LENGTH);
	ptr->willSendbytes += SIZE_HEADER_VOICE_LENGTH;
	
	Req_WSASend(ptr, threadNum);
}

void Main_VoiceServerLogic(char *threadBuf, HANDLE hcp, unsigned int cbTransferred, SOCKETINFO *ptr, int threadNum)
{
	ptr->recievedbytes += cbTransferred;

	if (ptr->recievedbytes >= SIZE_HEADER_VOICE)
	{
		TYPE_HEADER_VOICE_LENGTH length = *(TYPE_HEADER_VOICE_LENGTH *)&ptr->recvBuf[2];
		if (length <= (TYPE_HEADER_VOICE_LENGTH)ptr->recievedbytes)
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
				Send_Voice_Data(ptr, threadNum);
				break;
			default:
				break;
			}
			ptr->recievedbytes = ptr->recievedbytes - (unsigned int)length;
			memcpy(ptr->recvBuf, ptr->recvBuf + length, ptr->recievedbytes);
		}
	}
}