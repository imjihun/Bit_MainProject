using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace _04_Chatting_Client_01
{
	/// <summary>
	/// WindowRoomList.xaml에 대한 상호 작용 논리
	/// </summary>
	public partial class WindowRoomList : Window
	{
		public static WindowRoomList wnd = null;

		public DispatcherTimer m_Timer = new DispatcherTimer();
		public Window_createRoom wnd_create_room = null;
		public WindowRoomList(string id)
		{
			if (wnd == null)
				wnd = this;

			InitializeComponent();
			new UserData(id);
			button_createRoom.Click += Button_createRoom_Click;

			m_Timer.Interval = TimeSpan.FromSeconds(1);
			m_Timer.Tick += Timer_Tick;
			m_Timer.Start();
			MyNetwork.net.sendTotalRoomList();
			MyNetwork.net.sendMyRoomList();

			this.KeyDown += WindowRoomList_KeyDown;
			this.Closed += WindowRoomList_Closed;

			this.Left = SystemParameters.PrimaryScreenWidth - this.Width;
			this.Top = SystemParameters.PrimaryScreenHeight - this.Height;
			this.Title = UserData.ud.id;
			//this.Topmost = true;
			//this.WindowState = WindowState.Minimized;
		}

		private void WindowRoomList_Closed(object sender, EventArgs e)
		{
			MyRoom tmp;
			for(int i=0; i < UserData.ud.dic_my_rooms.Count; i++)
			{
				tmp = UserData.ud.dic_my_rooms.Values.ToArray()[i];
				if(tmp != null && tmp.wnd != null)
				{
					tmp.wnd.Close();
					tmp.wnd = null;
				}
			}
		}

		private void WindowRoomList_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.Key == Key.Escape)
			{
				this.WindowState = WindowState.Minimized;
				e.Handled = true;
			}
		}

		private void Button_createRoom_Click(object sender, RoutedEventArgs e)
		{
			if (wnd_create_room == null)
			{
				wnd_create_room = new Window_createRoom();
				wnd_create_room.Left = this.Left + this.Width / 2 - wnd_create_room.Width / 2;
				wnd_create_room.Top = this.Top + this.Height / 2 - wnd_create_room.Height / 2;
				wnd_create_room.Show();
				//wnd.ShowDialog();
			}
			else
			{
				wnd_create_room.inputFocus();
			}
		}


		private void Timer_Tick(object sender, EventArgs e)
		{
			MyNetwork.net.sendTotalRoomList();
			foreach (var v in UserData.ud.dic_my_rooms)
				MyNetwork.net.sendEnterRoom(v.Value.room_number);
		}


		public Button addButtonTotalList(int room_number, byte status, string subject)
		{
			Button newBtn = new Button();

			newBtn.Content = "[" + room_number + "] " + subject;
			newBtn.Name = "Button_" + room_number;
			newBtn.Height = 50;
			newBtn.Background = Brushes.White;
			if (status == Macro.ROOM_INFO_STATUS_SECRET)
			{
				newBtn.BorderBrush = Brushes.Red;
			}
			else if (status == Macro.ROOM_INFO_STATUS_NORMAL)
			{
				newBtn.BorderBrush = Brushes.YellowGreen;
			}
			newBtn.Margin = new Thickness(0, 5, 0, 0);
			newBtn.Click += delegate (object sender, RoutedEventArgs e)
			{
				if (UserData.ud.findMyRoom(room_number) == null)
				{
					MyNetwork.net.sendEnterRoom(room_number);
				}
				else
					MyNetwork.net.sendViewRoom(room_number);
			};
			newBtn.HorizontalContentAlignment = HorizontalAlignment.Left;

			stackPanel_totallist.Children.Add(newBtn);

			return newBtn;
		}
		public void delButtonTotalList(Button btn)
		{
			stackPanel_totallist.Children.Remove(btn);
		}

		public Grid addGridMyList(int room_number, byte status, string subject, byte[] key, string chat)
		{
			Grid newGrid = new Grid();
			newGrid.Height = 50;
			newGrid.Margin = new Thickness(0, 5, 0, 0);

			Button newBtn = new Button();

			newBtn.Content = "[" + room_number + "] " + subject + "\n" + chat;
			newBtn.Name = "Button_" + room_number;

			newBtn.Background = Brushes.White;
			if (status == Macro.ROOM_INFO_STATUS_SECRET)
			{
				newBtn.BorderBrush = Brushes.Red;
			}
			else if(status == Macro.ROOM_INFO_STATUS_NORMAL)
			{
				newBtn.BorderBrush = Brushes.YellowGreen;
			}
			

			newBtn.Margin = new Thickness(0, 0, 0, 0);
			newBtn.Click += delegate (object sender, RoutedEventArgs e)
			{
				MyNetwork.net.sendViewRoom(room_number);
			};
			newBtn.HorizontalContentAlignment = HorizontalAlignment.Left;
			newGrid.Children.Add(newBtn);

			Button closeBtn = new Button();

			closeBtn.Content = "X";
			closeBtn.Background = Brushes.White;
			closeBtn.BorderBrush = Brushes.Red;
			closeBtn.Height = 20;
			closeBtn.Width = 20;
			closeBtn.Margin = new Thickness(0, 5, 5, 0);
			closeBtn.VerticalAlignment = VerticalAlignment.Top;
			closeBtn.HorizontalAlignment = HorizontalAlignment.Right;
			closeBtn.Click += delegate (object sender, RoutedEventArgs e)
			{
				MyNetwork.net.sendLeaveRoom(room_number);
			};

			newGrid.Children.Add(closeBtn);

			stackPanel_mylist.Children.Add(newGrid);

			return newGrid;
		}
		public void delButtonMyList(Grid grd)
		{
			stackPanel_mylist.Children.Remove(grd);
		}
	}
}
