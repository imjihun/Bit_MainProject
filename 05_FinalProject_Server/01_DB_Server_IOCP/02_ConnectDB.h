#pragma once

#include <mysql.h>

#include "../DebugLog.h"
#include "05_Protocol.h"

#pragma comment(lib, "libmysql.lib")

void connectDB();
void closeDB();
int MAIN_DB(char *Packet, unsigned short Len_Pack, char *SendBuf, unsigned short *Len_SendBuf);
int SaveProfileImage(char *Packet, long Len_Pack, char *SendBuf, unsigned short *Len_SendBuf);
int GetUserProfileImage(char *Packet, unsigned short Len_Pack, char *SendBuf, int *Len_SendBuf);