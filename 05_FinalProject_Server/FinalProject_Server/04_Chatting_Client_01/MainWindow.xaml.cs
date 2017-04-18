using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
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
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace _04_Chatting_Client_01
{
    /// <summary>
    /// MainWindow.xaml에 대한 상호 작용 논리
    /// </summary>
    /// 
    public partial class MainWindow : Window
	{
		public static MainWindow wnd = null;
		public MainWindow()
		{
			if (wnd == null)
				wnd = this;

			InitializeComponent();
			textBox_id.KeyDown += TextBox_id_KeyDown;

			textBox_id.Focus();

			
		}
		
		private void TextBox_id_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.Key != Key.Enter)
				return;

			if (textBox_id.Text.Length < 1 || textBox_id.Text.Length >= Macro.SIZE_ID)
			{
				textBox_id.Text = "";
				return;
			}

			new MyNetwork();
			if (MyNetwork.net == null)
				return;

			MyNetwork.net.sendCreateId(textBox_id.Text);
			textBox_id.IsEnabled = false;
		}
	}
}
