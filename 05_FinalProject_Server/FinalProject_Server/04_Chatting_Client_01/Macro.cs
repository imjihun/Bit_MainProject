using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace _04_Chatting_Client_01
{
	public static class Macro
	{
		#region CMD
		public const UInt16 CMD_CREATE_ID = 10;
		public const UInt16 CMD_CREATE_ROOM = 20;
		public const UInt16 CMD_ENTER_ROOM = 30;
		public const UInt16 CMD_LEAVE_ROOM = 40;
		public const UInt16 CMD_VIEW_ROOM = 50;
		public const UInt16 CMD_TOTAL_ROOM_LIST = 60;
		public const UInt16 CMD_MY_ROOM_LIST = 70;
		public const UInt16 CMD_CHATTING_MESSAGE = 80;
		public const UInt16 CMD_INVITE = 90;
		#endregion

		// packet[SIZE_HEADER] == CMD_FAIL
		public const UInt16 CMD_FAIL = 10000;

		public const int SIZE_CMD = 2;
		public const int SIZE_PACKET_LENGTH = 4;
		public const int SIZE_HEADER = SIZE_CMD + SIZE_PACKET_LENGTH;
		public const int SIZE_BUFFER = 4096;
		public const int SIZE_ID = 16;
		public const int SIZE_ROOM_SUBJECT = 64;

		public const int SIZE_SECRET_KEY = 16;
		public const int SIZE_CIPHER_ELEMENT = 16;

		public const byte ROOM_INFO_STATUS_SECRET = 0x53;
		public const byte ROOM_INFO_STATUS_NORMAL = 0x4E;
	}
}
