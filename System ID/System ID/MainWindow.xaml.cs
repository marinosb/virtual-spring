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

namespace System_ID
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
            slider1.ValueChanged += slider1_ValueChanged;
            zeroSlider.ValueChanged += new RoutedPropertyChangedEventHandler<double>(zeroSlider_ValueChanged);

            InitializePortReader();

            this.Closing += new System.ComponentModel.CancelEventHandler(MainWindow_Closing);
        }


        void MainWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            pr.Stop();
        }

        private void InitializePortReader()
        {
            try
            {
                pr = new PortReader();
                pr.NewString += new EventHandler<PortEventArgs>(portReader_NewString);
                Thread t = new Thread(pr.loop);
                //t.IsBackground = true;
                t.Start();
            }
            catch (Exception e)
            {
                MessageBox.Show("Could not connect to Arduino");
                pr = null;
            }

        }

        void portReader_NewString(object sender, PortEventArgs e)
        {
            Dispatcher.Invoke(new Action(() =>
            {
                amplitudeLabel.Content = e.Data;
            }));
        }

        private void slider1_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            Write(string.Format("m{0}", slider1.Value));
        }


        void zeroSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            Write(string.Format("z{0}", zeroSlider.Value));
        }

        private void Write(string data)
        {
            pr.Write(data);
            Console.WriteLine(data);
        }

        private void button1_Click(object sender, RoutedEventArgs e)
        {
            Write("r");
        }
    }
}
