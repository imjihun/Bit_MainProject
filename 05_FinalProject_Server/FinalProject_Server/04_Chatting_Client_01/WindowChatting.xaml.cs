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

		public WindowChatting(MyRoom _my_room)
		{
			InitializeComponent();
			my_room = _my_room;
			textBlock_chat.Text = my_room.log_chatting.ToString();

			textBox_input.Focus();
			textBox_input.KeyDown += TextBox_input_KeyDown;

			my_room.wnd = this;
			this.Closed += WindowChatting_Closed;
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

			MyNetwork.net.sendChattingMessage(my_room.room_number, textBox_input.Text);
			textBox_input.Text = "";
		}
		
		public void updateChat()
		{
			textBlock_chat.Text = my_room.log_chatting.ToString();
		}
	}
}
