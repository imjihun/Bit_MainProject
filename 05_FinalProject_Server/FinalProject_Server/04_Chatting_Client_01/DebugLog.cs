using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace _04_Chatting_Client_01
{
	public class DebugLog
	{
		public static void viewChar(string str, byte[] buffer, int offset, int size)
		{
			if (buffer != null && size > 0)
			{
				Console.WriteLine(str);
				Console.Write(Encoding.UTF8.GetChars(buffer, offset, size));
			}
		}
		public static void debugLog(string str)
		{
			Console.WriteLine("\t[" + str + "] ");
		}
		public static void debugLog(string str, byte[] buffer, int size)
		{
			if (buffer != null && size > 0 
				&& BitConverter.ToUInt16(buffer, 0) != Macro.CMD_TOTAL_ROOM_LIST
				&& BitConverter.ToUInt16(buffer, 0) != Macro.CMD_ENTER_ROOM)
			{
				Console.WriteLine("\t[" + str + "] ");
				Console.Write("[buffer(" + size + ") = ");
				for (int i = 0; i < size; i++)
					Console.Write(string.Format("{0:X2} ", buffer[i]));
				Console.WriteLine("]");

				if(BitConverter.ToUInt16(buffer, 0) == Macro.CMD_CHATTING_MESSAGE)
					viewChar(" <string> ", buffer, Macro.SIZE_HEADER + Macro.SIZE_ID + 4, BitConverter.ToInt32(buffer, Macro.SIZE_CMD) - (Macro.SIZE_HEADER + Macro.SIZE_ID + 4));
				Console.WriteLine("\n");
				/*
				Console.Write("[header] [cmd(" + Macro.SIZE_CMD + ") = ");
				for (int i = 0; i < Macro.SIZE_CMD; i++)
					Console.Write(string.Format("{0:X2} ", buffer[i]));
				Console.Write("] ");
				Console.Write("[packet length(" + Macro.SIZE_PACKET_LENGTH + ") = ");
				for (int i = Macro.SIZE_CMD; i < Macro.SIZE_HEADER; i++)
					Console.Write(string.Format("{0:X2} ", buffer[i]));
				Console.WriteLine("]");
				*/
			}
		}
	}
}
