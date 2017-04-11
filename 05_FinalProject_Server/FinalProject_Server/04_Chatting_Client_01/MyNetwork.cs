using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Windows.Threading;

namespace _04_Chatting_Client_01
{
	public class MyNetwork
	{
		public static MyNetwork net = null;
		
		string IP = "192.168.1.3";
		int PORT = 9000;
		Socket m_sock;

		public DispatcherTimer m_recv_Timer = new DispatcherTimer();

		byte[] m_buffer_recv = new byte[Macro.SIZE_BUFFER];
		int m_cnt_recv = 0;

		public MyNetwork()
		{
			if (net == null)
				net = this;

			initSocket();
			m_recv_Timer.Interval = TimeSpan.FromMilliseconds(1);
			m_recv_Timer.Tick += recvTimerTick;
		
			m_recv_Timer.Start();
		}

		public void initSocket()
		{
			m_sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
			m_sock.Connect(IP, PORT);
			m_sock.Blocking = false;
		}
		private void recvTimerTick(object sender, EventArgs e)
		{
			recvProcess();
		}

		#region Send
		private int sendBuffer(byte[] buffer, int offset, int size)
		{
			int retval = 0, sendBytes = 0;
			while (sendBytes < size)
			{
				try
				{
					retval = m_sock.Send(buffer, offset, size, 0);
				}
				catch (SocketException ex)
				{
					if (ex.SocketErrorCode == SocketError.WouldBlock ||
						ex.SocketErrorCode == SocketError.IOPending ||
						ex.SocketErrorCode == SocketError.NoBufferSpaceAvailable)
					{
						// socket buffer is probably full, wait and try again
						Thread.Sleep(30);
					}
					else
						throw ex;  // any serious error occurr
				}
				if (retval > 0)
					sendBytes += retval;
			}
			DebugLog.debugLog("Send", buffer, size);
			return sendBytes;
		}

		private Int32 stringToPacket(byte[] buffer, int offset, string str, int max_size)
		{
			int size_str = Encoding.ASCII.GetByteCount(str);
			Array.Copy(Encoding.ASCII.GetBytes(str), 0, buffer, offset, size_str);
			offset += size_str;

			Array.Clear(buffer, offset, max_size - size_str);
			offset += max_size - size_str;

			return offset;
		}
		private Int32 valueToPacket(byte[] buffer, int offset, Int32 value, int size)
		{
			Array.Copy(BitConverter.GetBytes(value), 0, buffer, offset, size);
			offset += size;
			return offset;
		}
		private Int32 valueToPacket<T>(byte[] buffer, int offset, UInt16 value, int size)
		{
			Array.Copy(BitConverter.GetBytes(value), 0, buffer, offset, size);
			offset += size;
			return offset;
		}

		public void sendCreateId(string id)
		{
			byte[] buffer = new byte[Macro.SIZE_BUFFER];
			int idx_buffer = 0;
			
			//UserData.ud.id = id;

			// send
			valueToPacket(buffer, idx_buffer, Macro.CMD_CREATE_ID, Macro.SIZE_CMD);
			idx_buffer = Macro.SIZE_HEADER;

			// id
			idx_buffer = stringToPacket(buffer, idx_buffer, id, Macro.SIZE_ID);

			// length
			valueToPacket(buffer, Macro.SIZE_CMD, idx_buffer, Macro.SIZE_PACKET_LENGTH);

			// send
			sendBuffer(buffer, 0, idx_buffer);
		}
		public void sendCreateRoom(string room_subject)
		{
			byte[] buffer = new byte[Macro.SIZE_BUFFER];
			int idx_buffer = 0;

			// send
			valueToPacket(buffer, idx_buffer, Macro.CMD_CREATE_ROOM, Macro.SIZE_CMD);
			idx_buffer = Macro.SIZE_HEADER;

			// id
			idx_buffer = stringToPacket(buffer, idx_buffer, UserData.ud.id, Macro.SIZE_ID);

			// room is_secret
			idx_buffer = stringToPacket(buffer, idx_buffer, "N", 1);

			// room subject
			idx_buffer = stringToPacket(buffer, idx_buffer, room_subject, Macro.SIZE_ROOM_SUBJECT);

			// length
			valueToPacket(buffer, Macro.SIZE_CMD, idx_buffer, Macro.SIZE_PACKET_LENGTH);


			sendBuffer(buffer, 0, idx_buffer);
		}
		public void sendEnterRoom(Int32 room_number)
		{
			byte[] buffer = new byte[Macro.SIZE_BUFFER];
			int idx_buffer = 0;

			// send
			valueToPacket(buffer, idx_buffer, Macro.CMD_ENTER_ROOM, Macro.SIZE_CMD);
			idx_buffer = Macro.SIZE_HEADER;

			// id
			idx_buffer = stringToPacket(buffer, idx_buffer, UserData.ud.id, Macro.SIZE_ID);

			// room number
			idx_buffer = valueToPacket(buffer, idx_buffer, room_number, 4);

			// length
			valueToPacket(buffer, Macro.SIZE_CMD, idx_buffer, Macro.SIZE_PACKET_LENGTH);

			sendBuffer(buffer, 0, idx_buffer);
		}
		public void sendLeaveRoom(Int32 room_number)
		{
			byte[] buffer = new byte[Macro.SIZE_BUFFER];
			int idx_buffer = 0;

			// send
			valueToPacket(buffer, idx_buffer, Macro.CMD_LEAVE_ROOM, Macro.SIZE_CMD);
			idx_buffer = Macro.SIZE_HEADER;

			// id
			idx_buffer = stringToPacket(buffer, idx_buffer, UserData.ud.id, Macro.SIZE_ID);

			// room number
			idx_buffer = valueToPacket(buffer, idx_buffer, room_number, 4);

			// length
			valueToPacket(buffer, Macro.SIZE_CMD, idx_buffer, Macro.SIZE_PACKET_LENGTH);

			sendBuffer(buffer, 0, idx_buffer);
		}
		public void sendViewRoom(Int32 room_number)
		{
			byte[] buffer = new byte[Macro.SIZE_BUFFER];
			int idx_buffer = 0;

			// send
			valueToPacket(buffer, idx_buffer, Macro.CMD_VIEW_ROOM, Macro.SIZE_CMD);
			idx_buffer = Macro.SIZE_HEADER;

			// id
			idx_buffer = stringToPacket(buffer, idx_buffer, UserData.ud.id, Macro.SIZE_ID);

			// room number
			idx_buffer = valueToPacket(buffer, idx_buffer, room_number, 4);

			// length
			valueToPacket(buffer, Macro.SIZE_CMD, idx_buffer, Macro.SIZE_PACKET_LENGTH);

			sendBuffer(buffer, 0, idx_buffer);
		}
		public void sendTotalRoomList()
		{
			byte[] buffer = new byte[Macro.SIZE_BUFFER];
			int idx_buffer = 0;

			// send
			valueToPacket(buffer, idx_buffer, Macro.CMD_TOTAL_ROOM_LIST, Macro.SIZE_CMD);
			idx_buffer = Macro.SIZE_HEADER;

			// id
			idx_buffer = stringToPacket(buffer, idx_buffer, UserData.ud.id, Macro.SIZE_ID);

			// length
			valueToPacket(buffer, Macro.SIZE_CMD, idx_buffer, Macro.SIZE_PACKET_LENGTH);

			sendBuffer(buffer, 0, idx_buffer);
		}
		public void sendMyRoomList()
		{
			byte[] buffer = new byte[Macro.SIZE_BUFFER];
			int idx_buffer = 0;

			// send
			valueToPacket(buffer, idx_buffer, Macro.CMD_MY_ROOM_LIST, Macro.SIZE_CMD);
			idx_buffer = Macro.SIZE_HEADER;

			// id
			idx_buffer = stringToPacket(buffer, idx_buffer, UserData.ud.id, Macro.SIZE_ID);

			// length
			valueToPacket(buffer, Macro.SIZE_CMD, idx_buffer, Macro.SIZE_PACKET_LENGTH);

			sendBuffer(buffer, 0, idx_buffer);
		}
		public void sendChattingMessage(Int32 room_number, string message)
		{
			byte[] buffer = new byte[Macro.SIZE_BUFFER];
			int idx_buffer = 0;

			// send
			valueToPacket(buffer, idx_buffer, Macro.CMD_CHATTING_MESSAGE, Macro.SIZE_CMD);
			idx_buffer = Macro.SIZE_HEADER;

			// id
			idx_buffer = stringToPacket(buffer, idx_buffer, UserData.ud.id, Macro.SIZE_ID);

			// room number
			idx_buffer = valueToPacket(buffer, idx_buffer, room_number, 4);

			// message
			idx_buffer = stringToPacket(buffer, idx_buffer, message, Encoding.ASCII.GetByteCount(message));

			// length
			valueToPacket(buffer, Macro.SIZE_CMD, idx_buffer, Macro.SIZE_PACKET_LENGTH);

			sendBuffer(buffer, 0, idx_buffer);
		}
		#endregion

		#region Recv
		private int recvBuffer(byte[] buffer, int offset, int size)
		{
			int retval = 0;
			try
			{
				retval = m_sock.Receive(buffer, offset, size, 0);
			}
			catch (SocketException ex)
			{
				if (ex.SocketErrorCode == SocketError.WouldBlock ||
					ex.SocketErrorCode == SocketError.IOPending ||
					ex.SocketErrorCode == SocketError.NoBufferSpaceAvailable)
				{
					;
				}
				else
					throw ex;  // any serious error occurr
			}

			return retval;
		}
		
		private void cmdCreateId(byte[] packet, int size_packet)
		{
			WindowRoomList wnd_room_list = new WindowRoomList(
				//id
				Encoding.ASCII.GetString(packet, Macro.SIZE_HEADER, Macro.SIZE_ID));
			wnd_room_list.Show();
			MainWindow.wnd.Close();
		}
		private void cmdCreateRoom(byte[] packet, int size_packet)
		{
			byte[] key = new byte[Macro.SIZE_SECRET_KEY];
			Array.Copy(packet, Macro.SIZE_HEADER + Macro.SIZE_ID + 4, key, 0, Macro.SIZE_SECRET_KEY);

			//m_list_myroom.Add(new MyRoomList(BitConverter.ToInt32(m_buffer_recv, Macro.SIZE_HEADER + Macro.SIZE_ID), 'N', "test", key));
		}
		private void cmdEnterRoom(byte[] packet, int size_packet)
		{
			int idx = 0;

			idx = Macro.SIZE_HEADER + Macro.SIZE_ID;
			WindowRoomList.wnd.addButtonMyList(
				// room number
				BitConverter.ToInt32(packet, idx),
				// status
				packet[idx + 4],
				// room subject
				Encoding.ASCII.GetString(packet, idx + 5, Macro.SIZE_ROOM_SUBJECT),
				// key
				null,
				// chat
				"");

			sendViewRoom(BitConverter.ToInt32(packet, idx));
		}
		private void cmdLeaveRoom(byte[] packet, int size_packet)
		{
			int room_number = BitConverter.ToInt32(packet, Macro.SIZE_HEADER + Macro.SIZE_ID);
			MyRoom my_room = UserData.ud.dic_my_rooms[room_number];

			if(my_room != null)
			{
				WindowRoomList.wnd.delButtonMyList(my_room.btn);
				UserData.ud.dic_my_rooms.Remove(room_number);
			}
		}
		private void cmdViewRoom(byte[] packet, int size_packet)
		{
			int room_number = BitConverter.ToInt32(packet, Macro.SIZE_HEADER + Macro.SIZE_ID);
			MyRoom my_room = UserData.ud.dic_my_rooms[room_number];

			if (my_room != null)
			{
				if (my_room.wnd == null)
				{
					my_room.addLogChatting(
						// log chatting
						Encoding.ASCII.GetString(packet, Macro.SIZE_HEADER + Macro.SIZE_ID + 4, size_packet - (Macro.SIZE_HEADER + Macro.SIZE_ID + 4)));

					new WindowChatting(my_room).Show();
				}
				else
				{
					my_room.wnd.Focus();
				}
			}
		}
		private void cmdTotalRoomList(byte[] packet, int size_packet)
		{
			int idx;
			
			int room_number;
			string subject;

			idx = Macro.SIZE_HEADER + Macro.SIZE_ID;

			while (idx < size_packet)
			{
				room_number = BitConverter.ToInt32(m_buffer_recv, idx);
				subject = Encoding.ASCII.GetString(m_buffer_recv, idx + 5, Macro.SIZE_ROOM_SUBJECT);

				if (UserData.ud.dic_total_rooms[room_number] == null)
				{
					Button btn = WindowRoomList.wnd.addButtonTotalList(room_number, subject);
					UserData.ud.dic_total_rooms[room_number] = new TotalRoom(room_number, subject);
				}

				idx += 5 + Macro.SIZE_ROOM_SUBJECT;
			}
		}
		private void cmdMyRoomList(byte[] packet, int size_packet)
		{
			int idx;

			int room_number;
			byte status;
			string subject;
			byte[] key;
			string chat;

			idx = Macro.SIZE_HEADER + Macro.SIZE_ID;

			while (idx < size_packet)
			{
				room_number = BitConverter.ToInt32(m_buffer_recv, idx);
				status = m_buffer_recv[idx + 4];
				subject = Encoding.ASCII.GetString(m_buffer_recv, idx + 5, Macro.SIZE_ROOM_SUBJECT);
				key = null;
				chat = "";
				if (UserData.ud.dic_my_rooms[room_number] == null)
				{
					Button btn = WindowRoomList.wnd.addButtonMyList(
									room_number, status, subject, key, chat);
					UserData.ud.dic_my_rooms[room_number] = new MyRoom(
									room_number, status, subject, key, chat, btn);
				}
				idx += 5 + Macro.SIZE_ROOM_SUBJECT;
			}
		}
		private void cmdChattingMessage(byte[] packet, int size_packet)
		{
			int room_number = BitConverter.ToInt32(packet, Macro.SIZE_HEADER + Macro.SIZE_ID);
			MyRoom my_room = UserData.ud.dic_my_rooms[room_number];

			if (my_room != null)
			{
				my_room.addLogChatting(
					// id
					Encoding.ASCII.GetString(packet, Macro.SIZE_HEADER, Macro.SIZE_ID),
					// message
					Encoding.ASCII.GetString(packet, Macro.SIZE_HEADER + Macro.SIZE_ID + 4, size_packet - (Macro.SIZE_HEADER + Macro.SIZE_ID + 4)));

				if (my_room.wnd != null)
				{
					my_room.wnd.updateChat();
				}
			}
		}
		private void recvProcess()
		{
			int retval = recvBuffer(m_buffer_recv, m_cnt_recv, Macro.SIZE_BUFFER - m_cnt_recv);
			if (retval > 0)
				m_cnt_recv += retval;

			int size_packet = 0;
			while (m_cnt_recv >= Macro.SIZE_HEADER
				&& m_cnt_recv >= (size_packet = BitConverter.ToInt32(m_buffer_recv, Macro.SIZE_CMD)))
			{
				DebugLog.debugLog("Recv", m_buffer_recv, size_packet);


				switch (BitConverter.ToUInt16(m_buffer_recv, 0))
				{
					case Macro.CMD_CREATE_ID:
						cmdCreateId(m_buffer_recv, size_packet);
						break;
					case Macro.CMD_CREATE_ROOM:
						cmdCreateRoom(m_buffer_recv, size_packet);
						break;
					case Macro.CMD_ENTER_ROOM:
						cmdEnterRoom(m_buffer_recv, size_packet);
						break;
					case Macro.CMD_LEAVE_ROOM:
						cmdLeaveRoom(m_buffer_recv, size_packet);
						break;
					case Macro.CMD_VIEW_ROOM:
						cmdViewRoom(m_buffer_recv, size_packet);
						break;
					case Macro.CMD_TOTAL_ROOM_LIST:
						cmdTotalRoomList(m_buffer_recv, size_packet);
						break;
					case Macro.CMD_MY_ROOM_LIST:
						cmdMyRoomList(m_buffer_recv, size_packet);
						break;
					case Macro.CMD_CHATTING_MESSAGE:
						cmdChattingMessage(m_buffer_recv, size_packet);
						break;
					default:
						break;
				}


				m_cnt_recv -= size_packet;
				Array.Copy(m_buffer_recv, size_packet, m_buffer_recv, 0, m_cnt_recv);
			}
		}
		#endregion
		
	}
}
