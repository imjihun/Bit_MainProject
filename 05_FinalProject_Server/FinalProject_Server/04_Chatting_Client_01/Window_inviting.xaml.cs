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
	/// Window_inviting.xaml에 대한 상호 작용 논리
	/// </summary>
	public partial class Window_inviting : Window
	{
		public Window_inviting(int room_number)
		{
			InitializeComponent();

			this.KeyDown += Window_inviting_KeyDown;

			textBox_yourid.KeyDown += delegate (object sender, KeyEventArgs e)
			{
				if (e.Key != Key.Enter)
					return;

				if (textBox_yourid.Text.Length < 1)
					return;

				MyNetwork.net.sendInvite(textBox_yourid.Text, room_number);
				this.Close();
			};

			button_ok.Click += delegate (object sender, RoutedEventArgs e)
			{
				if (textBox_yourid.Text.Length < 1)
					return;

				MyNetwork.net.sendInvite(textBox_yourid.Text, room_number);
				this.Close();
			};
			button_cancel.Click += Button_cancel_Click;

			this.Closed += Window_inviting_Closed;
			inputFocus();
		}
		public void inputFocus()
		{
			textBox_yourid.Focus();
		}
		private void Window_inviting_Closed(object sender, EventArgs e)
		{
			WindowChatting wnd = this.Owner as WindowChatting;
			if(wnd != null)
			{
				wnd.wnd_inviting = null;
			}
		}

		private void Button_cancel_Click(object sender, RoutedEventArgs e)
		{
			this.Close();
		}

		private void Window_inviting_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.Key == Key.Escape)
				Close();
		}
	}
}
