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
	/// WindowCreateRoom.xaml에 대한 상호 작용 논리
	/// </summary>
	public partial class WindowCreateRoom : Window
	{
		public WindowCreateRoom()
		{
			InitializeComponent();
			textBox_subject.KeyDown += TextBox_subject_KeyDown;
			button_ok.Click += Button_ok_Click;

			textBox_subject.Focus();
		}

		private void TextBox_subject_KeyDown(object sender, KeyEventArgs e)
		{
			if(e.Key == Key.Enter)
			{
				createRoom();
			}
		}

		private void Button_ok_Click(object sender, RoutedEventArgs e)
		{
			createRoom();
		}

		private void createRoom()
		{
			if (textBox_subject.Text.Length < 1)
				return;

			MyNetwork.net.sendCreateRoom(textBox_subject.Text, checkBox_secret.IsChecked.Value);
			this.Close();
		}
	}
}
