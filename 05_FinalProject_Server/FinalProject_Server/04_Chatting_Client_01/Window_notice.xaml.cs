using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace _04_Chatting_Client_01
{
	/// <summary>
	/// Window_notice.xaml에 대한 상호 작용 논리
	/// </summary>
	public partial class Window_notice : Window
	{
		public DispatcherTimer delay_timer = new DispatcherTimer();
		public MyRoom room = null;
		public Window_notice(MyRoom r)
		{
			InitializeComponent();
			//this.Owner = WindowRoomList.wnd;

			this.Loaded += Window_notice_Loaded;
			this.MouseLeftButtonDown += Window_notice_MouseLeftButtonDown;
			this.Closed += Window_notice_Closed;
			room = r;

			delay_timer.Interval = TimeSpan.FromSeconds(2);
			delay_timer.Tick += Delay_timer_Tick;

			delay_timer.Start();
			textBlock_roomNumber.Text = r.room_number.ToString();
			textBlock_message.Text = r.Chatting_last_line.Substring(22);
		}

		private void Window_notice_Closed(object sender, EventArgs e)
		{
			room.notice = null;
		}

		private void Window_notice_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
		{
			this.Close();
			//this.DragMove();
		}

		private void Window_notice_Loaded(object sender, RoutedEventArgs e)
		{
			// 창의위치를 셋팅 (우측하단에 올라왔다 내려가게 할 예정)
			if (WindowRoomList.wnd.WindowState == WindowState.Minimized
				)//|| !WindowRoomList.wnd.Topmost)
			{
				this.Left = SystemParameters.PrimaryScreenWidth - this.Width;
				this.Top = SystemParameters.PrimaryScreenHeight - this.Height;
			}
			else
			{
				this.Left = WindowRoomList.wnd.Left + WindowRoomList.wnd.Width - this.Width;
				this.Top = WindowRoomList.wnd.Top + WindowRoomList.wnd.Height;
				//this.Owner = WindowRoomList.wnd;
			}
			this.Activate();
			this.Topmost = true;


			DoubleAnimation topAnim = new DoubleAnimation(this.Top - this.Height, TimeSpan.FromSeconds(1));
			this.BeginAnimation(Window_notice.TopProperty, topAnim);
			DoubleAnimation opaAnim = new DoubleAnimation(1, TimeSpan.FromSeconds(0.5));
			grid_background.BeginAnimation(Grid.OpacityProperty, opaAnim);
		}

		private void Delay_timer_Tick(object sender, EventArgs e)
		{
			Close();
		}
	}
}
