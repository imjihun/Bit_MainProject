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
		private string subject;
		private byte status;

		public int room_number;
		public byte Status
		{
			get { return status; }
			set { status = value; }
		}
		public string Subject
		{
			get { return subject; }
			set {
				subject = value;
				grd.Children.OfType<Button>().FirstOrDefault().Content = subject;
			}
		}
		public byte[] secret_key = new byte[Macro.SIZE_SECRET_KEY];
		public StringBuilder log_chatting = new StringBuilder();
		public Grid grd = null;

		public WindowChatting wnd = null;

		public MyRoom(int num, byte stat, string sub, byte[] key, string chat)
		{
			room_number = num;
			status = stat;
			subject = sub;
			if(key != null)
				Array.Copy(key, secret_key, Macro.SIZE_SECRET_KEY);
			log_chatting.Append(chat);
			grd = WindowRoomList.wnd.addGridMyList(
							room_number, status, subject, key, chat);
		}

		public void addLogChatting(string log)
		{
			log_chatting.Append(log);
		}
		public void setLogChatting(string log)
		{
			log_chatting.Clear();
			log_chatting.Append(log);
		}
	}

	public class TotalRoom
	{
		private string subject;
		private byte status;

		public int room_number;
		public byte Status
		{
			get { return status; }
			set
			{
				status = value;
				;
			}
		}

		public string Subject
		{
			get { return subject; }
			set {
				subject = value;
				btn.Content = subject;
			}
		}

		public Button btn = null;

		public TotalRoom(int r, byte stat, string s)
		{
			room_number = r;
			status = stat;
			subject = s;
			btn = WindowRoomList.wnd.addButtonTotalList(room_number, stat, subject);
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

		public int addMyRoom(int room_number, byte status, string subject, byte[] key, string chat)
		{
			if (!UserData.ud.dic_my_rooms.ContainsKey(room_number))
			{
				UserData.ud.dic_my_rooms[room_number] = new MyRoom(
								room_number, status, subject, key, chat);
				return 0;
			}
			else
			{
				UserData.ud.dic_my_rooms[room_number].Status = status;
				UserData.ud.dic_my_rooms[room_number].Subject = subject;
				Array.Copy(UserData.ud.dic_my_rooms[room_number].secret_key, key, Macro.SIZE_SECRET_KEY);
				UserData.ud.dic_my_rooms[room_number].setLogChatting(chat);
			}
			return -1;
		}
		public int delMyRoom(int room_number)
		{
			if (!UserData.ud.dic_my_rooms.ContainsKey(room_number))
				return -1;

			MyRoom my_room = UserData.ud.dic_my_rooms[room_number];

			if (my_room != null)
			{
				WindowRoomList.wnd.delButtonMyList(my_room.grd);
				UserData.ud.dic_my_rooms.Remove(room_number);
				return 0;
			}
			return -1;
		}
		public MyRoom findMyRoom(int room_number)
		{
			if (!UserData.ud.dic_my_rooms.ContainsKey(room_number))
				return null;
			
			return UserData.ud.dic_my_rooms[room_number];
		}

		public void clearTotalRoom()
		{
			foreach(var v in dic_total_rooms)
			{
				if (v.Value.btn != null)
				{
					WindowRoomList.wnd.stackPanel_totallist.Children.Remove(v.Value.btn);
					v.Value.btn = null;
				}
			}
			UserData.ud.dic_total_rooms.Clear();
		}
		public int addTotalRoom(int room_number, byte status, string subject)
		{
			if (!UserData.ud.dic_total_rooms.ContainsKey(room_number))
			{
				UserData.ud.dic_total_rooms[room_number] = new TotalRoom(room_number, status, subject);
				return 0;
			}
			else
			{
				UserData.ud.dic_total_rooms[room_number].Subject = subject;
				UserData.ud.dic_total_rooms[room_number].Status = status;
			}
			return -1;
		}
	}
}
