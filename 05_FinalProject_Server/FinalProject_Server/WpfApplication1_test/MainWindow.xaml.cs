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
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace WpfApplication1_test
{
	/// <summary>
	/// MainWindow.xaml에 대한 상호 작용 논리
	/// </summary>
	public partial class MainWindow : Window
	{
		public DispatcherTimer m_recv_Timer = new DispatcherTimer();
		bool bDownMouse = false;
		int count = 0;
		public MainWindow()
		{
			InitializeComponent();
			this.Loaded += MainWindow_Loaded;

			this.MouseLeftButtonDown += MainWindow_MouseLeftButtonDown;
			m_recv_Timer.Interval = TimeSpan.FromMilliseconds(1);
			m_recv_Timer.Tick += M_recv_Timer_Tick;

			m_recv_Timer.Start();
		}

		private void MainWindow_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
		{
			bDownMouse = true;
			while (bDownMouse)
				;
		}

		private void M_recv_Timer_Tick(object sender, EventArgs e)
		{
			Console.WriteLine(count++);
		}

		private void MainWindow_Loaded(object sender, RoutedEventArgs e)
		{
		}
	}
}
