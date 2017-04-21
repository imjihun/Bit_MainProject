using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Windows.Media;

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
				grd.Children.OfType<Button>().FirstOrDefault().Content = "[" + room_number + "] " + subject + "\n" + Chatting_last_line.TrimStart('\n');
			}
		}
		public byte[] secret_key = new byte[Macro.SIZE_SECRET_KEY];
		private int count_member = 0;
		public int Count_member
		{
			get { return count_member; }
			set
			{
				count_member = value;
				if (this.wnd != null)
				{
					this.wnd.Title = "(" + count_member + ")" + subject.TrimEnd('\0');
					this.wnd.textBlock_title.Text = "(" + count_member + ")" + subject.TrimEnd('\0');
				}
			}
		}
		StringBuilder log_chatting = new StringBuilder();
		public StringBuilder Log_chatting {
			get {return log_chatting; }
		}

		public bool bCreateRoom = false;
		string chatting_last_line = "";
		public string Chatting_last_line {
			get { return chatting_last_line; }
			set {
				string[] arr = value.Split('\n');
				if(arr.Length > 1)
					chatting_last_line = arr[arr.Length - 2];

				if (grd != null)
				{
					grd.Children.OfType<Button>().FirstOrDefault().Content = "[" + room_number + "] " + subject + "\n" + chatting_last_line.TrimStart('\n');
					if (wnd == null && !bCreateRoom)
					{
						if (this.notice == null)
						{
							this.notice = new Window_notice(this);
							notice.Show();
						}
						grd.Children.OfType<Button>().FirstOrDefault().Background = new SolidColorBrush(Color.FromArgb(255, 0xE1, 0xFF, 0xC8));
					}
					bCreateRoom = false;
				}
			}
		}

		public Grid grd = null;
		public Window_notice notice = null;

		public WindowChatting wnd = null;

		public MyRoom(int num, byte stat, string sub, byte[] key, int cnt_mem, string chat)
		{
			room_number = num;
			status = stat;
			subject = sub;
			if(key != null)
				Array.Copy(key, secret_key, Macro.SIZE_SECRET_KEY);
			count_member = cnt_mem;

			addLogChatting(chat);
			grd = WindowRoomList.wnd.addGridMyList(
							room_number, status, subject, key, Chatting_last_line);
		}

		const int MAX_LOG_CHATTING = 4096;
		public void addLogChatting(string log)
		{
			log_chatting.Append(log);

			Chatting_last_line = log;

			if (log_chatting.Length > MAX_LOG_CHATTING)
				log_chatting.Remove(0, log_chatting.Length - MAX_LOG_CHATTING);
		}
		public void setLogChatting(string log)
		{
			log_chatting.Clear();
			addLogChatting(log);
		}
	}

	public class TotalRoom
	{
		private string subject;
		private byte status;

		public bool isNewList = false;
		public int room_number;
		public byte Status
		{
			get { return status; }
			set
			{
				status = value;
				isNewList = true;
				;
			}
		}

		public string Subject
		{
			get { return subject; }
			set {
				subject = value;
				isNewList = true;
				;
			}
		}
		public int count_member = 0;
		public Button btn = null;

		public TotalRoom(int r, byte stat, string s, int c)
		{
			room_number = r;
			status = stat;
			subject = s;
			count_member = c;
			btn = WindowRoomList.wnd.addButtonTotalList(room_number, stat, subject);
			isNewList = true;
		}
	}

	public class UserData
	{
		public static UserData ud = null;

		public Socket server;
		public string id;
		public Dictionary<int, MyRoom> dic_my_rooms = new Dictionary<int, MyRoom>();
		public Dictionary<int, TotalRoom> dic_total_rooms = new Dictionary<int, TotalRoom>();
		public int count_total_room = 0;

		public UserData(string _id)
		{
			if (ud == null)
				ud = this;

			id = _id;
		}

		public int addMyRoom(int room_number, byte status, string subject, byte[] key, string chat, int count_member)
		{
			if (!UserData.ud.dic_my_rooms.ContainsKey(room_number))
			{
				UserData.ud.dic_my_rooms[room_number] = new MyRoom(
								room_number, status, subject, key, count_member, chat);
				return 0;
			}
			else
			{
				UserData.ud.dic_my_rooms[room_number].Status = status;
				UserData.ud.dic_my_rooms[room_number].Subject = subject;
				Array.Copy(UserData.ud.dic_my_rooms[room_number].secret_key, key, Macro.SIZE_SECRET_KEY);
				UserData.ud.dic_my_rooms[room_number].setLogChatting(chat);
				UserData.ud.dic_my_rooms[room_number].Count_member = count_member;
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
				if (my_room.Count_member <= 1)
					delTotalRoom(room_number);

				if(my_room.wnd != null)
					my_room.wnd.Close();
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

		public int delTotalRoom(int room_number)
		{
			if (!UserData.ud.dic_total_rooms.ContainsKey(room_number))
				return -1;

			TotalRoom total_room = UserData.ud.dic_total_rooms[room_number];

			if (total_room == null)
				return -1;
			
			if (total_room.btn != null)
			{
				WindowRoomList.wnd.stackPanel_totallist.Children.Remove(total_room.btn);
				total_room.btn = null;
			}
			UserData.ud.dic_total_rooms.Remove(total_room.room_number);
			return 0;
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
		public int addTotalRoom(int room_number, byte status, string subject, int count_member)
		{
			if (UserData.ud.count_total_room == 0)
			{
				TotalRoom tmp;
				for(int i = 0; i < UserData.ud.dic_total_rooms.Count; i++)
				{
					tmp = UserData.ud.dic_total_rooms.Values.ToArray()[i];
					if (tmp.isNewList == false)
					{
						if (tmp.btn != null)
						{
							WindowRoomList.wnd.stackPanel_totallist.Children.Remove(tmp.btn);
							tmp.btn = null;
						}
						UserData.ud.dic_total_rooms.Remove(tmp.room_number);
					}
					tmp.isNewList = false;
				}
			}

			UserData.ud.count_total_room++;
			if (!UserData.ud.dic_total_rooms.ContainsKey(room_number))
			{
				UserData.ud.dic_total_rooms[room_number] = new TotalRoom(room_number, status, subject, count_member);
				return 0;
			}
			else
			{
				UserData.ud.dic_total_rooms[room_number].room_number = room_number;
				UserData.ud.dic_total_rooms[room_number].Subject = subject;
				UserData.ud.dic_total_rooms[room_number].Status = status;
			}
			return -1;
		}
	}
}
