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
		
		string IP = "192.168.1.14";
		int PORT = 9000;
		Socket m_sock;

		public DispatcherTimer m_recv_Timer = new DispatcherTimer();

		byte[] m_buffer_recv = new byte[Macro.SIZE_BUFFER];
		int m_cnt_recv = 0;

		public MyNetwork()
		{
			if (initSocket() == 0)
			{
				if (net == null)
					net = this;

				m_recv_Timer.Interval = TimeSpan.FromMilliseconds(1);
				m_recv_Timer.Tick += recvTimerTick;

				m_recv_Timer.Start();
			}
			else
				net = null;
		}

		public void restartNetwork()
		{
			for (int i = 0; i < 10; i++)
			{
				Thread.Sleep(1000);

				m_recv_Timer.Stop();
				if (closeSocket() != 0)
					continue;
				if (initSocket() != 0)
					continue;
				m_cnt_recv = 0;

				sendCreateId(UserData.ud.id);

				m_recv_Timer.Start();
				return;
			}
			App.Current.Shutdown();
		}
		public int closeSocket()
		{
			if (m_sock != null && m_sock.Connected)
				m_sock.Close();
			m_sock = null;
			return 0;
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
				DebugLog.debugLog(ex.ToString());
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
					{
						restartNetwork();
						//throw ex;  // any serious error occurr
					}
				}
				if (retval > 0)
					sendBytes += retval;
			}
			DebugLog.debugLog("Send", buffer, size);
			return sendBytes;
		}

		private Int32 stringToPacket(byte[] buffer, int offset, string str, int max_size)
		{
			int size_str = Encoding.UTF8.GetByteCount(str);
			Array.Copy(Encoding.UTF8.GetBytes(str), 0, buffer, offset, size_str);
			offset += size_str;

			Array.Clear(buffer, offset, max_size - size_str);
			offset += max_size - size_str;

			return offset;
		}
		private Int32 stringToCypherPacket(byte[] buffer, int offset, string str, byte[] key)
		{
			byte[] buffer_plain = Encoding.UTF8.GetBytes(str);
			byte[] buffer_cypher = Rijndael.encrypt(buffer_plain, 0, buffer_plain.Length, key);
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
		private Int32 valueToPacket(byte[] buffer, int offset, UInt16 value, int size)
		{
			Array.Copy(BitConverter.GetBytes(value), 0, buffer, offset, size);
			offset += size;
			return offset;
		}

		public void sendCreateId(string id)
		{
			byte[] buffer = new byte[Macro.SIZE_BUFFER];
			int idx_buffer = 0;

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
			UserData.ud.count_total_room = 0;
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
				idx_buffer = stringToPacket(buffer, idx_buffer, message, Encoding.UTF8.GetByteCount(message));
			else if (my_room.Status == Macro.ROOM_INFO_STATUS_SECRET)
				idx_buffer = stringToCypherPacket(buffer, idx_buffer, message, my_room.secret_key);

			// length
			valueToPacket(buffer, Macro.SIZE_CMD, idx_buffer, Macro.SIZE_PACKET_LENGTH);

			sendBuffer(buffer, 0, idx_buffer);
		}
		public void sendInvite(string yourid, int room_number)
		{
			byte[] buffer = new byte[Macro.SIZE_BUFFER];
			int idx_buffer = 0;

			// send
			valueToPacket(buffer, idx_buffer, Macro.CMD_INVITE, Macro.SIZE_CMD);
			idx_buffer = Macro.SIZE_HEADER;

			// myid
			idx_buffer = stringToPacket(buffer, idx_buffer, UserData.ud.id, Macro.SIZE_ID);

			// your id
			idx_buffer = stringToPacket(buffer, idx_buffer, yourid, Macro.SIZE_ID);

			// room_number
			idx_buffer = valueToPacket(buffer, idx_buffer, room_number, 4);

			// length
			valueToPacket(buffer, Macro.SIZE_CMD, idx_buffer, Macro.SIZE_PACKET_LENGTH);

			// send
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
				DebugLog.debugLog(ex.ToString());
				m_recv_Timer.Stop();
			}
		}
		private int checkCMD()
		{
			UInt16 cmd = BitConverter.ToUInt16(m_buffer_recv, 0);
			if (cmd == Macro.CMD_CREATE_ID
				|| cmd == Macro.CMD_CREATE_ROOM
				|| cmd == Macro.CMD_ENTER_ROOM
				|| cmd == Macro.CMD_LEAVE_ROOM
				|| cmd == Macro.CMD_VIEW_ROOM
				|| cmd == Macro.CMD_TOTAL_ROOM_LIST
				|| cmd == Macro.CMD_MY_ROOM_LIST
				|| cmd == Macro.CMD_CHATTING_MESSAGE
				|| cmd == Macro.CMD_INVITE)
				return 0;
			else
				return -1;
		}
		private void recvProcess()
		{
			int retval = recvBuffer(m_buffer_recv, m_cnt_recv, Macro.SIZE_BUFFER - m_cnt_recv);
			if (retval > 0)
				m_cnt_recv += retval;

			if (m_cnt_recv < Macro.SIZE_HEADER)
				return;
			if (checkCMD() < 0)
			{
				restartNetwork();
			}

			int size_packet = 0;
			while (m_cnt_recv >= Macro.SIZE_HEADER
				&& m_cnt_recv >= (size_packet = BitConverter.ToInt32(m_buffer_recv, Macro.SIZE_CMD)))
			{
				DebugLog.debugLog("Recv(" + m_cnt_recv + ")", m_buffer_recv, size_packet);
				UInt16 cmd = BitConverter.ToUInt16(m_buffer_recv, 0);
				if (BitConverter.ToUInt16(m_buffer_recv, Macro.SIZE_HEADER) != Macro.CMD_FAIL)
				{
					switch (cmd)
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
						case Macro.CMD_INVITE:
							cmdInvite(m_buffer_recv, size_packet);
							break;
						default:
							restartNetwork();
							break;
					}
				}
				// dbfail packet processing
				else
				{
					if (cmd == Macro.CMD_ENTER_ROOM || cmd == Macro.CMD_VIEW_ROOM)
						UserData.ud.delMyRoom(BitConverter.ToInt32(m_buffer_recv, Macro.SIZE_HEADER + Macro.SIZE_CMD));
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
					restartNetwork();
					//throw ex;  // any serious error occurr
				}
			}

			return retval;
		}

		private void cmdCreateId(byte[] packet, int size_packet)
		{
			if (BitConverter.ToUInt16(packet, Macro.SIZE_HEADER) == Macro.CMD_FAIL)
				return;

			string id = Encoding.UTF8.GetString(packet, Macro.SIZE_HEADER, Macro.SIZE_ID);

			WindowRoomList wnd_room_list = new WindowRoomList(id);
			wnd_room_list.Show();
			WindowLogin.wnd.Close();
		}
		private void cmdCreateRoom(byte[] packet, int size_packet)
		{
			int idx = Macro.SIZE_HEADER + Macro.SIZE_ID;
			int room_number = BitConverter.ToInt32(packet, idx);
			byte status = packet[idx + 4];
			string subject = Encoding.UTF8.GetString(packet, idx + 4 + 1, Macro.SIZE_ROOM_SUBJECT);
			byte[] key = new byte[Macro.SIZE_SECRET_KEY];
			Array.Copy(packet, idx + 4 + 1 + Macro.SIZE_ROOM_SUBJECT, key, 0, Macro.SIZE_SECRET_KEY);
			string chat = "";

			if(status == Macro.ROOM_INFO_STATUS_NORMAL)
				UserData.ud.addTotalRoom(room_number, status, subject, 1);
			UserData.ud.addMyRoom(room_number, status, subject, key, chat, 1);
			MyRoom r = UserData.ud.findMyRoom(room_number);
			r.bCreateRoom = true;
			sendViewRoom(room_number);
		}
		private void cmdEnterRoom(byte[] packet, int size_packet)
		{
			string id = Encoding.UTF8.GetString(packet, Macro.SIZE_HEADER, Macro.SIZE_ID);
			int idx = Macro.SIZE_HEADER + Macro.SIZE_ID;
			int room_number = BitConverter.ToInt32(packet, idx);
			byte status = packet[idx + 4];
			string room_subject = Encoding.UTF8.GetString(packet, idx + 4 + 1, Macro.SIZE_ROOM_SUBJECT);
			byte[] key = new byte[Macro.SIZE_SECRET_KEY];
			Array.Copy(packet, idx + 4 + 1 + Macro.SIZE_ROOM_SUBJECT, key, 0, Macro.SIZE_SECRET_KEY);
			int count_member = BitConverter.ToInt32(packet, idx + 4 + 1 + Macro.SIZE_ROOM_SUBJECT + Macro.SIZE_SECRET_KEY);
			string chat = "";
			
			if (id == UserData.ud.id)
			{
				UserData.ud.addMyRoom(room_number, status, room_subject, key, chat, count_member);
			}
			else
			{
				MyRoom room = UserData.ud.findMyRoom(room_number);
				if (room != null)
				{
					room.Count_member++;
				}

			}
			MyRoom r = UserData.ud.findMyRoom(room_number);
			r.bCreateRoom = true;
			sendViewRoom(room_number);
		}
		private void cmdLeaveRoom(byte[] packet, int size_packet)
		{
			string id = Encoding.UTF8.GetString(packet, Macro.SIZE_HEADER, Macro.SIZE_ID);
			int room_number = BitConverter.ToInt32(packet, Macro.SIZE_HEADER + Macro.SIZE_ID);
			
			if (id == UserData.ud.id)
			{
				UserData.ud.delMyRoom(room_number);
			}
			else
			{
				MyRoom room = UserData.ud.findMyRoom(room_number);
				if (room != null)
				{
					room.Count_member--;
				}

			}
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
			/* cmdMyRoomList 와 cmdChattingMessage 로 업데이트를 하는데 굳이 chatting log 업데이트를 해야하나
			if (my_room.Status == Macro.ROOM_INFO_STATUS_SECRET)
			{
				byte[] buffer_plain = Rijndael.decrypt(
										// packet
										packet,
										// offset
										Macro.SIZE_HEADER + Macro.SIZE_ID + 4,
										// size
										size_packet - (Macro.SIZE_HEADER + Macro.SIZE_ID + 4),
										// key
										my_room.secret_key
										);

				if (buffer_plain != null)
				{
					my_room.setLogChatting(
						// log chatting
						Encoding.UTF8.GetString(buffer_plain, 0, buffer_plain.Length)
						);
				}
			}
			else
			{
				my_room.setLogChatting(
					// log chatting
					Encoding.UTF8.GetString(
										// packet
										packet,
										// offset
										Macro.SIZE_HEADER + Macro.SIZE_ID + 4,
										// size
										size_packet - (Macro.SIZE_HEADER + Macro.SIZE_ID + 4))
					);
			}
			*/
			WindowChatting wnd = new WindowChatting(my_room);
			// wnd.Left = 
			wnd.Show();
		}
		private void cmdTotalRoomList(byte[] packet, int size_packet)
		{
			int idx;
			
			int room_number;
			byte status;
			string subject;
			int count_member;

			idx = Macro.SIZE_HEADER + Macro.SIZE_ID;
			//if(UserData.ud.count_total_room == 0)
			//	UserData.ud.clearTotalRoom();
			//while (idx < size_packet)
			{
				// room number
				room_number = BitConverter.ToInt32(m_buffer_recv, idx);
				idx += 4;

				// status
				status = m_buffer_recv[idx];
				idx += 1;

				// subject
				subject = Encoding.UTF8.GetString(m_buffer_recv, idx, Macro.SIZE_ROOM_SUBJECT);
				idx += Macro.SIZE_ROOM_SUBJECT;

				// counter member
				count_member = BitConverter.ToInt32(m_buffer_recv, idx);
				idx += 4;

				UserData.ud.addTotalRoom(room_number, status, subject, count_member);
			}
		}
		private void cmdMyRoomList(byte[] packet, int size_packet)
		{
			int idx;

			int room_number;
			byte status;
			string subject;
			byte[] key = new byte[Macro.SIZE_SECRET_KEY];
			int count_member;
			string chat = "";

			idx = Macro.SIZE_HEADER + Macro.SIZE_ID;

			//while (idx < size_packet)
			{
				// room number
				room_number = BitConverter.ToInt32(m_buffer_recv, idx);
				idx += 4;

				// status
				status = m_buffer_recv[idx];
				idx++;

				// subject
				subject = Encoding.UTF8.GetString(m_buffer_recv, idx, Macro.SIZE_ROOM_SUBJECT);
				idx += Macro.SIZE_ROOM_SUBJECT;

				// secret key
				Array.Copy(packet, idx, key, 0, Macro.SIZE_SECRET_KEY);
				idx += Macro.SIZE_SECRET_KEY;

				// count member
				count_member = BitConverter.ToInt32(packet, idx);
				idx += 4;

				// log chatting
				if (status == Macro.ROOM_INFO_STATUS_SECRET)
				{
					byte[] buffer_plain = Rijndael.decrypt(
											// packet
											packet,
											// offset
											idx,
											// size
											size_packet - idx,
											// key
											key
											);
					Console.WriteLine("\tplain length = " + buffer_plain.Length);
					chat = Encoding.UTF8.GetString(buffer_plain, 0, buffer_plain.Length);
				}
				else if (status == Macro.ROOM_INFO_STATUS_NORMAL)
				{
					chat = Encoding.UTF8.GetString(packet, idx, size_packet - idx);
				}

				UserData.ud.addMyRoom(room_number, status, subject, key, chat, count_member);
			}
			Console.WriteLine("\n\ncmdMyRoomList()\n\n");
		}
		private void cmdChattingMessage(byte[] packet, int size_packet)
		{
			int room_number = BitConverter.ToInt32(packet, Macro.SIZE_HEADER + Macro.SIZE_ID);

			MyRoom my_room = UserData.ud.findMyRoom(room_number);

			if (my_room == null)
				return;

			if (my_room.Status == Macro.ROOM_INFO_STATUS_SECRET)
			{
				byte[] buffer_plain = Rijndael.decrypt(
											packet, 
											Macro.SIZE_HEADER + Macro.SIZE_ID + 4, 
											size_packet - (Macro.SIZE_HEADER + Macro.SIZE_ID + 4), my_room.secret_key
											);

				if (buffer_plain != null)
				{
					my_room.addLogChatting(
						// message
						Encoding.UTF8.GetString(buffer_plain, 0, buffer_plain.Length)
						);
				}
			}
			else if (my_room.Status == Macro.ROOM_INFO_STATUS_NORMAL)
			{
				my_room.addLogChatting(
					// message
					Encoding.UTF8.GetString(
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
		private void cmdInvite(byte[] packet, int size_packet)
		{
			if (BitConverter.ToUInt16(packet, Macro.SIZE_HEADER) == Macro.CMD_FAIL)
				return;

			string yourid = Encoding.UTF8.GetString(packet, Macro.SIZE_HEADER, Macro.SIZE_ID);
			int room_number = BitConverter.ToInt32(packet, Macro.SIZE_HEADER + Macro.SIZE_ID);

			// 내가 보낸 초대 메세지 이거나,
			// 내가 이미 방에 속해있을때 return
			if (yourid == UserData.ud.id
				|| UserData.ud.findMyRoom(room_number) != null)
				return;

			Window_invited wnd = new Window_invited(yourid, room_number);
			wnd.Left = WindowRoomList.wnd.Left + WindowRoomList.wnd.Width / 2 - wnd.Width / 2;
			wnd.Top = WindowRoomList.wnd.Top + WindowRoomList.wnd.Height / 2 - wnd.Height / 2;
			wnd.Show();
			//wnd.ShowDialog();
		}
		#endregion

	}
}
