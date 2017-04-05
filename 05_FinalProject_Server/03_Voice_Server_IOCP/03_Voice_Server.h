#pragma once

#pragma warning(disable:4996)

#include <WinSock2.h>
#include <list>

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
class UserData
{
public:
	SOCKETINFO *Ptr;
	char Id[16];
	int LenId;
	char Curkey;
	Vector3 Position;
	Vector3 Rotation;

	UserData(SOCKETINFO *ptr, char *id, int lenid)
	{
		memcpy_s(Id, 16, id, lenid);
		Id[lenid] = 0;
		LenId = lenid;
		Ptr = ptr;
		Curkey = 0;

		Position.X = Position.Y = Position.Z = 0;
		Rotation.X = Rotation.Y = Rotation.Z = 0;
	}
};

void InitVoiceServer();
void DeleteVoiceUser(SOCKETINFO *ptr);
void Req_WSASend(SOCKETINFO *ptr, int threadNum);
void Main_VoiceServerLogic(char *threadBuf, HANDLE hcp, unsigned int cbTransferred, SOCKETINFO *ptr, int threadNum);
