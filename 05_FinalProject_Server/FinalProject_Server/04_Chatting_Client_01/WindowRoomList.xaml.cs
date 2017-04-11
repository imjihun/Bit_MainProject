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

			m_Timer.Start();
			MyNetwork.net.sendTotalRoomList();
			MyNetwork.net.sendMyRoomList();
		}

		private void WindowRoomList_Loaded(object sender, RoutedEventArgs e)
		{
			m_Timer.Interval = TimeSpan.FromSeconds(1);
			m_Timer.Tick += Timer_Tick;
		}


		private void Timer_Tick(object sender, EventArgs e)
		{
			//sendTotalRoomList();
		}


		public Button addButtonTotalList(int room_number, string subject)
		{
			Button newBtn = new Button();

			newBtn.Content = subject;
			newBtn.Name = "Button_" + room_number;
			newBtn.Background = Brushes.White;
			newBtn.BorderBrush = Brushes.YellowGreen;
			newBtn.Height = 50;
			newBtn.Margin = new Thickness(0, 5, 0, 0);
			newBtn.Click += delegate (object sender, RoutedEventArgs e)
			{
				MyNetwork.net.sendEnterRoom(room_number);
			};

			stackPanel_totallist.Children.Add(newBtn);

			return newBtn;
		}
		public void delButtonTotalList(Button btn)
		{
			stackPanel_totallist.Children.Remove(btn);
		}

		public Button addButtonMyList(int room_number, byte status, string subject, byte[] key, string chat)
		{
			Button newBtn = new Button();

			newBtn.Content = subject;
			newBtn.Name = "Button_" + room_number;
			newBtn.Background = Brushes.White;
			newBtn.BorderBrush = Brushes.YellowGreen;
			newBtn.Height = 50;
			newBtn.Margin = new Thickness(0, 5, 0, 0);
			newBtn.Click += delegate (object sender, RoutedEventArgs e)
			{
				MyNetwork.net.sendViewRoom(room_number);
			};

			stackPanel_mylist.Children.Add(newBtn);

			return newBtn;
			//Button closeBtn = new Button();

			//closeBtn.Content = subject;
			//closeBtn.Name = "Button_" + room_number;
			//closeBtn.Background = Brushes.White;
			//closeBtn.BorderBrush = Brushes.YellowGreen;
			//closeBtn.Height = 50;
			//closeBtn.Margin = new Thickness(0, 5, 0, 0);
			//closeBtn.Click += delegate (object sender, RoutedEventArgs e)
			//{
			//	MyNetwork.net.sendViewRoom(room_number);
			//};

			//stackPanel_mylist.Children.Add(closeBtn);
		}
		public void delButtonMyList(Button btn)
		{
			stackPanel_mylist.Children.Remove(btn);
		}
	}
}
