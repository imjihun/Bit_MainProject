using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;

namespace _04_Chatting_Client_01
{

	public class MyRoom
	{
		public int room_number;
		public byte status;
		public string subject;
		public byte[] secret_key = new byte[Macro.SIZE_SECRET_KEY];
		public StringBuilder log_chatting = new StringBuilder();
		public WindowChatting wnd = null;
		public Button btn = null;

		public MyRoom(int num, byte stat, string sub, byte[] key, string chat, Button b)
		{
			room_number = num;
			status = stat;
			subject = sub;
			if(key != null)
				Array.Copy(key, secret_key, Macro.SIZE_SECRET_KEY);
			btn = b;
			log_chatting.Append(chat);
		}
		public void addLogChatting(string id, string message)
		{
			log_chatting.Append("[" + id + "] " + message + "\n");
		}
		public void addLogChatting(string log)
		{
			log_chatting.Append(log);
		}
	}

	public class TotalRoom
	{
		int room_number;
		string subject;

		public TotalRoom(int r, string s)
		{
			room_number = r;
			subject = s;
		}
	}

	public class UserData
	{
		public static UserData ud = null;

		public Socket server;
		public string id;
		public Dictionary<int, MyRoom> dic_my_rooms = new Dictionary<int, MyRoom>();
		public Dictionary<int, TotalRoom> dic_total_rooms = new Dictionary<int, TotalRoom>();
		public UserData(string _id)
		{
			if (ud == null)
				ud = this;

			id = _id;
		}		
	}
}
