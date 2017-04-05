#include "01_Main_TCP_Server.h"
#include "02_ConnectDB.h"

int main(int argc, char** argv)
{
	printf("[DB Server (MySQL, Disk)]\n\n");

	if (_mkdir(PATH_SAVE_IMAGE) != 0 && errno == ENOENT)
		ViewError("Image Folder Create Failed - (Folder Path Not Correct)");

	connectDB();
	Main_TCP_Server(argc, argv);
	closeDB();
	return 0;
}