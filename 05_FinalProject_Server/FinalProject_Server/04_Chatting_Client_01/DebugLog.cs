using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace _04_Chatting_Client_01
{
	public class DebugLog
	{
		public static void debugLog(string str, byte[] buffer, int size)
		{
			Console.Write("[" + str + "] ");
			if (buffer != null)
			{
				Console.Write("[buffer [" + size + "] = ");
				for (int i = 0; i < size; i++)
					Console.Write(string.Format("{0:X2} ", buffer[i]));
				Console.WriteLine("]");
			}
		}
	}
}
