#include "03_Logic_Work_Server.h"


std::map<string, UserData*> mapUserInfo_InRoom_id;
std::map<SOCKETINFO*, UserData*> mapUserInfo_InRoom_ptr;
CRITICAL_SECTION UserInfo_CS;

static int newRoomNum = 0;
static CRITICAL_SECTION newRoomNum_CS;

char DBIP[20] = "127.0.0.1";
//char DBIP[20] = "14.32.7.30";
int DBPORT = 8000;
SOCKET DBSocket[CNT_THREAD];


void PrintPacket(SOCKETINFO *ptr, char *packet, int len, char *str)
{
	printf("___[%s]___ %x[%u] : ", str, ptr, len);
	for (int i = 0; i < len; i++)
		printf("%x ", packet[i]);
	printf("\n");
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

	PrintPacket(ptr, ptr->sendBuf, *(TYPE_HEADER_LENGTH *)(ptr->sendBuf + SIZE_HEADER_LENGTH), "Send");
/*
	ViewMessage("IOCP[%d] : [TCP/%s:%d] [WillSend : %c%c] [len = %u]", threadNum,
		inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port), ptr->wsaSendBuf.buf[0], ptr->wsaSendBuf.buf[1], *(TYPE_HEADER_LENGTH*)(ptr->wsaSendBuf.buf + SIZE_HEADER_CMD));
*/
}
void SendBuf_ptr(SOCKETINFO *ptr, HANDLE hcp, char *buf, TYPE_HEADER_LENGTH idx, int threadNum)
{
	//ptr->Update_SendBuf(hcp, buf, idx);
	memcpy(ptr->sendBuf + ptr->willSendbytes, buf, idx);
	ptr->willSendbytes += idx;
	Req_WSASend(ptr, threadNum);
}
void SendBuf_ptr_img(SOCKETINFO *ptr, HANDLE hcp, char *buf, int idx, int threadNum)
{
	//ptr->Update_SendBuf(hcp, buf, idx);
	memcpy(ptr->sendBuf + ptr->willSendbytes, buf, idx);
	ptr->willSendbytes += idx;
	Req_WSASend(ptr, threadNum);
}

int ParseRecvBuf_DBRecv(char *src, char **dst)
{
	int idx_buf = SIZE_HEADER;
	unsigned short lenth = *(unsigned short *)(src + 2);
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
void Get_mapUserId(SOCKETINFO *ptr, char *retId, int sizeret, unsigned char *retLenid)
{
	EnterCriticalSection(&UserInfo_CS);
	if (mapUserInfo_InRoom_ptr.find(ptr) != mapUserInfo_InRoom_ptr.end())
	{
		*retLenid = mapUserInfo_InRoom_ptr[ptr]->LenId;
		memcpy_s(retId, sizeret, mapUserInfo_InRoom_ptr[ptr]->Id, *retLenid);
		retId[*retLenid] = 0;
	}
	LeaveCriticalSection(&UserInfo_CS);
}
void Remove_mapUserInfo(SOCKETINFO *ptr)
{
	EnterCriticalSection(&UserInfo_CS);
	if (mapUserInfo_InRoom_ptr.find(ptr) != mapUserInfo_InRoom_ptr.end())
	{
		char id[16];
		int lenid = mapUserInfo_InRoom_ptr[ptr]->LenId;
		memcpy_s(id, 16, mapUserInfo_InRoom_ptr[ptr]->Id, lenid);
		id[lenid] = 0;

		delete(mapUserInfo_InRoom_ptr[ptr]);
		mapUserInfo_InRoom_ptr.erase(ptr);
		mapUserInfo_InRoom_id.erase(id);

		printf("__________[Debug : Removed] Connected User Cnt = %d:%d //[id=%s, ptr=%x]\n", mapUserInfo_InRoom_ptr.size(), mapUserInfo_InRoom_id.size(), id, ptr);
	}
	LeaveCriticalSection(&UserInfo_CS);
}
void Insert_mapUserInfo(SOCKETINFO *ptr, char *_id, int lenid)
{
	EnterCriticalSection(&UserInfo_CS);
	_id[lenid] = 0;

	if (mapUserInfo_InRoom_id.find(_id) != mapUserInfo_InRoom_id.end())
	{
		SOCKETINFO *p = mapUserInfo_InRoom_id[_id]->Ptr;
		delete(mapUserInfo_InRoom_ptr[p]);
		mapUserInfo_InRoom_ptr.erase(p);
		mapUserInfo_InRoom_id.erase(_id);

		printf("__________[Debug : Removed] Connected User Cnt = %d:%d //[id=%s, ptr=%x]\n", mapUserInfo_InRoom_ptr.size(), mapUserInfo_InRoom_id.size(), _id, ptr);
	}

	mapUserInfo_InRoom_id[_id] = new UserData(ptr, _id, lenid);
	mapUserInfo_InRoom_ptr[ptr] = mapUserInfo_InRoom_id[_id];
	printf("__________[Debug : Inserted] Connected User Cnt = %d:%d //[id=%s, ptr=%x]\n", mapUserInfo_InRoom_ptr.size(), mapUserInfo_InRoom_id.size(), _id, ptr);
	LeaveCriticalSection(&UserInfo_CS);
}
void Update_mapUserInfo(SOCKETINFO *ptr, char key_packet)
{
	EnterCriticalSection(&UserInfo_CS);
	if (mapUserInfo_InRoom_ptr.find(ptr) != mapUserInfo_InRoom_ptr.end())
	{
		mapUserInfo_InRoom_ptr[ptr]->Curkey = key_packet;
	}
	LeaveCriticalSection(&UserInfo_CS);
}
void Update_mapUserInfo(SOCKETINFO *ptr, Vector3 pos, Vector3 rot, AnimStatus status)
{
	EnterCriticalSection(&UserInfo_CS);
	if (mapUserInfo_InRoom_ptr.find(ptr) != mapUserInfo_InRoom_ptr.end())
	{
		mapUserInfo_InRoom_ptr[ptr]->Position = pos;
		mapUserInfo_InRoom_ptr[ptr]->Rotation = rot;
		mapUserInfo_InRoom_ptr[ptr]->Status = status;
	}
	LeaveCriticalSection(&UserInfo_CS);
}
void Update_mapUserRoom(SOCKETINFO *ptr, int roomNum)
{
	EnterCriticalSection(&UserInfo_CS);
	if (mapUserInfo_InRoom_ptr.find(ptr) != mapUserInfo_InRoom_ptr.end())
	{
		mapUserInfo_InRoom_ptr[ptr]->roomNum = roomNum;
	}
	LeaveCriticalSection(&UserInfo_CS);
}
void Req_AllUserKey_InRoom(HANDLE hcp, SOCKETINFO *dst, char **id, int cntId, int threadNum)
{
	char buf[1024];
	int lenbuf = 1024;
	unsigned short cmd = CMD_KEY_PACKET;
	memcpy_s(buf, lenbuf, &cmd, 2);
	unsigned short idx = SIZE_HEADER;

	EnterCriticalSection(&UserInfo_CS);
	for (int i = 0; i < cntId; i++)
	{
		if (mapUserInfo_InRoom_id.find(id[i]) != mapUserInfo_InRoom_id.end())
		{
			memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->LenId, 1);
			idx++;
			memcpy_s(buf + idx, lenbuf - idx, id[i], mapUserInfo_InRoom_id[id[i]]->LenId);
			idx += mapUserInfo_InRoom_id[id[i]]->LenId;
			memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->Curkey, 1);
			idx++;
		}
	}
	memcpy_s(buf + 2, lenbuf-2, &idx, 2);
	LeaveCriticalSection(&UserInfo_CS);

	SendBuf_ptr(dst, hcp, buf, idx, threadNum);
}
void Req_AllUserPos_InRoom(HANDLE hcp, SOCKETINFO *dst, char **id, int cntId, int threadNum)
{
	char buf[1024];
	int lenbuf = 1024;
	unsigned short cmd = CMD_GET_POS_STATUS_PACKET;
	memcpy_s(buf, lenbuf, &cmd, 2);
	unsigned short idx = SIZE_HEADER;

	EnterCriticalSection(&UserInfo_CS);
	for (int i = 0; i < cntId; i++)
	{
		if (mapUserInfo_InRoom_id.find(id[i]) != mapUserInfo_InRoom_id.end())
		{
			memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->LenId, 1);
			idx++;
			memcpy_s(buf + idx, lenbuf - idx, id[i], mapUserInfo_InRoom_id[id[i]]->LenId);
			idx += mapUserInfo_InRoom_id[id[i]]->LenId;

			memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->Position.X, 4);
			idx += 4;
			memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->Position.Y, 4);
			idx += 4;
			memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->Position.Z, 4);
			idx += 4;

			memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->Rotation.X, 4);
			idx += 4;
			memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->Rotation.Y, 4);
			idx += 4;
			memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->Rotation.Z, 4);
			idx += 4;


			memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->Status.bSitting, 1);
			idx += 1;
			memcpy_s(buf + idx, lenbuf - idx, &mapUserInfo_InRoom_id[id[i]]->Status.bQuestioning, 1);
			idx += 1;
		}
	}
	memcpy_s(buf + 2, lenbuf - 2, &idx, 2);
	LeaveCriticalSection(&UserInfo_CS);

	SendBuf_ptr(dst, hcp, buf, idx, threadNum);
}
void Scatter_Buf_InRoom(HANDLE hcp, SOCKETINFO *src, char **id, int cntId, char *SendBuf, int lenSendBuf, int threadNum)
{
	EnterCriticalSection(&UserInfo_CS);
	for (int i = 0; i < cntId; i++)
	{
		if (mapUserInfo_InRoom_id.find(id[i]) != mapUserInfo_InRoom_id.end())
		{
			if (src == mapUserInfo_InRoom_id[id[i]]->Ptr)
				SendBuf_ptr(src, hcp, SendBuf, lenSendBuf, threadNum);
			else
				mapUserInfo_InRoom_id[id[i]]->Ptr->Update_SendBuf_FromOthreThread(hcp, SendBuf, lenSendBuf);
		}
	}
	LeaveCriticalSection(&UserInfo_CS);
}

void Init_WorkServer()
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

	InitializeCriticalSection(&UserInfo_CS);
	InitializeCriticalSection(&newRoomNum_CS);
}

void Req_DB(char *src, int len, char *dst, int dstSize, int threadNum)
{
	//printf("\t[t:%d]DB =", threadNum);
	//for (int i = 0; i < len; i++)
	//	printf(" %x", src[i]);
	//printf("\n");
	send(DBSocket[threadNum], src, len, 0);
	recv(DBSocket[threadNum], dst, dstSize, 0);
}
//
//void ConnectClient(SOCKETINFO *ptr, char *recv)
//{
//	char id[16];
//	int len_id = recv[SIZE_HEADER];
//	memcpy_s(id, 16, recv + SIZE_HEADER + 1, len_id);
//	id[len_id] = 0;
//	Insert_mapUserInfo(ptr, id, len_id);
//
//	//memcpy_s(retBuf, SIZE_THREAD_BUF, ptr->recvBuf, 2);
//	//unsigned short len = 4;
//	//memcpy_s(retBuf + 2, SIZE_THREAD_BUF - 2, &len, 2);
//}
void LogedIn(char *threadBuf, HANDLE hcp, SOCKETINFO *ptr, int threadNum)
{
	char id[16];
	int len_id = ptr->recvBuf[SIZE_HEADER];
	memcpy_s(id, 16, ptr->recvBuf + SIZE_HEADER + 1, len_id);
	id[len_id] = 0;
	Insert_mapUserInfo(ptr, id, len_id);

	Req_DB(ptr->recvBuf, *(TYPE_HEADER_LENGTH *)(ptr->recvBuf + SIZE_HEADER_CMD), threadBuf, SIZE_THREAD_BUF, threadNum);

	SendBuf_ptr(ptr, hcp, ptr->recvBuf, *(TYPE_HEADER_LENGTH *)(ptr->recvBuf + SIZE_HEADER_CMD), threadNum);
}
void _Make_CharPacket(char *dst, TYPE_HEADER_LENGTH *idx_dst, char *src, int len_copy)
{
	dst[(*idx_dst)++] = len_copy;
	memcpy(dst + (*idx_dst), src, len_copy);
	(*idx_dst) += len_copy;
}
void InsertPacket_ForCreateRoom(char *src, char *dst, int SizeDst, int roomNum)
{
	char tmp[16];
	unsigned char len;
	static int Room_CurMemberCount = 0;
	static int Room_State = 0;
	// Recv
	// User_Id, RoomMode_Id, Room_Subject, Room_Title, Room_MaxMemberCount
	// Send
	// User_Id, Room_Number, RoomMode_Id, Room_Subject, Room_Title, Room_MaxMemberCount, Room_CurMemberCount, Room_State
	memcpy_s(dst, SizeDst, src, SIZE_HEADER_CMD);

	TYPE_HEADER_LENGTH idxDst = SIZE_HEADER;
	TYPE_HEADER_LENGTH idxSrc = SIZE_HEADER;

	//User_Id
	_Make_CharPacket(dst, &idxDst, src + idxSrc + 1, src[idxSrc]);
	idxSrc += src[idxSrc] + 1;

	//Room_Number
	itoa(roomNum, tmp, 10);
	len = (unsigned char)strlen(tmp);
	_Make_CharPacket(dst, &idxDst, tmp, len);

	//RoomMode_Id
	_Make_CharPacket(dst, &idxDst, src + idxSrc + 1, src[idxSrc]);
	idxSrc += src[idxSrc] + 1;

	//Room_Subject
	_Make_CharPacket(dst, &idxDst, src + idxSrc + 1, src[idxSrc]);
	idxSrc += src[idxSrc] + 1;

	//Room_Title
	_Make_CharPacket(dst, &idxDst, src + idxSrc + 1, src[idxSrc]);
	idxSrc += src[idxSrc] + 1;

	//Room_MaxMemberCount
	_Make_CharPacket(dst, &idxDst, src + idxSrc + 1, src[idxSrc]);
	idxSrc += src[idxSrc] + 1;

	//Room_CurMemberCount
	itoa(Room_CurMemberCount, tmp, 10);
	len = (unsigned char)strlen(tmp);
	_Make_CharPacket(dst, &idxDst, tmp, len);

	//Room_State
	itoa(Room_State, tmp, 10);
	len = (unsigned char)strlen(tmp);
	_Make_CharPacket(dst, &idxDst, tmp, len);

	memcpy_s(dst + SIZE_HEADER_CMD, SizeDst - SIZE_HEADER_CMD, &idxDst, SIZE_HEADER_LENGTH);
}
void CreateRoom(char *threadBuf, HANDLE hcp, SOCKETINFO *ptr, int threadNum, int Len_Pack)
{
	EnterCriticalSection(&newRoomNum_CS);
	{
		// Recv
		// User_Id, RoomMode_Id, Room_Subject, Room_Title, Room_MaxMemberCount
		// Send
		// User_Id, Room_Number, RoomMode_Id, Room_Subject, Room_Title, Room_MaxMemberCount, Room_CurMemberCount, Room_State
		TYPE_HEADER_CMD cmd = CMD_DB_FAIL;
		memcpy(threadBuf, &cmd, SIZE_HEADER_CMD);

		cmd = CMD_CREATE_ROOM;
		while (cmd != *(TYPE_HEADER_CMD*)threadBuf)
		{
			newRoomNum++;
			Update_mapUserRoom(ptr, newRoomNum);

			InsertPacket_ForCreateRoom(ptr->recvBuf, threadBuf, SIZE_THREAD_BUF, newRoomNum);
			Req_DB(threadBuf, *(TYPE_HEADER_LENGTH*)(threadBuf + SIZE_HEADER_CMD), threadBuf, SIZE_THREAD_BUF, threadNum);
		}

		TYPE_HEADER_LENGTH len = SIZE_HEADER + sizeof(newRoomNum);
		memcpy(threadBuf, &cmd, SIZE_HEADER_CMD);
		memcpy(threadBuf + SIZE_HEADER_CMD, &len, SIZE_HEADER_LENGTH);
		memcpy(threadBuf + SIZE_HEADER, &newRoomNum, sizeof(newRoomNum));
		SendBuf_ptr(ptr, hcp, threadBuf, *(TYPE_HEADER_LENGTH *)(threadBuf + 2), threadNum);
	}
	LeaveCriticalSection(&newRoomNum_CS);
}
void EnterRoom(char *threadBuf, HANDLE hcp, SOCKETINFO *ptr, int threadNum, int Len_Pack)
{
	{
		char roomNum[16];
		int len_roomNum = ptr->recvBuf[SIZE_HEADER + ptr->recvBuf[SIZE_HEADER] + 1];
		memcpy(roomNum, ptr->recvBuf + SIZE_HEADER + ptr->recvBuf[SIZE_HEADER] + 2, len_roomNum);
		roomNum[len_roomNum] = 0;
		Update_mapUserRoom(ptr, atoi(roomNum));

		Req_DB(ptr->recvBuf, Len_Pack, threadBuf, SIZE_THREAD_BUF, threadNum);

		TYPE_HEADER_CMD cmd = CMD_DB_FAIL;
		if (*(TYPE_HEADER_CMD*)threadBuf == cmd)
		{
			SendBuf_ptr(ptr, hcp, threadBuf, *(TYPE_HEADER_LENGTH *)(threadBuf + SIZE_HEADER_CMD), threadNum);
		}
		else
		{
			char *id_arr[MAX_MEMBERCNT_INROOM];
			int cntId = ParseRecvBuf_DBRecv(threadBuf, id_arr);

			char buf[1024];
			cmd = CMD_ENTER_ROOM;
			memcpy_s(buf, 1024, &cmd, 2);
			TYPE_HEADER_LENGTH idx = SIZE_HEADER;
			memcpy_s(buf + idx, 1024 - idx, ptr->recvBuf + SIZE_HEADER, 1 + ptr->recvBuf[SIZE_HEADER]);
			idx += 1 + ptr->recvBuf[SIZE_HEADER];
			memcpy_s(buf + 2, 1024 - 2, &idx, 2);
			Scatter_Buf_InRoom(hcp, ptr, id_arr, cntId, buf, idx, threadNum);
		}
	}
}
void LeaveRoom(char *threadBuf, HANDLE hcp,  SOCKETINFO *ptr, int threadNum)
{
	unsigned char lenid = 0;
	char id[16];
	Get_mapUserId(ptr, id, 16, &lenid);

	unsigned short cmd = CMD_LEAVE_ROOM;
	memcpy_s(threadBuf, 1024, &cmd, 2);
	unsigned short idx = SIZE_HEADER;
	threadBuf[idx++] = lenid;
	memcpy_s(threadBuf + idx, 1024 - idx, id, lenid);
	idx += lenid;
	memcpy_s(threadBuf + 2, 1024 - 2, &idx, 2);
	Req_DB(threadBuf, idx, threadBuf, SIZE_THREAD_BUF, threadNum);


	char buf[1024];
	cmd = CMD_LEAVE_ROOM;
	memcpy_s(buf, 1024, &cmd, 2);
	idx = SIZE_HEADER;
	memcpy_s(buf + idx, 1024 - idx, ptr->recvBuf + SIZE_HEADER, 1 + ptr->recvBuf[SIZE_HEADER]);
	idx += 1 + ptr->recvBuf[SIZE_HEADER];
	memcpy_s(buf + 2, 1024 - 2, &idx, 2);
	char *split_buf[MAX_MEMBERCNT_INROOM];
	int cnt = ParseRecvBuf_DBRecv(threadBuf, split_buf);
	Scatter_Buf_InRoom(hcp, ptr, split_buf, cnt, buf, idx, threadNum);
}
void LogOut(char *threadBuf, SOCKETINFO *ptr, int threadNum)
{
	unsigned char lenid = 0;
	char id[16];
	Get_mapUserId(ptr, id, 16, &lenid);

	TYPE_HEADER_CMD cmd = CMD_LOGOUT;
	TYPE_HEADER_LENGTH len = SIZE_HEADER;
	memcpy(threadBuf, &cmd, SIZE_HEADER_CMD);

	threadBuf[len++] = lenid;
	memcpy(threadBuf + len, id, lenid);
	len += lenid;

	memcpy(threadBuf + SIZE_HEADER_CMD, &len, SIZE_HEADER_LENGTH);
	Req_DB(threadBuf, len, threadBuf, SIZE_THREAD_BUF, threadNum);
}
void DisConnectClient(HANDLE hcp, char *threadBuf, SOCKETINFO *ptr, int threadNum)
{
	LeaveRoom(threadBuf, hcp, ptr, threadNum);
	LogOut(threadBuf, ptr, threadNum);
	Remove_mapUserInfo(ptr);
}

void TradeKeyPacket(char *threadBuf, HANDLE hcp, SOCKETINFO *ptr, int threadNum, int Len_Pack)
{
	{
		// CMD, LEN
		// userid, key;
		Update_mapUserInfo(ptr, ptr->recvBuf[Len_Pack - 1]);
		unsigned short len = Len_Pack - 1;
		memcpy_s(ptr->recvBuf + 2, SIZE_BUF, &len, 2);

		Req_DB(ptr->recvBuf, len, threadBuf, SIZE_THREAD_BUF, threadNum);
		char *id[MAX_MEMBERCNT_INROOM];
		int cntId = ParseRecvBuf_DBRecv(threadBuf, id);
		Req_AllUserKey_InRoom(hcp, ptr, id, cntId, threadNum);
	}
}
void Set_Pos_Status_Packet(SOCKETINFO *ptr)
{
	{
		//CMD:LENGTH
		// :ID :POS :ROT
		unsigned short idx = SIZE_HEADER + 1 + ptr->recvBuf[SIZE_HEADER];
		Update_mapUserInfo(ptr, Vector3(ptr->recvBuf + idx), Vector3(ptr->recvBuf + idx + 12)
							, AnimStatus(ptr->recvBuf + idx + 24));

	}
}
void Get_Pos_Status_Packet(char *threadBuf, HANDLE hcp, SOCKETINFO *ptr, int threadNum)
{
	{
		char id[16];
		unsigned char lenid = ptr->recvBuf[SIZE_HEADER];
		memcpy_s(id, 16, ptr->recvBuf + SIZE_HEADER + 1, lenid);
		id[lenid] = 0;

		unsigned short cmd = CMD_GET_USER_LIST_INROOM;
		memcpy_s(threadBuf, SIZE_THREAD_BUF, &cmd, 2);

		unsigned short idx = SIZE_HEADER;
		threadBuf[idx++] = lenid;
		memcpy_s(threadBuf + idx, SIZE_THREAD_BUF, &id, lenid);
		idx += lenid;

		memcpy_s(threadBuf + 2, SIZE_THREAD_BUF, &idx, 2);

		Req_DB(threadBuf, idx, threadBuf, SIZE_THREAD_BUF, threadNum);
		char *id_arr[MAX_MEMBERCNT_INROOM];
		int cntId = ParseRecvBuf_DBRecv(threadBuf, id_arr);

		Req_AllUserPos_InRoom(hcp, ptr, id_arr, cntId, threadNum);
	}
}


void Scatter_CMD_InRoom(char *threadBuf, HANDLE hcp, SOCKETINFO *ptr, int threadNum, TYPE_HEADER_CMD Cmd_Send)
{
	unsigned char lenid = 0;
	char id[16];
	Get_mapUserId(ptr, id, 16, &lenid);

	unsigned short cmd = CMD_GET_USER_LIST_INROOM;
	memcpy_s(threadBuf, 1024, &cmd, 2);
	unsigned short idx = SIZE_HEADER;
	threadBuf[idx++] = lenid;
	memcpy_s(threadBuf + idx, 1024 - idx, id, lenid);
	idx += lenid;
	memcpy_s(threadBuf + 2, 1024 - 2, &idx, 2);
	Req_DB(threadBuf, idx, threadBuf, SIZE_THREAD_BUF, threadNum);


	char buf[1024];
	memcpy_s(buf, 1024, &Cmd_Send, 2);
	idx = SIZE_HEADER;
	memcpy_s(buf + 2, 1024 - 2, &idx, 2);
	char *split_buf[MAX_MEMBERCNT_INROOM];
	int cnt = ParseRecvBuf_DBRecv(threadBuf, split_buf);
	Scatter_Buf_InRoom(hcp, ptr, split_buf, cnt, buf, idx, threadNum);
}
//void ScatterRoomStart(char *threadBuf, HANDLE hcp, SOCKETINFO *ptr, int threadNum)
//{
//	unsigned char lenid = 0;
//	char id[16];
//	Get_mapUserId(ptr, id, 16, &lenid);
//
//	unsigned short cmd = CMD_GET_USER_LIST_INROOM;
//	memcpy_s(threadBuf, 1024, &cmd, 2);
//	unsigned short idx = SIZE_HEADER;
//	threadBuf[idx++] = lenid;
//	memcpy_s(threadBuf + idx, 1024 - idx, id, lenid);
//	idx += lenid;
//	memcpy_s(threadBuf + 2, 1024 - 2, &idx, 2);
//	Req_DB(threadBuf, idx, threadBuf, SIZE_THREAD_BUF, threadNum);
//
//
//	char buf[1024];
//	cmd = CMD_START_ROOM;
//	memcpy_s(buf, 1024, &cmd, 2);
//	idx = SIZE_HEADER;
//	memcpy_s(buf + 2, 1024 - 2, &idx, 2);
//	char *split_buf[MAX_MEMBERCNT_INROOM];
//	int cnt = ParseRecvBuf_DBRecv(threadBuf, split_buf);
//	Scatter_Buf_InRoom(hcp, ptr, split_buf, cnt, buf, idx, threadNum);
//}
//void VoteStart(char *threadBuf, HANDLE hcp, SOCKETINFO *ptr, int threadNum)
//{
//	unsigned char lenid = 0;
//	char id[16];
//	Get_mapUserId(ptr, id, 16, &lenid);
//
//	unsigned short cmd = CMD_GET_USER_LIST_INROOM;
//	memcpy_s(threadBuf, 1024, &cmd, 2);
//	unsigned short idx = SIZE_HEADER;
//	threadBuf[idx++] = lenid;
//	memcpy_s(threadBuf + idx, 1024 - idx, id, lenid);
//	idx += lenid;
//	memcpy_s(threadBuf + 2, 1024 - 2, &idx, 2);
//	Req_DB(threadBuf, idx, threadBuf, SIZE_THREAD_BUF, threadNum);
//
//
//	char buf[1024];
//	cmd = CMD_VOTE_START;
//	memcpy_s(buf, 1024, &cmd, 2);
//	idx = SIZE_HEADER;
//	memcpy_s(buf + 2, 1024 - 2, &idx, 2);
//	char *split_buf[MAX_MEMBERCNT_INROOM];
//	int cnt = ParseRecvBuf_DBRecv(threadBuf, split_buf);
//	Scatter_Buf_InRoom(hcp, ptr, split_buf, cnt, buf, idx, threadNum);
//}
void Main_WorkServerLogic(char *threadBuf, HANDLE hcp, unsigned int cbTransferred, SOCKETINFO *ptr, int threadNum)
{
	ptr->recievedbytes += cbTransferred;

	unsigned short length = *(unsigned short *)&ptr->recvBuf[2];
	while (ptr->recievedbytes >= SIZE_HEADER && length <= ptr->recievedbytes)
	{
		switch (*(unsigned short *)&ptr->recvBuf)
		{
		case CMD_LOGOUT:
		case CMD_GET_ROOM_LIST:
		case CMD_VOTE_AGREE:
		case CMD_VOTE_DISAGREE:
		case CMD_GET_VOTE_RESULT:
		case CMD_GET_USER_INFO:
		case CMD_FRIEND_FIND_ID:
		case CMD_FRIEND_FIND_NAME:
		case CMD_FRIEND_ADD:
		case CMD_GET_FRIEND_LIST:
		case CMD_STAGE_RESULT:
			Req_DB(ptr->recvBuf, length, threadBuf, SIZE_THREAD_BUF, threadNum);
			SendBuf_ptr(ptr, hcp, threadBuf, *(TYPE_HEADER_LENGTH *)(threadBuf + SIZE_HEADER_CMD), threadNum);
			break;
		case CMD_START_ROOM:
			Req_DB(ptr->recvBuf, length, threadBuf, SIZE_THREAD_BUF, threadNum);
			SendBuf_ptr(ptr, hcp, threadBuf, *(TYPE_HEADER_LENGTH *)(threadBuf + SIZE_HEADER_CMD), threadNum);
			Scatter_CMD_InRoom(threadBuf, hcp, ptr, threadNum, CMD_START_ROOM);
			break;
		case CMD_LOAD_PROFILE_IMAGE:
			Req_DB(ptr->recvBuf, length, threadBuf, SIZE_THREAD_BUF, threadNum);
			SendBuf_ptr_img(ptr, hcp, threadBuf, *(int *)(threadBuf + SIZE_HEADER_CMD), threadNum);
			break;
		case CMD_CREATE_ROOM:
			CreateRoom(threadBuf, hcp, ptr, threadNum, length);
			break;
		case CMD_LEAVE_ROOM:
			LeaveRoom(threadBuf, hcp, ptr, threadNum);
			break;
		case CMD_ENTER_ROOM:
			EnterRoom(threadBuf, hcp, ptr, threadNum, length);
			break;
		case CMD_LOGED_IN:
			LogedIn(threadBuf, hcp, ptr, threadNum);
			break;
		case CMD_KEY_PACKET:
			//PrintPacket(ptr, ptr->recvBuf, *(TYPE_HEADER_LENGTH *)(ptr->recvBuf + SIZE_HEADER_LENGTH), "Recv");
			TradeKeyPacket(threadBuf, hcp, ptr, threadNum, length);
			//printf("_____ %s : ", mapUserInfo_InRoom_ptr[ptr]->Id);
			//for (int i = 0; i < 8; i++)
			//{
			//	printf("%d ", (mapUserInfo_InRoom_ptr[ptr]->Curkey >> i) & 1);
			//}
			//printf("\n");
			break;
		case CMD_POS_STATUS_PACKET:
			Set_Pos_Status_Packet(ptr);
			break;
		case CMD_GET_POS_STATUS_PACKET:
			Get_Pos_Status_Packet(threadBuf, hcp, ptr, threadNum);
			break;
		case CMD_VOTE_START:
			Scatter_CMD_InRoom(threadBuf, hcp, ptr, threadNum, CMD_VOTE_START);
			break;
		case CMD_VOTE_NULLITY:
			break;
		default:
			break;
		}
		ptr->recievedbytes = ptr->recievedbytes - length;
		memcpy(ptr->recvBuf, ptr->recvBuf + length, ptr->recievedbytes);

		length = *(unsigned short *)&ptr->recvBuf[2];
	}
}