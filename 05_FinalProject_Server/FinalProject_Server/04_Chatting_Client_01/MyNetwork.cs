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
		
		string IP = "192.168.1.9";
		int PORT = 9000;
		Socket m_sock;

		public DispatcherTimer m_recv_Timer = new DispatcherTimer();

		byte[] m_buffer_recv = new byte[Macro.SIZE_BUFFER];
		int m_cnt_recv = 0;

		public MyNetwork()
		{
			if (net == null)
				net = this;

			if (initSocket() == 0)
			{
				m_recv_Timer.Interval = TimeSpan.FromMilliseconds(1);
				m_recv_Timer.Tick += recvTimerTick;

				m_recv_Timer.Start();
			}
		}

		public int initSocket()
		{
			try
			{
				m_sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
				if (m_sock != null)
				{
					m_sock.Connect(IP, PORT);
					m_sock.Blocking = false;
				}
			}
			catch(SocketException ex)
			{
				DebugLog.debugLog(ex.ToString(), null, 0);
				return -1;
			}
			return 0;
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
		private Int32 stringToCypherPacket(byte[] buffer, int offset, string str, byte[] key)
		{
			byte[] buffer_plain = Encoding.ASCII.GetBytes(str);
			byte[] buffer_cypher = Rijndael.Encrypt(buffer_plain, 0, buffer_plain.Length, key);
			Array.Copy(buffer_cypher, 0, buffer, offset, buffer_cypher.Length);
			offset += buffer_cypher.Length;

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
		public void sendCreateRoom(string room_subject, bool is_secret)
		{
			byte[] buffer = new byte[Macro.SIZE_BUFFER];
			int idx_buffer = 0;

			// send
			valueToPacket(buffer, idx_buffer, Macro.CMD_CREATE_ROOM, Macro.SIZE_CMD);
			idx_buffer = Macro.SIZE_HEADER;

			// id
			idx_buffer = stringToPacket(buffer, idx_buffer, UserData.ud.id, Macro.SIZE_ID);

			// room is_secret
			if (is_secret)
				buffer[idx_buffer++] = Macro.ROOM_INFO_STATUS_SECRET;
			else
				buffer[idx_buffer++] = Macro.ROOM_INFO_STATUS_NORMAL;

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
		public void sendChattingMessage(MyRoom my_room, string message)
		{
			byte[] buffer = new byte[Macro.SIZE_BUFFER];
			int idx_buffer = 0;

			// send
			valueToPacket(buffer, idx_buffer, Macro.CMD_CHATTING_MESSAGE, Macro.SIZE_CMD);
			idx_buffer = Macro.SIZE_HEADER;

			// id
			idx_buffer = stringToPacket(buffer, idx_buffer, UserData.ud.id, Macro.SIZE_ID);

			// room number
			idx_buffer = valueToPacket(buffer, idx_buffer, my_room.room_number, 4);

			// message
			if (my_room.Status == Macro.ROOM_INFO_STATUS_NORMAL)
				idx_buffer = stringToPacket(buffer, idx_buffer, message, Encoding.ASCII.GetByteCount(message));
			else if (my_room.Status == Macro.ROOM_INFO_STATUS_SECRET)
				idx_buffer = stringToCypherPacket(buffer, idx_buffer, message, my_room.secret_key);

			// length
			valueToPacket(buffer, Macro.SIZE_CMD, idx_buffer, Macro.SIZE_PACKET_LENGTH);

			sendBuffer(buffer, 0, idx_buffer);
		}
		#endregion

		#region Recv

		private void recvTimerTick(object sender, EventArgs e)
		{
			try
			{
				recvProcess();
			}
			catch (SocketException ex)
			{
				DebugLog.debugLog(ex.ToString(), null, 0);
				m_recv_Timer.Stop();
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
				DebugLog.debugLog("Recv [" + m_cnt_recv + "]", m_buffer_recv, size_packet);


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
				{
					throw ex;  // any serious error occurr
				}
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
			int idx = Macro.SIZE_HEADER + Macro.SIZE_ID;
			int room_number = BitConverter.ToInt32(packet, idx);
			byte status = packet[idx + 4];
			string subject = Encoding.ASCII.GetString(packet, idx + 4 + 1, Macro.SIZE_ROOM_SUBJECT);
			byte[] key = new byte[Macro.SIZE_SECRET_KEY];
			Array.Copy(packet, idx + 4 + 1 + Macro.SIZE_ROOM_SUBJECT, key, 0, Macro.SIZE_SECRET_KEY);
			string chat = "";

			UserData.ud.addTotalRoom(room_number, status, subject);
			UserData.ud.addMyRoom(room_number, status, subject, key, chat);

			sendViewRoom(BitConverter.ToInt32(packet, idx));
		}
		private void cmdEnterRoom(byte[] packet, int size_packet)
		{
			int idx = Macro.SIZE_HEADER + Macro.SIZE_ID;
			int room_number = BitConverter.ToInt32(packet, idx);
			byte status = packet[idx + 4];
			string room_subject = Encoding.ASCII.GetString(packet, idx + 4 + 1, Macro.SIZE_ROOM_SUBJECT);
			byte[] key = new byte[Macro.SIZE_SECRET_KEY];
			Array.Copy(packet, idx + 4 + 1 + Macro.SIZE_ROOM_SUBJECT, key, 0, Macro.SIZE_SECRET_KEY);
			string chat = "";

			UserData.ud.addMyRoom(room_number, status, room_subject, key, chat);

			sendViewRoom(BitConverter.ToInt32(packet, idx));
		}
		private void cmdLeaveRoom(byte[] packet, int size_packet)
		{
			int room_number = BitConverter.ToInt32(packet, Macro.SIZE_HEADER + Macro.SIZE_ID);

			UserData.ud.delMyRoom(room_number);
		}
		private void cmdViewRoom(byte[] packet, int size_packet)
		{
			int room_number = BitConverter.ToInt32(packet, Macro.SIZE_HEADER + Macro.SIZE_ID);

			MyRoom my_room = UserData.ud.findMyRoom(room_number);

			if (my_room == null)
				return;

			if (my_room.wnd != null)
			{
				my_room.wnd.Focus();
				return;
			}

			if (my_room.Status == Macro.ROOM_INFO_STATUS_SECRET)
			{
				byte[] buffer_plain = Rijndael.Decrypt(
										packet,
										Macro.SIZE_HEADER + Macro.SIZE_ID + 4,
										size_packet - (Macro.SIZE_HEADER + Macro.SIZE_ID + 4),
										my_room.secret_key
										);

				my_room.setLogChatting(
					// log chatting
					Encoding.ASCII.GetString(buffer_plain, 0, buffer_plain.Length)
					);
			}
			else
			{
				my_room.setLogChatting(
					// log chatting
					Encoding.ASCII.GetString(packet, Macro.SIZE_HEADER + Macro.SIZE_ID + 4, size_packet - (Macro.SIZE_HEADER + Macro.SIZE_ID + 4))
					);
			}
			new WindowChatting(my_room).Show();
		}
		private void cmdTotalRoomList(byte[] packet, int size_packet)
		{
			int idx;
			
			int room_number;
			byte status;
			string subject;

			idx = Macro.SIZE_HEADER + Macro.SIZE_ID;
			UserData.ud.clearTotalRoom();
			while (idx < size_packet)
			{
				room_number = BitConverter.ToInt32(m_buffer_recv, idx);
				status = m_buffer_recv[idx + 4];
				subject = Encoding.ASCII.GetString(m_buffer_recv, idx + 4 + 1, Macro.SIZE_ROOM_SUBJECT);
				idx += 5 + Macro.SIZE_ROOM_SUBJECT;

				UserData.ud.addTotalRoom(room_number, status, subject);
			}
		}
		private void cmdMyRoomList(byte[] packet, int size_packet)
		{
			int idx;

			int room_number;
			byte status;
			string subject;
			byte[] key = new byte[Macro.SIZE_SECRET_KEY];
			string chat;

			idx = Macro.SIZE_HEADER + Macro.SIZE_ID;

			while (idx < size_packet)
			{
				room_number = BitConverter.ToInt32(m_buffer_recv, idx);
				status = m_buffer_recv[idx + 4];
				subject = Encoding.ASCII.GetString(m_buffer_recv, idx + 4 + 1, Macro.SIZE_ROOM_SUBJECT);
				Array.Copy(packet, idx + 4 + 1 + Macro.SIZE_ROOM_SUBJECT, key, 0, Macro.SIZE_SECRET_KEY);
				chat = "";
				idx += 5 + Macro.SIZE_ROOM_SUBJECT;

				UserData.ud.addMyRoom(room_number, status, subject, key, chat);
			}
		}
		private void cmdChattingMessage(byte[] packet, int size_packet)
		{
			int room_number = BitConverter.ToInt32(packet, Macro.SIZE_HEADER + Macro.SIZE_ID);

			MyRoom my_room = UserData.ud.findMyRoom(room_number);

			if (my_room == null)
				return;

			if (my_room.Status == Macro.ROOM_INFO_STATUS_SECRET)
			{
				byte[] buffer_plain = Rijndael.Decrypt(
											packet, 
											Macro.SIZE_HEADER + Macro.SIZE_ID + 4, 
											size_packet - (Macro.SIZE_HEADER + Macro.SIZE_ID + 4), my_room.secret_key
											);


				my_room.addLogChatting(
					// message
					Encoding.ASCII.GetString(buffer_plain, 0, buffer_plain.Length)
					);
			}
			else if (my_room.Status == Macro.ROOM_INFO_STATUS_NORMAL)
			{
				my_room.addLogChatting(
					// message
					Encoding.ASCII.GetString(
						packet, 
						Macro.SIZE_HEADER + Macro.SIZE_ID + 4, 
						size_packet - (Macro.SIZE_HEADER + Macro.SIZE_ID + 4))
					);
			}

			if (my_room.wnd != null)
			{
				my_room.wnd.updateChat();
			}
		}
		
		#endregion
		
	}
}
