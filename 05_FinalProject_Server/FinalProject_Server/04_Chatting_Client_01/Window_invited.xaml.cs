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
	/// Window_invited.xaml에 대한 상호 작용 논리
	/// </summary>
	public partial class Window_invited : Window
	{
		public Window_invited(string inviteid, int room_number)
		{
			InitializeComponent();
			textBlock_invite.Text = inviteid.TrimEnd('\0') + "이(가) 초대하였습니다.";
			button_ok.Click += delegate (object sender, RoutedEventArgs e)
			{
				MyNetwork.net.sendEnterRoom(room_number);
				Console.WriteLine("\t\troom_number = " + room_number);
				this.Close();
			};
			button_ok.Focus();
		}
	}
}
