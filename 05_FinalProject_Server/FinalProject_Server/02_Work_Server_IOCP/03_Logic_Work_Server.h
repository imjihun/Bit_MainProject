#pragma once

#pragma warning(disable:4996)

#include <WinSock2.h>
#include <list>

//#include "02_IOCP_Model.h"
#include "../DebugLog.h"
#include "05_Protocol.h"

#include <map>
#include <process.h>
#include <string>
using namespace std;

class Vector3
{
public:
	float X, Y, Z;
	Vector3()
	{
		X = Y = Z = 0;
	}
	Vector3(float x, float y, float z)
	{
		X = x; Y = y; Z = z;
	}
	Vector3(char *ptr)
	{
		X = *(float *)(ptr);
		Y = *(float *)(ptr + 4);
		Z = *(float *)(ptr + 8);
	}
	void operator=(Vector3 p)
	{
		X = p.X; Y = p.Y; Z = p.Z;
	}
};
class AnimStatus
{
public:
	bool bSitting;
	bool bQuestioning;

	AnimStatus()
	{
		bSitting = false;
		bQuestioning = false;
	}
	AnimStatus(bool bsitting, bool bquestioning)
	{
		bSitting = bsitting;
		bQuestioning = bquestioning;
	}
	AnimStatus(char *ptr)
	{
		bSitting = *(bool *)(ptr);
		bQuestioning = *(bool *)(ptr + 1);
	}

	void operator=(AnimStatus s)
	{
		bSitting = s.bSitting;
		bQuestioning = s.bQuestioning;
	}
};
class UserData
{
public:
	SOCKETINFO *Ptr;
	char Id[16];
	int LenId;
	char Curkey;
	Vector3 Position;
	Vector3 Rotation;
	AnimStatus Status;
	int roomNum;

	UserData(SOCKETINFO *ptr, char *id, int lenid)
	{
		memcpy_s(Id, 16, id, lenid);
		Id[lenid] = 0;
		LenId = lenid;
		Ptr = ptr;
		Curkey = 0;
		roomNum = 0;
		Position.X = Position.Y = Position.Z = 0;
		Rotation.X = Rotation.Y = Rotation.Z = 0;
	}
};

void Init_WorkServer();
void Main_WorkServerLogic(char *threadBuf, HANDLE hcp, unsigned int cbTransferred, SOCKETINFO *ptr, int threadNum);
void DisConnectClient(HANDLE hcp, char *threadBuf, SOCKETINFO *ptr, int threadNum);
void PrintPacket(SOCKETINFO *ptr, char *packet, int len, char *str);