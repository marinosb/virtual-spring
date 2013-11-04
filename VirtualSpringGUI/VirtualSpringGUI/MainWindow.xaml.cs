using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading;

namespace VirtualSpringGUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private PortReader pr;
        public MainWindow()
        {
            InitializeComponent();
        }

        void pr_NewString(object sender, PortEventArgs e)
        {
            
            Dispatcher.BeginInvoke(new Action(() => {
                this.positionLabel.Content = e.Data;
            }));
        }


        private void start_toggled(object sender, RoutedEventArgs e)
        {
            if (this.startButton.IsChecked == true)
            {
                pr = new PortReader();
                pr.NewString += new EventHandler<PortEventArgs>(pr_NewString);
                Thread t = new Thread(pr.loop);
                t.Start();
            }
            else
            {
                pr.NewString -= pr_NewString;
                pr.Stop();
            }

        }


    }
}


