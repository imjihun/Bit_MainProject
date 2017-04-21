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
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace _04_Chatting_Client_01
{
	/// <summary>
	/// WindowChatting.xaml에 대한 상호 작용 논리
	/// </summary>
	public partial class WindowChatting : Window
	{
		MyRoom my_room = null;

		public Window_inviting wnd_inviting = null;

		public WindowChatting(MyRoom _my_room)
		{
			InitializeComponent();
			// this.Owner = WindowRoomList.wnd;

			my_room = _my_room;

			textBlock_chat.Text = "";
			textBlock_chat.Text += my_room.Log_chatting.ToString();
			textBox_input.Focus();
			textBox_input.KeyDown += TextBox_input_KeyDown;

			my_room.wnd = this;
			this.Loaded += WindowChatting_Loaded;
			this.Closed += WindowChatting_Closed;
			button_invite.Click += Button_invite_Click;
			this.KeyDown += WindowChatting_KeyDown;
			this.MouseLeftButtonDown += WindowChatting_MouseLeftButtonDown;
			this.PreviewMouseLeftButtonDown += WindowChatting_PreviewMouseLeftButtonDown;
			this.MouseLeftButtonUp += WindowChatting_MouseLeftButtonUp;

			my_room.grd.Children.OfType<Button>().FirstOrDefault().Background = Brushes.White;
			if(my_room.notice != null)
				my_room.notice.Close();
		}
		

		private void WindowChatting_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
		{
			textBox_input.Focus();
		}

		private void WindowChatting_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
		{
			textBox_input.Focus();
		}

		private void WindowChatting_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
		{
			textBox_input.Focus();
		}

		private void WindowChatting_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.Key == Key.Escape)
				this.Close();
		}

		private void Button_invite_Click(object sender, RoutedEventArgs e)
		{
			if (wnd_inviting == null)
			{
				wnd_inviting = new Window_inviting(my_room.room_number);
				wnd_inviting.Left = this.Left + this.Width / 2 - wnd_inviting.Width / 2;
				wnd_inviting.Top = this.Top + this.Height / 2 - wnd_inviting.Height / 2;
				wnd_inviting.Owner = this;
				wnd_inviting.Show();
				//wnd.ShowDialog();
			}
			else
			{
				wnd_inviting.inputFocus();
			}
		}

		private void WindowChatting_Loaded(object sender, RoutedEventArgs e)
		{
			// for title update
			my_room.Count_member = my_room.Count_member;

			// secret room background
			if (my_room.Status == Macro.ROOM_INFO_STATUS_SECRET)
			{
				mygrid.Background = Brushes.LightGoldenrodYellow;
				//this.Background = Brushes.Red;
				//mygrid.Margin = new Thickness(1, 1, 1, 1);
			}
		}

		private void WindowChatting_Closed(object sender, EventArgs e)
		{
			my_room.wnd = null;
		}

		private void TextBox_input_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.Key != Key.Enter)
				return;
			if (textBox_input.Text.Length < 1)
				return;

			MyNetwork.net.sendChattingMessage(my_room, textBox_input.Text);
			textBox_input.Text = "";
		}
		
		public void updateChat()
		{
			textBlock_chat.Text = my_room.Log_chatting.ToString();
		}
	}

	public class ScrollViewerExtensions
	{
		public static readonly DependencyProperty AlwaysScrollToEndProperty = DependencyProperty.RegisterAttached("AlwaysScrollToEnd", typeof(bool), typeof(ScrollViewerExtensions), new PropertyMetadata(false, AlwaysScrollToEndChanged));
		private static bool _autoScroll;

		private static void AlwaysScrollToEndChanged(object sender, DependencyPropertyChangedEventArgs e)
		{
			ScrollViewer scroll = sender as ScrollViewer;
			if (scroll != null)
			{
				bool alwaysScrollToEnd = (e.NewValue != null) && (bool)e.NewValue;
				if (alwaysScrollToEnd)
				{
					scroll.ScrollToEnd();
					scroll.ScrollChanged += ScrollChanged;
				}
				else { scroll.ScrollChanged -= ScrollChanged; }
			}
			else { throw new InvalidOperationException("The attached AlwaysScrollToEnd property can only be applied to ScrollViewer instances."); }
		}

		public static bool GetAlwaysScrollToEnd(ScrollViewer scroll)
		{
			if (scroll == null) { throw new ArgumentNullException("scroll"); }
			return (bool)scroll.GetValue(AlwaysScrollToEndProperty);
		}

		public static void SetAlwaysScrollToEnd(ScrollViewer scroll, bool alwaysScrollToEnd)
		{
			if (scroll == null) { throw new ArgumentNullException("scroll"); }
			scroll.SetValue(AlwaysScrollToEndProperty, alwaysScrollToEnd);
		}

		private static void ScrollChanged(object sender, ScrollChangedEventArgs e)
		{
			ScrollViewer scroll = sender as ScrollViewer;
			if (scroll == null) { throw new InvalidOperationException("The attached AlwaysScrollToEnd property can only be applied to ScrollViewer instances."); }

			// User scroll event : set or unset autoscroll mode
			if (e.ExtentHeightChange == 0) { _autoScroll = scroll.VerticalOffset == scroll.ScrollableHeight; }

			// Content scroll event : autoscroll eventually
			if (_autoScroll && e.ExtentHeightChange != 0) { scroll.ScrollToVerticalOffset(scroll.ExtentHeight); }
		}
	}
}
