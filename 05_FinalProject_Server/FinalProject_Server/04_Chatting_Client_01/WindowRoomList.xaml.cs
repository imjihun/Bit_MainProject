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

		public WindowRoomList(string id)
		{
			if (wnd == null)
				wnd = this;

			InitializeComponent();
			new UserData(id);
			this.Loaded += WindowRoomList_Loaded;
			this.Closed += WindowRoomList_Closed;
			button_createRoom.Click += Button_createRoom_Click;

			m_Timer.Start();
			MyNetwork.net.sendTotalRoomList();
			MyNetwork.net.sendMyRoomList();
		}

		private void WindowRoomList_Closed(object sender, EventArgs e)
		{
			foreach (var v in UserData.ud.dic_my_rooms)
			{
				if (v.Value.wnd != null)
				{
					v.Value.wnd.Close();
					v.Value.wnd = null;
				}
			}
		}

		private void Button_createRoom_Click(object sender, RoutedEventArgs e)
		{
			WindowCreateRoom wnd = new WindowCreateRoom();
			wnd.ShowDialog();
		}

		private void WindowRoomList_Loaded(object sender, RoutedEventArgs e)
		{
			m_Timer.Interval = TimeSpan.FromSeconds(1);
			m_Timer.Tick += Timer_Tick;
		}


		private void Timer_Tick(object sender, EventArgs e)
		{
			//MyNetwork.net.sendTotalRoomList();
		}


		public Button addButtonTotalList(int room_number, byte status, string subject)
		{
			Button newBtn = new Button();

			newBtn.Content = subject;
			newBtn.Name = "Button_" + room_number;
			newBtn.Height = 50;
			if (status == Macro.ROOM_INFO_STATUS_SECRET)
			{
				newBtn.Background = Brushes.LightPink;
				newBtn.BorderBrush = Brushes.Red;
			}
			else if (status == Macro.ROOM_INFO_STATUS_NORMAL)
			{
				newBtn.Background = null;
				newBtn.BorderBrush = Brushes.YellowGreen;
			}
			newBtn.Margin = new Thickness(0, 5, 0, 0);
			newBtn.Click += delegate (object sender, RoutedEventArgs e)
			{
				if(UserData.ud.findMyRoom(room_number) == null)
					MyNetwork.net.sendEnterRoom(room_number);
				else
					MyNetwork.net.sendViewRoom(room_number);
			};

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

			newBtn.Content = subject;
			newBtn.Name = "Button_" + room_number;

			if (status == Macro.ROOM_INFO_STATUS_SECRET)
			{
				newBtn.Background = Brushes.LightPink;
				newBtn.BorderBrush = Brushes.Red;
			}
			else if(status == Macro.ROOM_INFO_STATUS_NORMAL)
			{
				newBtn.Background = null;
				newBtn.BorderBrush = Brushes.YellowGreen;
			}
			

			newBtn.Margin = new Thickness(0, 0, 0, 0);
			newBtn.Click += delegate (object sender, RoutedEventArgs e)
			{
				MyNetwork.net.sendViewRoom(room_number);
			};

			newGrid.Children.Add(newBtn);

			Button closeBtn = new Button();

			closeBtn.Content = "X";
			closeBtn.Background = Brushes.White;
			closeBtn.BorderBrush = Brushes.PaleVioletRed;
			//closeBtn.Height = 50;
			closeBtn.Margin = new Thickness(250, 5, 5, 25);
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
