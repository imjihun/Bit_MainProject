#include "DebugLog.h"

void DebugMessage(char *buf)
{
#ifdef _DEBUG_LOGFILE_
	char stime[30];
	time_t timer;
	struct tm t;
	int err = 0;
	timer = time(NULL);
	if ((err = localtime_s(&t, &timer)) != 0)
		fprintf(stdout, "DebugLog : localtime_s() Error = %d\n", err);
	sprintf_s(stime, "%d%d%d_%d.log", t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour);
	FILE *fp = NULL;
	if((err = fopen_s(&fp, stime, "at")) != 0)
		fprintf(stdout, "DebugLog : fopen_s() Error = %d\n", err);
	fprintf(fp, "[%d-%d-%d_%d:%d:%d] : ", t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	fprintf(fp, buf);
	fprintf(fp, "\n");
	fclose(fp);
#endif
#ifdef _DEBUF_OUTPUT_
	OutputDebugStringA(buf);
#endif
}

void ViewMessage(char *format, ...)
{
	char buf[1024];
	va_list va;

	va_start(va, format);
	vsprintf_s(buf, format, va);
	va_end(va);

	puts(buf);
	DebugMessage(buf);
}

void ViewError(char *format, ...)
{
	LPSTR lpMsgBuf;

	ViewMessage(format);

	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPSTR)&lpMsgBuf, 0, NULL);

	//WideCharToMultiByte(CP_ACP, 0, (LPTSTR)lpMsgBuf, len, t, len, NULL, NULL);
	ViewMessage((LPSTR)lpMsgBuf);

	LocalFree(lpMsgBuf);
	exit(1);
}

void ViewError(int errorcode)
{
	LPSTR lpMsgBuf;

	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
				NULL, errorcode, 
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPSTR)&lpMsgBuf, 0, NULL);

	ViewMessage(lpMsgBuf);

	LocalFree(lpMsgBuf);
}