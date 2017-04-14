/* 2017-04-04 : created by JHLim */
/***************************************************************************
* Copyright (c) 2004-2016, eGlobal Systems, Co., Ltd.
* Seoul, Republic of Korea
* All Rights Reserved.
*
* Description : prototype.c
*       xxxx
*
***************************************************************************/

/*************** Header files *********************************************/
#include "connectDB.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/epoll.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>


#include "../../printLog/printLog.h"
#include "../../macroFile.h"
#include "../../aes/source/aes.h"

/*************** New Data Types *******************************************/

/*************** Definitions / Macros  ************************************/

/*************** Global Variables *****************************************/
int PORT = PORT_SERVER;

socket_info *g_arr_p_socket_info[MAX_CLIENT];
int g_cnt_socket = 0;

room_info g_arr_room_info[MAX_ROOM];
int g_cnt_room;

unsigned int g_cnt_chatting_log = 0;

TYPE_SECRET_KEY g_nonce[8] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
TYPE_SECRET_KEY g_iv[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

/*************** Prototypes ***********************************************/
int initListenfd(int *listenfd, int port);
int initEpoll(int *epollfd);
int addSocketEpoll(int epollfd, int sockfd);
int delSocketEpoll(int epollfd, socket_info * sockfd);
int mainChattingServer();
int recvProcessing(int epollfd, socket_info * sockfd);
int packetProcessing(socket_info *p_socket_info, char *packet);
int reuse(int sockfd);

int addSocketInfoArr(socket_info *p_socket_info);
int delSocketInfoArr(socket_info *p_socket_info);
int findSocketInfoArr(const char *id, socket_info **p_socket_info_ret);

int initRoomInfoArr();
int addRoomInfoArr(socket_info *p_socket_info, char is_secret, char *subject);
int delRoomInfoArr(TYPE_ROOM_NUMBER room_number);
int findRoomInfoArr(TYPE_ROOM_NUMBER room_number);

int addListSocketInRoom(TYPE_ROOM_NUMBER room_number, socket_info *p_socket_info);
int delListSocketInRoom(TYPE_ROOM_NUMBER room_number, socket_info *p_socket_info);
int delAllListSocketInRoom(TYPE_ROOM_NUMBER room_number);
int delListSocketAllRoom(socket_info *p_socket_info);
int findListSocketInRoom(TYPE_ROOM_NUMBER room_number, socket_info *p_socket_info);

int makeMessage(char *id, TYPE_ROOM_NUMBER room_number, char *message, int size_message, char *message_ret);
int resetFile(TYPE_ROOM_NUMBER room_number);
int writeChattingLog(TYPE_ROOM_NUMBER room_number, char *id, char *message, size_t size_message);
size_t readChattingLog(TYPE_ROOM_NUMBER room_number, char *id, char *buffer_ret, size_t size_buffer_ret);

int sprideChattingMessage(TYPE_ROOM_NUMBER room_number, char *id, char *message, int size_message);

int cmdCreateId(socket_info *p_socket_info);
int cmdCreateRoom(socket_info *p_socket_info);
int cmdEnterRoom(socket_info *p_socket_info);
int cmdLeaveRoom(socket_info *p_socket_info);
int cmdViewRoom(socket_info *p_socket_info);
int cmdTotalRoomList(socket_info *p_socket_info);
int cmdMyRoomList(socket_info *p_socket_info);
int cmdChattingMessage(socket_info *p_socket_info);


int initDaemon();

int requestDB(const char *query)
{

	return 0;
}

void makeKey(TYPE_SECRET_KEY *key)
{
	int i;

	for (i = 0; i < SIZE_SECRET_KEY; i++)
	{
		key[i] = rand() % MAX_TYPE_SECRET_KEY;
	}
}

int viewPacket(char *packet)
{

	TYPE_PACKET_LENGTH size = *(TYPE_PACKET_LENGTH *)(packet + SIZE_CMD);
	int i;


	printf("[cmd = %d, length = %d] [ ", *(TYPE_CMD*)packet, *(TYPE_PACKET_LENGTH *)(packet + SIZE_CMD));
	for (i = 0; i < size; i++)
	{
		printf("%2x ", *(unsigned char *)(packet + i));
	}
	printf("]\n");

	return 0;
}
int viewSend(int sockfd, char *packet, int size_packet)
{
	//printf("[server] [send sock(%d) size = %d] ", sockfd, size_packet);
	//return viewPacket(packet);

	return 0;
}
int viewRecv(int sockfd, char *packet)
{
	//printf("[server] [recv sock(%d)] ", sockfd);
	//return viewPacket(packet);

	return 0;
}
int viewCipher(char *cipher_text, int size_cipher_text, TYPE_SECRET_KEY *key)
{
	char plain_text[SIZE_BUFFER];

	int i;

	aesCtrEncryptBuffer((TYPE_SECRET_KEY *)plain_text, (TYPE_SECRET_KEY *)cipher_text, size_cipher_text, key, g_nonce);

	printLog("size = %d", size_cipher_text);

	printf(" => [ ");
	for (i = 0; i < size_cipher_text; i++)
	{
		printf("%x ", plain_text[i]);
	}
	printf("]\n");
	return 0;
}

int sendPacket(socket_info *p_socket_info, char *packet, int size_packet)
{
	int send_cnt = 0, retval;

	while (send_cnt < size_packet)
	{
		retval = send(p_socket_info->sockfd, packet + send_cnt, size_packet - send_cnt, 0);
		if (retval <= 0)
		{
			printLog("send() sockfd = %d, send_cnt = %d, retval = %d, size_packet = %d", p_socket_info->sockfd, send_cnt, retval, size_packet);
			return -1;
		}
		send_cnt += retval;
	}
	viewSend(p_socket_info->sockfd, packet, size_packet);
	return 0;
}

int encryptBuffer(TYPE_ROOM_NUMBER room_number, char *buffer, int length_buffer, char *buffer_cipher)
{
	//AES_CBC_encrypt_buffer(type_Secret_key* output, type_Secret_key* input, uint32_t length, const type_Secret_key* key, const type_Secret_key* iv, int keySize);
	int retval = 0;

	retval = AES_CBC_encrypt_buffer(
		// output
		(TYPE_SECRET_KEY *)buffer_cipher,
		// input
		(TYPE_SECRET_KEY *)buffer,
		// length_buffer
		length_buffer,
		// key
		g_arr_room_info[room_number].key,
		// iv
		g_iv, 128);
	/*
	aesCtrEncryptBuffer(
	// output
	(TYPE_SECRET_KEY *)buffer_cipher,
	// input
	(TYPE_SECRET_KEY *)buffer,
	// length_buffer
	length_buffer,
	// key
	g_arr_room_info[room_number].key,
	// nonce
	g_nonce);*/

	return retval;
}
int decryptBuffer(TYPE_ROOM_NUMBER room_number, char *buffer_cipher, int length_buffer_cipher, char *buffer_ret)
{
	//AES_CBC_encrypt_buffer(type_Secret_key* output, type_Secret_key* input, uint32_t length, const type_Secret_key* key, const type_Secret_key* iv, int keySize);
	int retval = 0;

	retval = AES_CBC_decrypt_buffer(
		// output
		(TYPE_SECRET_KEY *)buffer_ret,
		// input
		(TYPE_SECRET_KEY *)buffer_cipher,
		// length_buffer
		length_buffer_cipher,
		// key
		g_arr_room_info[room_number].key,
		// iv, keysize
		g_iv, 128);
	/*
	aesCtrEncryptBuffer(
	// output
	(TYPE_SECRET_KEY *)buffer_cipher,
	// input
	(TYPE_SECRET_KEY *)buffer,
	// length_buffer
	length_buffer,
	// key
	g_arr_room_info[room_number].key,
	// nonce
	g_nonce);*/

	return retval;
}
/*************** Function *************************************************/

int main(int argc, char **argv)
{
	int retval;

	if (argc == 2 && argv[1][0] == 'd')
	{
		if (initDaemon() != 0)
		{
			printLog("initDaemon()");
			return -1;
		}
	}

	dbOpen();

	//dbDropAllTable();
	dbCreateAllTable();

	retval = mainChattingServer();

	dbClose();

	return retval;
}

int packetProcessing(socket_info *p_socket_info, char *packet)
{
	int retval = 0;
	char *buffer = packet;
	TYPE_CMD cmd = *(TYPE_CMD*)buffer;
	TYPE_PACKET_LENGTH size_packet = *(TYPE_PACKET_LENGTH *)(buffer + SIZE_CMD);


	buffer[size_packet] = 0;

	viewRecv(p_socket_info->sockfd, packet);
	switch (cmd)
	{
	case CMD_CREATE_ID:
		printLog("[server] [create id] [id = %s]", buffer + SIZE_HEADER);
		retval = cmdCreateId(p_socket_info);
		break;

	case CMD_CREATE_ROOM:
		printLog("[server] [create room] [id = %s][%c][subject = %s]", buffer + SIZE_HEADER, *(buffer + SIZE_HEADER + SIZE_ID), buffer + SIZE_HEADER + SIZE_ID + SIZE_ROOM_STATUS);
		retval = cmdCreateRoom(p_socket_info);
		break;

	case CMD_ENTER_ROOM:
		printLog("[server] [enter room] [id = %s]", buffer + SIZE_HEADER);
		retval = cmdEnterRoom(p_socket_info);
		break;

	case CMD_LEAVE_ROOM:
		printLog("[server] [leave room] [id = %s]", buffer + SIZE_HEADER);
		retval = cmdLeaveRoom(p_socket_info);
		break;

	case CMD_VIEW_ROOM:
		printLog("[server] [view room] [id = %s]", buffer + SIZE_HEADER);
		retval = cmdViewRoom(p_socket_info);
		break;

	case CMD_TOTAL_ROOM_LIST:
		printLog("[server] [total room list] [id = %s]", buffer + SIZE_HEADER);
		retval = cmdTotalRoomList(p_socket_info);
		break;

	case CMD_MY_ROOM_LIST:
		printLog("[server] [my room list] [id = %s]", buffer + SIZE_HEADER);
		retval = cmdMyRoomList(p_socket_info);
		break;

	case CMD_CHATTING_MESSAGE:
		printLog("[server] [message] [id = %s] [%d]", buffer + SIZE_HEADER, size_packet - (SIZE_HEADER + SIZE_ID + SIZE_ROOM_NUMBER));
		retval = cmdChattingMessage(p_socket_info);
		break;

	default:
		printLog("[server] [not define command]");
		retval = -1;
		break;
	}

	if (retval != 0)
		printLog("in packetProcessing()");

	return retval;
}
int recvProcessing(int epollfd, socket_info *p_socket_info)
{
	int retval;

	int sockfd = p_socket_info->sockfd;
	char *buffer = p_socket_info->buffer;
	char packet[SIZE_BUFFER];

	TYPE_PACKET_LENGTH size_packet;

	retval = recv(sockfd, buffer + p_socket_info->cnt_read, SIZE_BUFFER - p_socket_info->cnt_read - 1, 0);
	if (retval <= 0)
	{
		printLog("recv()");

		if (delSocketEpoll(epollfd, p_socket_info) != 0)
		{
			printLog("delSocketEpoll()");
			return -1;
		}
		printLog("close sock(%d)", sockfd);
		return -1;
	}
	p_socket_info->cnt_read += retval;

	if (p_socket_info->cnt_read < SIZE_HEADER)
		return 1;

	size_packet = *(TYPE_PACKET_LENGTH *)(buffer + SIZE_CMD);
	if (size_packet > p_socket_info->cnt_read)
		return 1;

	memcpy(packet, buffer, size_packet);
	if (packetProcessing(p_socket_info, packet) != 0)
	{
		printLog("packetProcessing()");
		return -1;
	}

	p_socket_info->cnt_read -= size_packet;
	if (p_socket_info->cnt_read > 0)
		memcpy(buffer, buffer + size_packet, p_socket_info->cnt_read);
	return 0;
}
int mainChattingServer()
{
	int listenfd;
	int epollfd;
	struct epoll_event arr_return_event[MAX_CLIENT];

	int sockfd;
	int i;
	int cnt_return_event;

	socket_info *p_socket_info;


	if (initListenfd(&listenfd, PORT) != 0)
	{
		printLog("initListenfd()");
		return -1;
	}

	if (initEpoll(&epollfd) != 0)
	{
		printLog("initEpoll()");
		return -1;
	}

	if (initRoomInfoArr() != 0)
	{
		printLog("initRoomInfoArr()");
		return -1;
	}

	if (addSocketEpoll(epollfd, listenfd) != 0)
	{
		printLog("addSocketEpoll()");
		return -1;
	}

	while (1)
	{
		cnt_return_event = epoll_wait(epollfd, arr_return_event, MAX_CLIENT, -1);
		if (cnt_return_event < 0)
		{
			printLog("epoll_wait()");
			return -1;
		}

		for (i = 0; i < cnt_return_event; i++)
		{
			p_socket_info = (socket_info*)arr_return_event[i].data.ptr;
			if (p_socket_info->sockfd == listenfd)
			{
				sockfd = accept(listenfd, NULL, NULL);
				if (sockfd <= 0)
				{
					printLog("accept()");
					return -1;
				}
				if (addSocketEpoll(epollfd, sockfd) != 0)
				{
					printLog("addSocketEpoll()");
					return -1;
				}
				printLog("accept sock(%d)", sockfd);
			}
			else {
				if (recvProcessing(epollfd, p_socket_info) != 0)
				{
					printLog("recvProcessing()");
				}
			}
		}
	}

	return 0;
}

int reuse(int sockfd)
{
	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
		printLog("setsockopt(SO_REUSEADDR)");
		return -1;
	}
	return 0;
}

int initListenfd(int *listenfd, int port)
{
	struct sockaddr_in listen_addr;

	if (((*listenfd) = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printLog("socket()");
		return -1;
	}
	if (reuse((*listenfd)) != 0)
	{
		printLog("reuse()");
		return -1;
	}

	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = htons(port);

	if (bind((*listenfd), (struct sockaddr *) &listen_addr, sizeof(listen_addr)) < 0)
	{
		printLog("bind()");
		return -1;
	}

	if (listen((*listenfd), SOMAXCONN) != 0)
	{
		printLog("listen()");
		return -1;
	}

	return 0;
}
int initEpoll(int *epollfd)
{
	(*epollfd) = epoll_create(MAX_CLIENT);
	if (!(*epollfd))
	{
		printLog("epoll_create()");
		return -1;
	}

	return 0;
}
int addSocketEpoll(int epollfd, int sockfd)
{
	struct epoll_event tmp_event;

	socket_info *p_socket_info = (socket_info*)malloc(sizeof(socket_info));
	if (p_socket_info == NULL)
	{
		printLog("malloc()");
		return -1;
	}
	p_socket_info->sockfd = sockfd;
	p_socket_info->cnt_read = 0;
	memset(p_socket_info->id, 0, SIZE_ID);

	tmp_event.events = EPOLLIN | EPOLLHUP;
	tmp_event.data.ptr = (void*)p_socket_info;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &tmp_event) < 0)
	{
		printLog("epoll_ctl(), adding sockfd");
		return -1;
	}

	if (addSocketInfoArr(p_socket_info) != 0)
	{
		printLog("addSocketInfoArr()");
		return -1;
	}

	return 0;
}

int delSocketEpoll(int epollfd, socket_info *p_socket_info)
{
	if (epoll_ctl(epollfd, EPOLL_CTL_DEL, p_socket_info->sockfd, NULL) != 0)
	{
		printLog("epoll_ctl(), deleting sockfd");
		return -1;
	}

	if (close(p_socket_info->sockfd) != 0)
	{
		printLog("close()");
		return -1;
	}

	free(p_socket_info);

	if (delSocketInfoArr(p_socket_info) != 0)
	{
		printLog("delSocketInfoArr()");
		return -1;
	}

	if (delListSocketAllRoom(p_socket_info) != 0)
	{
		printLog("delListSocketAllRoom()");
		return -1;
	}

	return 0;
}

int addSocketInfoArr(socket_info *p_socket_info)
{
	if (g_cnt_socket >= MAX_CLIENT)
	{
		printLog("socket max");
		return -1;
	}

	g_arr_p_socket_info[g_cnt_socket++] = p_socket_info;
	return 0;
}

int delSocketInfoArr(socket_info *p_socket_info)
{
	int i;
	for (i = 0; i < g_cnt_socket; i++)
	{
		if (g_arr_p_socket_info[i] == p_socket_info)
			break;
	}
	if (i >= g_cnt_socket)
	{
		printLog("socket max");
		return -1;
	}

	g_cnt_socket--;
	for (; i < g_cnt_socket; i++)
	{
		g_arr_p_socket_info[i] = g_arr_p_socket_info[i + 1];
	}

	return 0;
}

int findSocketInfoArr(const char *id, socket_info **p_socket_info_ret)
{
	int i;
	for (i = 0; i < g_cnt_socket; i++)
	{
		if (memcmp(id, g_arr_p_socket_info[i]->id, SIZE_ID) == 0)
		{
			*p_socket_info_ret = g_arr_p_socket_info[i];
			return 0;
		}
	}
	printLog("not found socket");
	return -1;
}

int initRoomInfoArr()
{
	int i;
	for (i = 0; i < MAX_ROOM; i++)
	{
		g_arr_room_info[i].status = ROOM_INFO_STATUS_NOT_EXIST;
		g_arr_room_info[i].p_list_socket = NULL;
	}

	return 0;
}


TYPE_ROOM_NUMBER addRoomInfoArr(socket_info *p_socket_info, char is_secret, char *subject)
{
	TYPE_ROOM_NUMBER room_number;
	for (room_number = 0; room_number < MAX_ROOM; room_number++)
	{
		if (g_arr_room_info[room_number].status == ROOM_INFO_STATUS_NOT_EXIST)
			break;
	}

	if (room_number >= MAX_ROOM)
	{
		printLog("room max");
		return -1;
	}

	g_arr_room_info[room_number].status = is_secret;
	memcpy(g_arr_room_info[room_number].subject, subject, SIZE_ROOM_SUBJECT);
	g_arr_room_info[room_number].index_chatting_log = g_cnt_chatting_log++;

	if (is_secret == ROOM_INFO_STATUS_SECRET)
	{
		makeKey(g_arr_room_info[room_number].key);
	}
	else
	{
		memset(g_arr_room_info[room_number].key, 0, SIZE_SECRET_KEY);
	}

	return room_number;
}

int delRoomInfoArr(TYPE_ROOM_NUMBER room_number)
{
	if (room_number >= MAX_ROOM || room_number < 0)
	{
		printLog("invalid room number");
		return -1;
	}

	if (g_arr_room_info[room_number].status == ROOM_INFO_STATUS_NOT_EXIST)
	{
		printLog("Already Not Existed Room Number");
		return 0;
	}

	g_arr_room_info[room_number].status = ROOM_INFO_STATUS_NOT_EXIST;

	if (delAllListSocketInRoom(room_number) != 0)
	{
		printLog("delAllListSocketInRoom()");
		return -1;
	}
	return 0;
}

int findRoomInfoArr(TYPE_ROOM_NUMBER room_number)
{
	int idx;

	for (idx = 0; idx < g_cnt_room && idx < MAX_ROOM; idx++)
	{
		if (g_arr_room_info[idx].room_number == room_number)
			return idx;
	}

	return -1;
}

int delAllListSocketInRoom(TYPE_ROOM_NUMBER room_number)
{
	list_socket *prev_list_socket = g_arr_room_info[room_number].p_list_socket;
	list_socket *cur_list_socket = NULL;

	if (prev_list_socket == NULL)
		return 0;

	cur_list_socket = prev_list_socket->next;
	while (cur_list_socket != NULL)
	{
		if (prev_list_socket != NULL)
			free(prev_list_socket);

		prev_list_socket = cur_list_socket;
		cur_list_socket = cur_list_socket->next;
	}

	if (prev_list_socket != NULL)
		free(prev_list_socket);

	g_arr_room_info[room_number].p_list_socket = NULL;
	return 0;
}
int addListSocketInRoom(int room_number, socket_info *p_socket_info)
{
	list_socket *cur_list_socket, *prev_list_socket;
	list_socket *p_list_socket = (list_socket*)malloc(sizeof(list_socket));
	if (p_list_socket == NULL)
	{
		printLog("malloc");
		return -1;
	}
	p_list_socket->p_socket_info = p_socket_info;
	p_list_socket->next = NULL;


	if (g_arr_room_info[room_number].status == ROOM_INFO_STATUS_NOT_EXIST)
		return -1;

	prev_list_socket = NULL;
	cur_list_socket = g_arr_room_info[room_number].p_list_socket;
	while (cur_list_socket != NULL)
	{
		prev_list_socket = cur_list_socket;
		cur_list_socket = cur_list_socket->next;
	}

	if (prev_list_socket == NULL)
		g_arr_room_info[room_number].p_list_socket = p_list_socket;
	else
		prev_list_socket->next = p_list_socket;

	return 0;
}
int delListSocketInRoom(TYPE_ROOM_NUMBER room_number, socket_info *p_socket_info)
{
	list_socket *prev_list_socket = NULL;
	list_socket *cur_list_socket = g_arr_room_info[room_number].p_list_socket;


	while (cur_list_socket != NULL)
	{
		if (memcmp(cur_list_socket->p_socket_info->id, p_socket_info->id, SIZE_ID) == 0)
		{
			if (prev_list_socket != NULL)
				prev_list_socket->next = cur_list_socket->next;
			else
				g_arr_room_info[room_number].p_list_socket = cur_list_socket->next;

			break;
		}
		prev_list_socket = cur_list_socket;
		cur_list_socket = cur_list_socket->next;
	}

	if (cur_list_socket != NULL)
	{
		free(cur_list_socket);
	}
	else
	{
		printLog("Not Exist ListSocket To Delete");
		return 0;
	}

	if (g_arr_room_info[room_number].p_list_socket == NULL)
	{
		if (delRoomInfoArr(room_number) != 0)
		{
			printLog("delRoomInfoArr()");
			return -1;
		}
	}
	return 0;
}
int delListSocketAllRoom(socket_info *p_socket_info)
{
	int i;
	for (i = 0; i < MAX_ROOM; i++)
	{
		if (g_arr_room_info[i].status != ROOM_INFO_STATUS_NOT_EXIST)
		{
			delListSocketInRoom(i, p_socket_info);
		}
	}

	return 0;
}

int findListSocketInRoom(TYPE_ROOM_NUMBER room_number, socket_info *p_socket_info)
{
	list_socket *cur_list_socket = g_arr_room_info[room_number].p_list_socket;

	while (cur_list_socket != NULL)
	{
		if (memcmp(cur_list_socket->p_socket_info->id, p_socket_info->id, SIZE_ID) == 0)
		{
			return 0;
		}
		cur_list_socket = cur_list_socket->next;
	}
	return 1;
}
int makeMessage(char *id, TYPE_ROOM_NUMBER room_number, char *message, int size_message, char *message_ret)
{
	int retval;
	time_t timer;
	struct tm *t;

	char buffer_plain[SIZE_BUFFER];
	char *p_buffer = message_ret;

	timer = time(NULL);
	t = localtime(&timer);

	if (g_arr_room_info[room_number].status == ROOM_INFO_STATUS_SECRET)
		p_buffer = buffer_plain;

	// make message template
	retval = sprintf(p_buffer, "[%4d_%2d_%2d %2d:%2d:%2d] %s : ", t->tm_year + 1900, t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, id);
	memcpy(p_buffer + retval, message, size_message);
	retval += size_message;
	p_buffer[retval++] = '\n';
	p_buffer[retval] = '\0';

	// check encrypt & encrypt
	if (g_arr_room_info[room_number].status == ROOM_INFO_STATUS_SECRET)
		retval = encryptBuffer(room_number, p_buffer, retval, message_ret);

	return retval;
}

int resetFile(TYPE_ROOM_NUMBER room_number)
{
	FILE *fp;
	char file_name[256];
	char file_path[256];

	sprintf(file_name, "log/%d_%d_%c.txt", g_arr_room_info[room_number].index_chatting_log, room_number, g_arr_room_info[room_number].status);

	realpath(file_name, file_path);

	fp = fopen(file_path, "w");
	if (fp == NULL)
	{
		printLog("fopen");
		return -1;
	}

	if (fclose(fp) != 0)
	{
		printLog("fclose");
		return -1;
	}

	return 0;
}
int writeChattingLog(TYPE_ROOM_NUMBER room_number, char *id, char *message, size_t size_message)
{
	FILE *fp;
	char file_name[256];
	char file_path[256];

	sprintf(file_name, "log/%d_%d_%c.txt", g_arr_room_info[room_number].index_chatting_log, room_number, g_arr_room_info[room_number].status);

	realpath(file_name, file_path);

	fp = fopen(file_path, "a");
	if (fp == NULL)
	{
		printLog("fopen");
		return -1;
	}

	fwrite(message, 1, size_message, fp);

	if (fclose(fp) != 0)
	{
		printLog("fclose");
		return -1;
	}

	return 0;
}
size_t readChattingLog(TYPE_ROOM_NUMBER room_number, char *id, char *buffer_ret, size_t size_buffer_ret)
{
	FILE *fp;
	char file_name[256];
	char file_path[256];

	size_t retval;

	sprintf(file_name, "log/%d_%d_%c.txt", g_arr_room_info[room_number].index_chatting_log, room_number, g_arr_room_info[room_number].status);

	realpath(file_name, file_path);

	fp = fopen(file_path, "r");
	if (fp == NULL)
	{
		printLog("fopen");
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	if (ftell(fp) > SIZE_CHATTING_LOG)
		fseek(fp, -SIZE_CHATTING_LOG, SEEK_END);

	retval = fread(buffer_ret, 1, size_buffer_ret, fp);
	//printLog("[message log read] [%s] [%s]", file_path, buffer_ret);

	if (fclose(fp) != 0)
	{
		printLog("fclose");
		return -1;
	}

	return retval;
}


int sprideChattingMessage(TYPE_ROOM_NUMBER room_number, char *id, char *message, int size_message)
{
	char packet[SIZE_BUFFER];
	char remake_message[SIZE_BUFFER];
	int size_remake_message;
	TYPE_CMD cmd;
	TYPE_PACKET_LENGTH length;

	list_socket *cur_list_socket;

	// make packet
	// cmd
	cmd = CMD_CHATTING_MESSAGE;
	memcpy(packet, &cmd, SIZE_CMD);
	length = SIZE_HEADER;

	// id
	memcpy(packet + length, id, SIZE_ID);
	length += SIZE_ID;

	// room number
	memcpy(packet + length, &room_number, SIZE_ROOM_NUMBER);
	length += SIZE_ROOM_NUMBER;

	// message
	size_remake_message = makeMessage(id, room_number, message, size_message, remake_message);
	memcpy(packet + length, remake_message, size_remake_message);
	length += size_remake_message;

	// length
	memcpy(packet + SIZE_CMD, &length, SIZE_PACKET_LENGTH);

	// send packet
	cur_list_socket = g_arr_room_info[room_number].p_list_socket;
	while (cur_list_socket != NULL)
	{
		if (sendPacket(cur_list_socket->p_socket_info, packet, *(TYPE_PACKET_LENGTH *)(packet + SIZE_CMD)) != 0)
		{
			printLog("sendPacket()");
			return -1;
		}
		cur_list_socket = cur_list_socket->next;
	}

	if (writeChattingLog(room_number, packet + SIZE_HEADER, remake_message, size_remake_message) != 0)
	{
		printLog("writeChattingLog()");
		return -1;
	}
	return 0;
}
/*****************************************************/
/*********************** cmd Function *****************************/
int cmdCreateId(socket_info *p_socket_info)
{
	char *buffer = p_socket_info->buffer;
	char *id = buffer + SIZE_HEADER;

	if (dbInsertUserinfo(id) != 0)
	{
		printLog("requestDB()");
		return -2;
	}

	memcpy(p_socket_info->id, id, SIZE_ID);

	// send packet
	if (sendPacket(p_socket_info, buffer, *(TYPE_PACKET_LENGTH *)(buffer + SIZE_CMD)) != 0)
	{
		printLog("sendPacket()");
		return -1;
	}
	return 0;
}

int cmdCreateRoom(socket_info *p_socket_info)
{
	char buffer_send[SIZE_BUFFER];
	int size_send_packet = 0;
	char *buffer = p_socket_info->buffer;

	char *id = buffer + SIZE_HEADER;
	char is_secret = *(id + SIZE_ID);
	char *room_subject = id + SIZE_ID + SIZE_ROOM_STATUS;

	TYPE_ROOM_NUMBER room_number;
	int retval;

	if (resetFile(room_number) != 0)
	{
		printLog("resetFile()");
		return -1;
	}

	room_number = addRoomInfoArr(p_socket_info, is_secret, room_subject);
	if (room_number < 0)
	{
		printLog("addRoomInfoArr()");
		return -1;
	}

	if (is_secret == ROOM_INFO_STATUS_SECRET)
	{
		;
	}

	if (dbInsertRoominfo(room_number, room_subject, g_arr_room_info[room_number].key) != 0)
	{
		printLog("requestDB()");
		return -2;
	}
	if (dbInsertRoomUser(room_number, id) != 0)
	{
		printLog("requestDB()");
		return -2;
	}

	if (addListSocketInRoom(room_number, p_socket_info))
	{
		printLog("addListSocketInRoom()");
		return -1;
	}

	// S->C # CMD[2] : LENGTH[4] : ID or CMD_FAIL : ROOM_NUMBER : STATUS : SUBJECT : SECRETKEY
	// make packet
	// cmd, id
	memcpy(buffer_send, buffer, SIZE_HEADER + SIZE_ID);
	size_send_packet += SIZE_HEADER + SIZE_ID;

	// room number
	memcpy(buffer_send + size_send_packet, &room_number, SIZE_ROOM_NUMBER);
	size_send_packet += SIZE_ROOM_NUMBER;

	// status
	memcpy(buffer_send + size_send_packet, &g_arr_room_info[room_number].status, SIZE_ROOM_STATUS);
	size_send_packet += SIZE_ROOM_STATUS;

	// subject
	memcpy(buffer_send + size_send_packet, g_arr_room_info[room_number].subject, SIZE_ROOM_SUBJECT);
	size_send_packet += SIZE_ROOM_SUBJECT;

	// secretKey
	memcpy(buffer_send + size_send_packet, g_arr_room_info[room_number].key, SIZE_SECRET_KEY);
	size_send_packet += SIZE_SECRET_KEY;

	// length
	memcpy(buffer_send + SIZE_CMD, &size_send_packet, SIZE_PACKET_LENGTH);

	// send packet
	if (sendPacket(p_socket_info, buffer_send, size_send_packet) != 0)
	{
		printLog("sendPacket()");
		return -1;
	}


	// send message packet
	retval = sprintf(buffer_send, "[%s is joined]", buffer + SIZE_HEADER);
	if (sprideChattingMessage(room_number, buffer + SIZE_HEADER, buffer_send, retval) != 0)
	{
		printLog("sprideChattingMessage()");
		return -1;
	}

	return 0;
}

int cmdEnterRoom(socket_info *p_socket_info)
{
	char *buffer = p_socket_info->buffer;
	char *id = buffer + SIZE_HEADER;
	TYPE_ROOM_NUMBER room_number = *(TYPE_ROOM_NUMBER *)(buffer + SIZE_HEADER + SIZE_ID);

	char buffer_send[SIZE_BUFFER];
	int cnt_send;
	int retval;

	if (dbInsertRoomUser(room_number, id) != 0)
	{
		printLog("requestDB()");
		return -2;
	}

	if (addListSocketInRoom(room_number, p_socket_info))
	{
		printLog("addListSocketInRoom()");
		return -1;
	}

	// make packet EnterRoom
	// CMD : ID : ROOM_NUMBER 
	cnt_send = SIZE_HEADER + SIZE_ID + SIZE_ROOM_NUMBER;
	memcpy(buffer_send, buffer, cnt_send);

	// status
	buffer_send[cnt_send] = g_arr_room_info[room_number].status;
	cnt_send++;

	// subject
	memcpy(buffer_send + cnt_send, g_arr_room_info[room_number].subject, SIZE_ROOM_SUBJECT);
	cnt_send += SIZE_ROOM_SUBJECT;

	// secretKey
	memcpy(buffer_send + cnt_send, g_arr_room_info[room_number].key, SIZE_SECRET_KEY);
	cnt_send += SIZE_SECRET_KEY;

	// length
	memcpy(buffer_send + SIZE_CMD, &cnt_send, SIZE_PACKET_LENGTH);

	// send packet EnterRoom
	if (sendPacket(p_socket_info, buffer_send, cnt_send) != 0)
	{
		printLog("sendPacket()");
		return -1;
	}

	// send message packet
	retval = sprintf(buffer_send, "[%s is joined]", buffer + SIZE_HEADER);
	if (sprideChattingMessage(room_number, buffer + SIZE_HEADER, buffer_send, retval) != 0)
	{
		printLog("sprideChattingMessage()");
		return -1;
	}
	return 0;
}

int cmdLeaveRoom(socket_info *p_socket_info)
{
	char *buffer = p_socket_info->buffer;
	char *id = buffer + SIZE_HEADER;
	TYPE_ROOM_NUMBER room_number = *(TYPE_ROOM_NUMBER *)(buffer + SIZE_HEADER + SIZE_ID);

	char buffer_send[SIZE_BUFFER];
	int retval;

	if (dbDeleteRoomUser(room_number, id) != 0)
	{
		printLog("requestDB()");
		return -2;
	}

	if (delListSocketInRoom(room_number, p_socket_info) != 0)
	{
		printLog("delListSocketInRoom()");
		return -1;
	}

	// send packet
	if (sendPacket(p_socket_info, buffer, *(TYPE_PACKET_LENGTH *)(buffer + SIZE_CMD)) != 0)
	{
		printLog("sendPacket()");
		return -1;
	}


	if (g_arr_room_info[room_number].p_list_socket == NULL)
	{
		if (dbDeleteRoominfo(room_number) != 0)
		{
			printLog("requestDB()");
			return -2;
		}
		if (delRoomInfoArr(room_number) != 0)
		{
			printLog("delRoomInfoArr()");
			return -1;
		}
		return 0;
	}

	// send message packet
	retval = sprintf(buffer_send, "[%s is leaved]", buffer + SIZE_HEADER);
	if (sprideChattingMessage(room_number, buffer + SIZE_HEADER, buffer_send, retval) != 0)
	{
		printLog("sprideChattingMessage()");
		return -1;
	}
	return 0;
}

int cmdViewRoom(socket_info *p_socket_info)
{
	char *buffer = p_socket_info->buffer;
	char *id = buffer + SIZE_HEADER;
	TYPE_ROOM_NUMBER room_number = *(TYPE_ROOM_NUMBER *)(buffer + SIZE_HEADER + SIZE_ID);

	char buffer_send[SIZE_BUFFER];

	int size_send_packet;
	int retval;

	// make packet
	// cmd, id, room_number
	size_send_packet = SIZE_HEADER + SIZE_ID + SIZE_ROOM_NUMBER;
	memcpy(buffer_send, buffer, size_send_packet);

	// chatting log
	if ((retval = readChattingLog(room_number, id, buffer_send + size_send_packet, (size_t)(SIZE_BUFFER - size_send_packet))) < 0)
	{
		printLog("readChattingLog()");
		return -1;
	}
	size_send_packet += retval;

	// length
	memcpy(buffer_send + SIZE_CMD, &size_send_packet, SIZE_PACKET_LENGTH);

	// send packet
	if (sendPacket(p_socket_info, buffer_send, size_send_packet) != 0)
	{
		printLog("sendPacket()");
		return -1;
	}

	return 0;
}

int cmdTotalRoomList(socket_info *p_socket_info)
{
	char *buffer = p_socket_info->buffer;

	char buffer_send[SIZE_BUFFER];
	int size_send_packet = 0;

	TYPE_ROOM_NUMBER i;

	if (dbSelectAllRoom(g_arr_room_info, MAX_ROOM) != 0)
	{
		printLog("requestDB()");
		return -2;
	}
	// make packet
	// cmd, id
	memcpy(buffer_send, buffer, SIZE_HEADER + SIZE_ID);
	size_send_packet += SIZE_HEADER + SIZE_ID;

	// roomlist
	for (i = 0; i < MAX_ROOM; i++)
	{
		if (g_arr_room_info[i].status == ROOM_INFO_STATUS_NOT_EXIST)
			continue;
		// room number
		memcpy(buffer_send + size_send_packet, &i, SIZE_ROOM_NUMBER);
		size_send_packet += SIZE_ROOM_NUMBER;

		// room status
		memcpy(buffer_send + size_send_packet, &(g_arr_room_info[i].status), SIZE_ROOM_STATUS);
		size_send_packet += SIZE_ROOM_STATUS;

		// room subject
		memcpy(buffer_send + size_send_packet, g_arr_room_info[i].subject, SIZE_ROOM_SUBJECT);
		size_send_packet += SIZE_ROOM_SUBJECT;
	}

	// length
	memcpy(buffer_send + SIZE_CMD, &size_send_packet, SIZE_PACKET_LENGTH);

	// send packet
	if (sendPacket(p_socket_info, buffer_send, size_send_packet) != 0)
	{
		printLog("sendPacket()");
		return -1;
	}

	return 0;
}

int cmdMyRoomList(socket_info *p_socket_info)
{
	char *buffer = p_socket_info->buffer;

	char buffer_send[SIZE_BUFFER];
	int size_send_packet = 0;

	TYPE_ROOM_NUMBER i;

	// make packet
	// cmd, id
	memcpy(buffer_send, buffer, SIZE_HEADER + SIZE_ID);
	size_send_packet += SIZE_HEADER + SIZE_ID;

	// roomlist
	for (i = 0; i < MAX_ROOM; i++)
	{
		if (g_arr_room_info[i].status == ROOM_INFO_STATUS_NOT_EXIST
			|| findListSocketInRoom(i, p_socket_info) != 0)
			continue;
		// room number
		memcpy(buffer_send + size_send_packet, &i, SIZE_ROOM_NUMBER);
		size_send_packet += SIZE_ROOM_NUMBER;

		// status
		memcpy(buffer_send + size_send_packet, &(g_arr_room_info[i].status), SIZE_ROOM_STATUS);
		size_send_packet += SIZE_ROOM_STATUS;

		// subject
		memcpy(buffer_send + size_send_packet, g_arr_room_info[i].subject, SIZE_ROOM_SUBJECT);
		size_send_packet += SIZE_ROOM_SUBJECT;

		// secret key
		memcpy(buffer_send + size_send_packet, g_arr_room_info[i].key, SIZE_SECRET_KEY);
		size_send_packet += SIZE_SECRET_KEY;
	}
	// length
	memcpy(buffer_send + SIZE_CMD, &size_send_packet, SIZE_PACKET_LENGTH);

	// send packet
	if (sendPacket(p_socket_info, buffer_send, size_send_packet) != 0)
	{
		printLog("sendPacket()");
		return -1;
	}

	return 0;
}

int cmdChattingMessage(socket_info *p_socket_info)
{
	char *buffer = p_socket_info->buffer;
	TYPE_PACKET_LENGTH size_packet = *(TYPE_PACKET_LENGTH *)(buffer + SIZE_CMD);
	TYPE_ROOM_NUMBER room_number = *(TYPE_ROOM_NUMBER *)(buffer + SIZE_HEADER + SIZE_ID);
	char *id = buffer + SIZE_HEADER;
	char *message = buffer + (SIZE_HEADER + SIZE_ID + SIZE_ROOM_NUMBER);
	int size_message = size_packet - (SIZE_HEADER + SIZE_ID + SIZE_ROOM_NUMBER);

	char buffer_plain[SIZE_BUFFER];
	char *message_send = message;
	int size_message_send = size_message;

	if (g_arr_room_info[room_number].status == ROOM_INFO_STATUS_SECRET)
	{
		size_message_send = decryptBuffer(room_number, message, size_message, buffer_plain);
		message_send = buffer_plain;
	}


	// send message packet
	if (sprideChattingMessage(
		// room_number
		room_number,
		// id
		id,
		// message
		message_send,
		// size_message
		size_message_send) != 0)
	{
		printLog("sprideChattingMessage()");
		return -1;
	}

	return 0;
}

/********************* end cmd Function ***************************/


//////////////////////////////////////
int initDaemon()
{
	pid_t pid;

	umask(0);

	if (daemon(1, 1) == -1) {
		printLog("daemon()");
		return -1;
	}

	if ((pid = fork()) == -1) {
		printLog("fork()");
		return -1;
	}
	else if (pid > 0) {           /* parent */
		_exit(0);
	}
	/* child */

	stdin = freopen("/dev/null", "r", stdin);
	stdout = freopen("/dev/null", "w", stdout);
	stderr = freopen("/dev/null", "w", stderr);

	return 0;
}
/*************** END OF FILE **********************************************/





