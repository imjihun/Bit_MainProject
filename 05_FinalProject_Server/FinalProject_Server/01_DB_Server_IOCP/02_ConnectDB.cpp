
#include "02_ConnectDB.h"

const char *host = "localhost";
const char *user = "root";
const char *pw = "opgg1004";
const char *db = "mysql_test";


MYSQL *connection = NULL;
MYSQL conn;
char query[1024];

CRITICAL_SECTION DB_CS;
void init_Critical_Section(CRITICAL_SECTION *pDB_CS)
{
	InitializeCriticalSection(pDB_CS);
}
void del_Critical_Section(CRITICAL_SECTION *pDB_CS)
{
	DeleteCriticalSection(pDB_CS);
}

void connectDB()
{
	if (mysql_init(&conn) == NULL)
	{
		ViewError("mysql_init() error!");
	}

	connection = mysql_real_connect(&conn, host, user, pw, db, 3306, (const char*)NULL, 0);
	if (connection == NULL)    // 연결 에러 처리
	{
		ViewError("%d 에러 : %s, %d\n", mysql_errno(&conn), mysql_error(&conn));
	}
	else
	{
		ViewMessage("MySQL DB Direct Connected!");    // 연결 성공 메시지 출력

		if (mysql_select_db(&conn, db))    // 데이터베이스 선택
		{
			ViewError("%d 에러 : %s, %d\n", mysql_errno(&conn), mysql_error(&conn));
		}
	}
	init_Critical_Section(&DB_CS);
}
void closeDB()
{
	mysql_close(connection);        // db서버 종료
	connection = NULL;
	del_Critical_Section(&DB_CS);
}

void ViewMySQLError(MYSQL *connect)
{
	ViewMessage("\t\t\t%s", mysql_error(connect));
}

int ParseRecvBuf(char *src, char **dst)
{
	int idx_buf = SIZE_HEADER;
	unsigned short lenth = *(unsigned short *)(src + 2);
	int len = 0;
	int cnt = 0;
	for (int i = 0; idx_buf < lenth; idx_buf += len + 1)
	{
		len = (unsigned char)src[idx_buf];
		src[idx_buf] = '\0';
		dst[i++] = &src[idx_buf + 1];
		cnt++;
	}
	src[idx_buf] = '\0';
	return cnt;
}
void MakeSendPacket_CMD(char *SendBuf, unsigned short CMD, unsigned short *Len)
{
	memcpy(SendBuf, &CMD, 2);
	*Len = 4;
	memcpy(SendBuf + 2, Len, 2);
}
void MakeSendPacket_CMD(char *SendBuf, unsigned short CMD, int *Len)
{
	memcpy(SendBuf, &CMD, 2);
	*Len = 6;
	memcpy(SendBuf + 2, Len, 4);
}

int _Join(char **split_buf)
{
	// CMD(2) : Length(2)
	// : len_id(1) : id() : len_pw(1) : pw() : len_name(1) : name() : len_attach(1) : attach()////////{ : len_profilePath(1) : profilePath()}
	int retval = 0;

	sprintf_s(query, "Insert into userinfo (User_Id, User_Password, User_Name, User_Attach) VALUES('%s', '%s', '%s', '%s')",
		split_buf[0], split_buf[1], split_buf[2], split_buf[3]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}
	return DB_SUCCESS;
}
int Join(char **split_buf, char *SendBuf, unsigned short *Len_SendBuf)
{
	// CMD(2) : Length(2)
	// : len_id(1) : id() : len_pw(1) : pw() : len_name(1) : name() : len_attach(1) : attach(){ : len_profilePath(1) : profilePath()}
	if (_Join(split_buf) == DB_SUCCESS)
		MakeSendPacket_CMD(SendBuf, CMD_JOIN_SUCCESS, Len_SendBuf);
	else
		MakeSendPacket_CMD(SendBuf, CMD_JOIN_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}
int _Login(char **split_buf)
{
	// CMD(2) : Length(2)
	// : len_id(1) : id() : len_pw(1) : pw()
	int retval = 0;
	//sprintf_s(query, "Insert into user_room (User_Id) SELECT '%s' FROM DUAL WHERE EXISTS (Select * from userinfo where User_Id='%s' and User_Password='%s')",
	//	split_buf[0], split_buf[0], split_buf[1]);

	sprintf_s(query, "Insert into user_room (User_Id) VALUE('%s')",
		split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}
	return DB_SUCCESS;
}
int Login(char **split_buf, char *SendBuf, unsigned short *Len_SendBuf)
{
	// CMD(2) : Length(2)
	// : len_id(1) : id() : len_pw(1) : pw()

	// CMD(2) : Length(2)
	if (_Login(split_buf) == DB_SUCCESS)
		MakeSendPacket_CMD(SendBuf, CMD_LOGIN_SUCCESS, Len_SendBuf);
	else
		MakeSendPacket_CMD(SendBuf, CMD_LOGIN_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}
int _LogOut(char **split_buf)
{
	// User_Id
	int retval = 0;

	sprintf_s(query, "DELETE FROM user_room WHERE User_Id='%s'",
		split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}

	return DB_SUCCESS;
}
int LogOut(char **split_buf, char *SendBuf, unsigned short *Len_SendBuf)
{
	if (_LogOut(split_buf) == DB_SUCCESS)
		MakeSendPacket_CMD(SendBuf, CMD_DB_SUCCESS, Len_SendBuf);
	else
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}

int _CreateRoom(char **split_buf)
{
	// User_Id, Room_Number, RoomMode_Id, Room_Subject, Room_Title, Room_MaxMemberCount, Room_CurMemberCount, Room_State
	int retval = 0;

	sprintf_s(query, "INSERT INTO roominfo(Room_Number, RoomMode_Id, Room_Subject, Room_Title, Room_MaxMemberCount, Room_CurMemberCount, Room_State, Room_Host_Id) VALUES(%d, %d, '%s', '%s', %d, %d, %d, '%s')",
		atoi(split_buf[1]), atoi(split_buf[2]), split_buf[3], split_buf[4], atoi(split_buf[5]), atoi(split_buf[6]), atoi(split_buf[7]), split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}


	sprintf_s(query, "UPDATE user_room SET Room_Number=%d, bAgree=0 WHERE User_Id='%s'",
		atoi(split_buf[1]), split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}

	return DB_SUCCESS;
}
int CreateRoom(char **split_buf, char *SendBuf, unsigned short *Len_SendBuf)
{
	if (_CreateRoom(split_buf) == DB_SUCCESS)
		MakeSendPacket_CMD(SendBuf, CMD_CREATE_ROOM, Len_SendBuf);
	else
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}
int _StartRoom(char **split_buf)
{
	// user_Id
	int retval = 0;

	sprintf_s(query, "UPDATE roominfo SET Room_State=1 WHERE Room_Host_Id='%s'",
		split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}

	return DB_SUCCESS;
}
int StartRoom(char **split_buf, char *SendBuf, unsigned short *Len_SendBuf)
{
	if (_StartRoom(split_buf) == DB_SUCCESS)
		MakeSendPacket_CMD(SendBuf, CMD_DB_SUCCESS, Len_SendBuf);
	else
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}
int _GetRoomList(char *retSendBuf)
{
	// send
	// Room_Number, RoomMode_Id, Room_Subject, Room_Title, Room_MaxMemberCount, Room_CurMemberCount, Room_State, Room_Host_Id
	int retval = 0;

	sprintf_s(query, "SELECT * From roominfo");
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}

	MYSQL_RES *sql_result;
	MYSQL_ROW sql_row;
	sql_result = mysql_store_result(connection);            // Result Set에 저장

	int sql_fields = mysql_num_fields(sql_result);

	unsigned char len = 0;
	unsigned short idx_retBuf = 4;
	while ((sql_row = mysql_fetch_row(sql_result)) != NULL)    // Result Set에서 1개씩 배열에 가져옴
	{
		for (int i = 0; i < sql_fields; i++)
		{
			len = (unsigned char)strlen(sql_row[i]);
			memcpy(retSendBuf + idx_retBuf++, &len, 1);
			memcpy(retSendBuf + idx_retBuf, sql_row[i], len);    // 저장된 배열을 출력
			idx_retBuf += len;
		}
		printf("\n");
	}
	mysql_free_result(sql_result);        // Result Set 해제한다

	unsigned short cmd = CMD_GET_ROOM_LIST;
	memcpy(retSendBuf, (unsigned char *)&cmd, 2);
	memcpy(retSendBuf + 2, (unsigned char *)&idx_retBuf, 2);

	return DB_SUCCESS;
}
int GetRoomList(char *SendBuf, unsigned short *Len_SendBuf)
{
	if (_GetRoomList(SendBuf) == DB_SUCCESS)
		*Len_SendBuf = *(unsigned short*)(SendBuf + 2);
	else
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}

int _GetUserInfo(char **split_buf, char *SendBuf)
{
	// recv
	// CMD(2) : Len(2)
	// : lenId(1) : User_Id()
	int retval = 0;

	sprintf_s(query, "SELECT * From userinfo WHERE User_Id='%s'", split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}

	// send
	// User_Name, User_Attach, User_TotalPart, User_Win, User_Lose, User_ProfilePath
	MYSQL_RES *sql_result;
	MYSQL_ROW sql_row;
	sql_result = mysql_store_result(connection);            // Result Set에 저장

	unsigned char len = 0;
	unsigned short idx_retBuf = 4;

	if ((sql_row = mysql_fetch_row(sql_result)) != NULL)    // Result Set에서 1개씩 배열에 가져옴
	{
		int i = 1;

		len = (unsigned char)strlen(sql_row[i]);
		memcpy(SendBuf + idx_retBuf++, &len, 1);
		memcpy(SendBuf + idx_retBuf, sql_row[i], len);
		idx_retBuf += len;
		i += 2;

		len = (unsigned char)strlen(sql_row[i]);
		memcpy(SendBuf + idx_retBuf++, &len, 1);
		memcpy(SendBuf + idx_retBuf, sql_row[i], len);
		idx_retBuf += len;
		i++;

		if (sql_row[i] != NULL)
		{
			len = (unsigned char)strlen(sql_row[i]);
			memcpy(SendBuf + idx_retBuf++, &len, 1);
			memcpy(SendBuf + idx_retBuf, sql_row[i], len);
			idx_retBuf += len;
		}
		else
		{
			len = 0;
			memcpy(SendBuf + idx_retBuf++, &len, 1);
			idx_retBuf += len;
		}
		i += 2;

		int t = atoi(sql_row[i]);
		memcpy(SendBuf + idx_retBuf, &t, 4);
		idx_retBuf += 4;
		i++;

		t = atoi(sql_row[i]);
		memcpy(SendBuf + idx_retBuf, &t, 4);
		idx_retBuf += 4;
		i++;

		t = atoi(sql_row[i]);
		memcpy(SendBuf + idx_retBuf, &t, 4);
		idx_retBuf += 4;
		i++;
		mysql_free_result(sql_result);        // Result Set 해제한다

		memcpy(SendBuf + 2, (unsigned char *)&idx_retBuf, 2);

		return DB_SUCCESS;
	}
	else
	{
		mysql_free_result(sql_result);        // Result Set 해제한다
		return DB_FAIL;
	}
}
int GetUserInfo(unsigned short CMD, char **split_buf, char *SendBuf, unsigned short *Len_SendBuf)
{
	if (_GetUserInfo(split_buf, SendBuf) == DB_SUCCESS)
	{
		memcpy(SendBuf, &CMD, 2);
		*Len_SendBuf = *(unsigned short*)(SendBuf + 2);
	}
	else
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}
int _GetUserInfoByName(char **split_buf, char *SendBuf)
{
	// recv
	// CMD(2) : Len(2)
	// : lenId(1) : User_Id()
	int retval = 0;

	sprintf_s(query, "SELECT * From userinfo WHERE User_Name='%s'", split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}

	// send
	// User_Name, User_Attach, User_TotalPart, User_Win, User_Lose, User_ProfilePath
	MYSQL_RES *sql_result;
	MYSQL_ROW sql_row;
	sql_result = mysql_store_result(connection);            // Result Set에 저장

	unsigned char len = 0;
	unsigned short idx_retBuf = 4;

	if ((sql_row = mysql_fetch_row(sql_result)) != NULL)    // Result Set에서 1개씩 배열에 가져옴
	{
		int i = 1;

		len = (unsigned char)strlen(sql_row[i]);
		memcpy(SendBuf + idx_retBuf++, &len, 1);
		memcpy(SendBuf + idx_retBuf, sql_row[i], len);
		idx_retBuf += len;
		i+=2;

		len = (unsigned char)strlen(sql_row[i]);
		memcpy(SendBuf + idx_retBuf++, &len, 1);
		memcpy(SendBuf + idx_retBuf, sql_row[i], len);
		idx_retBuf += len;
		i++;

		if (sql_row[i] != NULL)
		{
			len = (unsigned char)strlen(sql_row[i]);
			memcpy(SendBuf + idx_retBuf++, &len, 1);
			memcpy(SendBuf + idx_retBuf, sql_row[i], len);
			idx_retBuf += len;
		}
		else
		{
			len = 0;
			memcpy(SendBuf + idx_retBuf++, &len, 1);
			idx_retBuf += len;
		}
		i += 2;

		int t = atoi(sql_row[i]);
		memcpy(SendBuf + idx_retBuf, &t, 4);
		idx_retBuf += 4;
		i++;

		t = atoi(sql_row[i]);
		memcpy(SendBuf + idx_retBuf, &t, 4);
		idx_retBuf += 4;
		i++;

		t = atoi(sql_row[i]);
		memcpy(SendBuf + idx_retBuf, &t, 4);
		idx_retBuf += 4;
		i++;
		mysql_free_result(sql_result);        // Result Set 해제한다

		memcpy(SendBuf + 2, (unsigned char *)&idx_retBuf, 2);

		return DB_SUCCESS;
	}
	else
	{
		mysql_free_result(sql_result);        // Result Set 해제한다
		return DB_FAIL;
	}
}
int GetUserInfoByName(unsigned short CMD, char **split_buf, char *SendBuf, unsigned short *Len_SendBuf)
{
	if (_GetUserInfoByName(split_buf, SendBuf) == DB_SUCCESS)
	{
		memcpy(SendBuf, &CMD, 2);
		*Len_SendBuf = *(unsigned short*)(SendBuf + 2);
	}
	else
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}
int _SaveProfileImage(char *id, char *path)
{
	// CMD(2) : Length(2)
	// : len_id(1) : id() : len_pw(1) : pw()
	int retval = 0;
	//sprintf_s(query, "Insert into user_room (User_Id) SELECT '%s' FROM DUAL WHERE EXISTS (Select * from userinfo where User_Id='%s' and User_Password='%s')",
	//	split_buf[0], split_buf[0], split_buf[1]);

	EnterCriticalSection(&DB_CS);
	sprintf_s(query, "Update userinfo Set User_ProfilePath='%s' Where User_Id='%s'",
		path, id);
	retval = mysql_query(connection, query);
	LeaveCriticalSection(&DB_CS);

	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}
	return DB_SUCCESS;
}
int SaveProfileImage(char *Packet, long Len_Pack, char *SendBuf, unsigned short *Len_SendBuf)
{
	char id[17];
	memcpy(id, Packet + 7, Packet[6]);
	id[Packet[6]] = 0;
	long Len_Data = Len_Pack - (7 + Packet[6]);

	char path[256];
	sprintf_s(path, "%s/%s.jpg", PATH_SAVE_IMAGE, id);

	int err = 0;
	FILE *fp = NULL;
	if ((err = fopen_s(&fp, path, "wb")) != 0)
	{
		ViewMessage("FILEOPEN : fopen_s() Error = %d", err);
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);
		return EXSIST_SEND_BUF;
	}

	size_t Len_Writed = 0;
	Len_Writed = fwrite(Packet + 7 + Packet[6], 1, Len_Data, fp);
	if (Len_Writed != Len_Data)
	{
		ViewMessage("FILEOPEN : fwrite() Error");
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);
		return EXSIST_SEND_BUF;
	}

	fclose(fp);

	if (_SaveProfileImage(id, path) == DB_SUCCESS)
		MakeSendPacket_CMD(SendBuf, CMD_DB_SUCCESS, Len_SendBuf);
	else
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);

	return EXSIST_SEND_BUF;
}

int _AddFriend(char **split_buf)
{
	// CMD(2) : LEN(2)
	// : lenid(1) : id() : lenotherid(1) : otherid()
	int retval = 0;

	sprintf_s(query, "Insert Into friendinfo(Friend_User_Id, Friend_Other_Id) Value('%s', '%s')",
		split_buf[0], split_buf[1]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}

	return DB_SUCCESS;
}
int AddFriend(char **split_buf, char *SendBuf, unsigned short *Len_SendBuf)
{
	if (_AddFriend(split_buf) == DB_SUCCESS)
		MakeSendPacket_CMD(SendBuf, CMD_FRIEND_ADD_SUCCESS, Len_SendBuf);
	else
		MakeSendPacket_CMD(SendBuf, CMD_FRIEND_ADD_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}
int _GetUserProfileImage(char **split_buf, char* path)
{
	// CMD(2) : LEN(2)
	// : lenid(1) : id() : lenotherid(1) : otherid()
	int retval = 0;

	sprintf_s(query, "Select User_ProfilePath From userinfo Where User_Id='%s'",
		split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}

	// send
	// User_Name, User_Attach, User_TotalPart, User_Win, User_Lose, User_ProfilePath
	MYSQL_RES *sql_result;
	MYSQL_ROW sql_row;
	sql_result = mysql_store_result(connection);            // Result Set에 저장
	if ((sql_row = mysql_fetch_row(sql_result)) != NULL && sql_row[0] != NULL)    // Result Set에서 1개씩 배열에 가져옴
	{
		int len = strlen(sql_row[0]);
		memcpy_s(path, 256, sql_row[0], len);
		path[len] = 0;

		mysql_free_result(sql_result);        // Result Set 해제한다
		return DB_SUCCESS;
	}
	else
	{
		mysql_free_result(sql_result);        // Result Set 해제한다
		return DB_FAIL;
	}
}
int _GetUserProfileImage(char **split_buf, char *SendBuf, int *Len_SendBuf)
{
	char path[256];
	try
	{
		EnterCriticalSection(&DB_CS);
		if (_GetUserProfileImage(split_buf, path) != DB_SUCCESS)
		{
			LeaveCriticalSection(&DB_CS);
			throw 0;
		}
		LeaveCriticalSection(&DB_CS);
		unsigned short cmd = CMD_LOAD_PROFILE_IMAGE;
		memcpy_s(SendBuf, SIZE_BUF, &cmd, 2);
		unsigned char len_id = (unsigned char)strlen(split_buf[0]);
		memcpy_s(SendBuf + 6, SIZE_BUF - 6, &len_id, 1);
		memcpy_s(SendBuf + 7, SIZE_BUF - 7, split_buf[0], len_id);

		int err = 0;
		FILE *fp = NULL;
		if ((err = fopen_s(&fp, path, "rb")) != 0)
		{
			ViewMessage("FILEOPEN : fopen_s() Error = %d", err);
			throw 0;
		}

		size_t Len_Read = 0;
		Len_Read = fread_s(SendBuf + 7 + SendBuf[6], SIZE_BUF - (7 + SendBuf[6]), 1, SIZE_BUF - (7 + SendBuf[6]), fp);
		if (ferror(fp) != 0)
		{
			ViewMessage("FILEOPEN : fwrite() Error");
			throw 0;
		}

		int len = ftell(fp) + 7 + SendBuf[6];
		memcpy_s(SendBuf + 2, SIZE_BUF - 2, &len, 4);
		fclose(fp);
		*Len_SendBuf = len;
	}
	catch (int e)
	{
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);
	}
	return EXSIST_SEND_BUF;
}
int GetUserProfileImage(char *Packet, unsigned short Len_Pack, char *SendBuf, int *Len_SendBuf)
{
	char* split_buf[20];
	ParseRecvBuf(Packet, split_buf);

	return _GetUserProfileImage(split_buf, SendBuf, Len_SendBuf);
}

int _GetFriendList(char **split_buf, char *retSendBuf)
{
	// send
	// Room_Number, RoomMode_Id, Room_Subject, Room_Title, Room_MaxMemberCount, Room_CurMemberCount, Room_State
	int retval = 0;

	sprintf_s(query, "SELECT Friend_Other_Id From friendinfo Where Friend_User_Id='%s'", split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}

	MYSQL_RES *sql_result;
	MYSQL_ROW sql_row;
	sql_result = mysql_store_result(connection);            // Result Set에 저장

	unsigned int sql_fields = mysql_num_fields(sql_result);

	unsigned char len = 0;
	unsigned short idx_retBuf = 4;
	while ((sql_row = mysql_fetch_row(sql_result)) != NULL)    // Result Set에서 1개씩 배열에 가져옴
	{
		len = (unsigned char)strlen(sql_row[0]);
		memcpy(retSendBuf + idx_retBuf++, &len, 1);
		memcpy(retSendBuf + idx_retBuf, sql_row[0], len);    // 저장된 배열을 출력
		idx_retBuf += len;

	}
	mysql_free_result(sql_result);        // Result Set 해제한다

	unsigned short cmd = CMD_GET_FRIEND_LIST;
	memcpy(retSendBuf, (unsigned char *)&cmd, 2);
	memcpy(retSendBuf + 2, (unsigned char *)&idx_retBuf, 2);

	return DB_SUCCESS;
}
int GetFriendList(char **split_buf, char *SendBuf, unsigned short *Len_SendBuf)
{
	if (_GetFriendList(split_buf, SendBuf) == DB_SUCCESS)
		*Len_SendBuf = *(unsigned short*)(SendBuf + 2);
	else
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}

int _GetUserListInRoom(char **split_buf, char *retSendBuf)
{
	//recv
	// CMD : LEN
	// : lenid : id;

	// send
	// CMD : LEN
	// : lenid : id .....
	int retval = 0;

	sprintf_s(query, "SELECT User_Id From user_room Where Room_Number=(SELECT Room_Number From user_room Where User_Id='%s') And Room_Number!=0", split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}

	MYSQL_RES *sql_result;
	MYSQL_ROW sql_row;
	sql_result = mysql_store_result(connection);            // Result Set에 저장

	unsigned int sql_fields = mysql_num_fields(sql_result);

	unsigned char len = 0;
	unsigned short idx_retBuf = 4;
	while ((sql_row = mysql_fetch_row(sql_result)) != NULL)    // Result Set에서 1개씩 배열에 가져옴
	{
		len = (unsigned char)strlen(sql_row[0]);
		memcpy(retSendBuf + idx_retBuf++, &len, 1);
		memcpy(retSendBuf + idx_retBuf, sql_row[0], len);    // 저장된 배열을 출력
		idx_retBuf += len;

	}
	mysql_free_result(sql_result);        // Result Set 해제한다

	unsigned short cmd = CMD_GET_USER_LIST_INROOM;
	memcpy(retSendBuf, (unsigned char *)&cmd, 2);
	memcpy(retSendBuf + 2, (unsigned char *)&idx_retBuf, 2);

	return DB_SUCCESS;
}
int GetUserListInRoom(char **split_buf, char *SendBuf, unsigned short *Len_SendBuf)
{
	if (_GetUserListInRoom(split_buf, SendBuf) == DB_SUCCESS)
		*Len_SendBuf = *(unsigned short*)(SendBuf + 2);
	else
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}
int _EnterRoom(char **split_buf, char *SendBuf)
{
	// User_Id, Room_Number
	int retval = 0;

	sprintf_s(query, "UPDATE user_room SET Room_Number=%d, bAgree=0 WHERE User_Id='%s'",
		atoi(split_buf[1]), split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}

	return _GetUserListInRoom(split_buf, SendBuf);
}
int EnterRoom(char **split_buf, char *SendBuf, unsigned short *Len_SendBuf)
{
	if (_EnterRoom(split_buf, SendBuf) == DB_SUCCESS)
		*Len_SendBuf = *(unsigned short*)(SendBuf + 2);
	else
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}
int _LeaveRoom(char **split_buf, char *SendBuf)
{
	// User_Id
	int retval;

	sprintf_s(query, "SELECT Room_Number From user_room Where User_Id='%s'", split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}

	MYSQL_RES *sql_result;
	MYSQL_ROW sql_row;
	sql_result = mysql_store_result(connection);            // Result Set에 저장

	unsigned int sql_fields = mysql_num_fields(sql_result);

	int roomNum = 0;
	while ((sql_row = mysql_fetch_row(sql_result)) != NULL)    // Result Set에서 1개씩 배열에 가져옴
	{
		roomNum = atoi(sql_row[0]);
	}
	mysql_free_result(sql_result);        // Result Set 해제한다

	//////////////////////////////////////////////////////////////////////////
	sprintf_s(query, "UPDATE user_room SET Room_Number=%d WHERE User_Id='%s'",
		0, split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}
	//////////////////////////////////////////////////////////////////////////
	sprintf_s(query, "SELECT User_Id From user_room Where Room_Number=%d And Room_Number!=0", roomNum);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}

	sql_result = mysql_store_result(connection);            // Result Set에 저장

	sql_fields = mysql_num_fields(sql_result);

	unsigned char len = 0;
	unsigned short idx_retBuf = 4;
	while ((sql_row = mysql_fetch_row(sql_result)) != NULL)    // Result Set에서 1개씩 배열에 가져옴
	{
		len = (unsigned char)strlen(sql_row[0]);
		memcpy(SendBuf + idx_retBuf++, &len, 1);
		memcpy(SendBuf + idx_retBuf, sql_row[0], len);    // 저장된 배열을 출력
		idx_retBuf += len;

	}
	mysql_free_result(sql_result);        // Result Set 해제한다

	unsigned short cmd = CMD_LEAVE_ROOM;
	memcpy(SendBuf, (unsigned char *)&cmd, 2);
	memcpy(SendBuf + 2, (unsigned char *)&idx_retBuf, 2);

	return DB_SUCCESS;
}
int LeaveRoom(char **split_buf, char *SendBuf, unsigned short *Len_SendBuf)
{
	if (_LeaveRoom(split_buf, SendBuf) == DB_SUCCESS)
		*Len_SendBuf = *(unsigned short*)(SendBuf + 2);
	else
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}

int _AgreeInStage(char **split_buf)
{
	// CMD(2) : Length(2)
	// : len_id(1) : id()
	int retval = 0;

	sprintf_s(query, "UPDATE user_room SET bAgree=1 WHERE User_Id='%s'",
		split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}
	return DB_SUCCESS;
}
int AgreeInStage(char **split_buf, char *SendBuf, unsigned short *Len_SendBuf)
{
	// CMD(2) : Length(2)
	// : len_id(1) : id(len_id)
	if (_AgreeInStage(split_buf) == DB_SUCCESS)
		MakeSendPacket_CMD(SendBuf, CMD_VOTE_AGREE, Len_SendBuf);
	else
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}
int _DisagreeInStage(char **split_buf)
{
	// CMD(2) : Length(2)
	// : len_id(1) : id()
	int retval = 0;

	sprintf_s(query, "UPDATE user_room SET bAgree=2 WHERE User_Id='%s'",
		split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}
	return DB_SUCCESS;
}
int DisagreeInStage(char **split_buf, char *SendBuf, unsigned short *Len_SendBuf)
{
	// CMD(2) : Length(2)
	// : len_id(1) : id(len_id)
	if (_DisagreeInStage(split_buf) == DB_SUCCESS)
		MakeSendPacket_CMD(SendBuf, CMD_VOTE_DISAGREE, Len_SendBuf);
	else
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}

int _GetResult(char **split_buf, char *SendBuf)
{
	// CMD(2) : Length(2)
	// : lenId(1) : Id;
	int retval;

	sprintf_s(query, "SELECT * From roomresultinfo Where Room_Number=(Select Room_Number From user_room Where User_Id='%s')", split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}

	MYSQL_RES *sql_result;
	MYSQL_ROW sql_row;
	sql_result = mysql_store_result(connection);            // Result Set에 저장

	unsigned int sql_fields = mysql_num_fields(sql_result);

	while ((sql_row = mysql_fetch_row(sql_result)) != NULL)    // Result Set에서 1개씩 배열에 가져옴
	{
		SendBuf[4] = atoi(sql_row[1]);
		SendBuf[5] = atoi(sql_row[2]);
		SendBuf[6] = atoi(sql_row[3]);
	}
	mysql_free_result(sql_result);        // Result Set 해제한다

	///////////////////////////////////////////////////////////////////////////////
	
	sprintf_s(query, "UPDATE roomresultinfo SET RoomResult_Result=%d  WHERE Room_Number=(Select Room_Number From user_room Where User_Id='%s')", 
		SendBuf[4] > SendBuf[5] + SendBuf[6] ? 1 : 2,
		split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}

	//sprintf_s(query, "DELETE FROM roomresultinfo WHERE Room_Number=(Select Room_Number From user_room Where User_Id='%s')",
	//	split_buf[0]);
	//retval = mysql_query(connection, query);
	//if (retval != 0)
	//{
	//	ViewMySQLError(connection); return DB_FAIL;
	//}
	// Return
	// CMD(2) : Length(2)
	// : Agree : Disagree, Nullity

	unsigned short cmd = CMD_GET_VOTE_RESULT;
	unsigned short len = 7;
	memcpy(SendBuf, &cmd, 2);
	memcpy(SendBuf + 2, &len, 2);

	return DB_SUCCESS;
}
int GetResult(char **split_buf, char *SendBuf, unsigned short *Len_SendBuf)
{
	// CMD(2) : Length(2)
	// : len_id(1) : id(len_id)
	if (_GetResult(split_buf, SendBuf) == DB_SUCCESS)
		*Len_SendBuf = *(unsigned short*)(SendBuf + 2);
	else
		MakeSendPacket_CMD(SendBuf, CMD_DB_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}

int _LogedIn(char **split_buf)
{
	// CMD(2) : Length(2)
	// : len_id(1) : id()
	int retval = 0;

	sprintf_s(query, "Insert into user_room (User_Id) VALUE('%s')",
		split_buf[0]);
	retval = mysql_query(connection, query);
	if (retval != 0)
	{
		ViewMySQLError(connection); return DB_FAIL;
	}
	return DB_SUCCESS;
}
int LogedIn(char **split_buf, char *SendBuf, unsigned short *Len_SendBuf)
{
	// CMD(2) : Length(2)
	// : len_id(1) : id()

	// CMD(2) : Length(2)
	if (_LogedIn(split_buf) == DB_SUCCESS)
		MakeSendPacket_CMD(SendBuf, CMD_LOGIN_SUCCESS, Len_SendBuf);
	else
		MakeSendPacket_CMD(SendBuf, CMD_LOGIN_FAIL, Len_SendBuf);
	return DB_SUCCESS;
}

int MAIN_DB(char *Packet, unsigned short Len_Pack, char *SendBuf, unsigned short *Len_SendBuf)
{
	int ret_val = 0;
	char* split_buf[20];
	ParseRecvBuf(Packet, split_buf);
	unsigned short cmd = *(unsigned short*)Packet;
	EnterCriticalSection(&DB_CS);
	switch (cmd)
	{
	case CMD_JOIN:
		ret_val = Join(split_buf, SendBuf, Len_SendBuf);
		break;
	case CMD_LOGIN:
		ret_val = Login(split_buf, SendBuf, Len_SendBuf);
		break;
	case CMD_LOGOUT:
		ret_val = LogOut(split_buf, SendBuf, Len_SendBuf);
		break;
	case CMD_CREATE_ROOM:
		ret_val = CreateRoom(split_buf, SendBuf, Len_SendBuf);
		break;
	case CMD_ENTER_ROOM:
		ret_val = EnterRoom(split_buf, SendBuf, Len_SendBuf);
		break;
	case CMD_LEAVE_ROOM:
		ret_val = LeaveRoom(split_buf, SendBuf, Len_SendBuf);
		break;
	case CMD_START_ROOM:
		ret_val = StartRoom(split_buf, SendBuf, Len_SendBuf);
		break;
	case CMD_GET_ROOM_LIST:
		ret_val = GetRoomList(SendBuf, Len_SendBuf);
		break;
	case CMD_GET_USER_INFO:
	case CMD_FRIEND_FIND_ID:
		ret_val = GetUserInfo(*(unsigned short *)Packet, split_buf, SendBuf, Len_SendBuf);
		break;
	case CMD_FRIEND_FIND_NAME:
		ret_val = GetUserInfoByName(*(unsigned short *)Packet, split_buf, SendBuf, Len_SendBuf);
		break;
	case CMD_GET_FRIEND_LIST:
		ret_val = GetFriendList(split_buf, SendBuf, Len_SendBuf);
		break;
	case CMD_FRIEND_ADD:
		ret_val = AddFriend(split_buf, SendBuf, Len_SendBuf);
		break;
	case CMD_GET_USER_LIST_INROOM:
		ret_val = GetUserListInRoom(split_buf, SendBuf, Len_SendBuf);
		break;
	case CMD_VOTE_AGREE:
		AgreeInStage(split_buf, SendBuf, Len_SendBuf);
		break;
	case CMD_VOTE_DISAGREE:
		DisagreeInStage(split_buf, SendBuf, Len_SendBuf);
		break;
	case CMD_GET_VOTE_RESULT:
		GetResult(split_buf, SendBuf, Len_SendBuf);
		break;
	case CMD_LOGED_IN:
		LogedIn(split_buf, SendBuf, Len_SendBuf);
		break;
	default:
		ret_val = FAIL_CMD;
		MakeSendPacket_CMD(SendBuf, CMD_NOT_FOUND_CMD, Len_SendBuf);
	}
	LeaveCriticalSection(&DB_CS);
	return EXSIST_SEND_BUF;
}