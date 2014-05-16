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
using System.IO;

namespace DataCapture
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private List<string> data;
        private List<int> torques;
        private PortReader pr;

        private int maxPosition=0;
        private int minPosition=100;

        bool pendingReset=false;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void start_toggled(object sender, RoutedEventArgs e)
        {
            if (this.startButton.IsChecked == true)
            {
                try
                {
                    InitializePortReader();
                }
                catch (Exception ex)
                {
                    this.startButton.IsChecked = false;
                    MessageBox.Show(ex.ToString(), "Error");
                }
            }
            else if (pr != null)
            {
                pr.NewString -= pr_NewString;
                pr.Stop();
            }

        }

        private void InitializePortReader()
        {
            try
            {
                pr = new PortReader();
                pr.NewString += new EventHandler<PortEventArgs>(pr_NewString);
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

        void pr_NewString(object sender, PortEventArgs e)
        {
            Dispatcher.BeginInvoke(new Action(() =>
            {
                this.dataLabel.Content = e.Data;

                string[] cracked = e.Data.Split(new char[] { ',' });
                if (cracked.Length != 2)
                    return;
                int position;
                int torque;
                
                if(!(int.TryParse(cracked[0], out position) && int.TryParse(cracked[1], out torque)))
                {
                    return;
                }

                this.positionLabel.Content = cracked[0];
                this.torqueLabel.Content = cracked[1];

                UpdateSlider(position);

                if (pendingReset && Math.Abs(position) < 10)
                {
                    maxPosition = 0;
                    minPosition = 0;
                    UpdateSlider(0);
                    pendingReset = false;
                }

                if (this.data != null)
                {
                    this.data.Add(e.Data);
                    torques.Add(torque);
                    this.meanTorqueLabel.Content = torques.Average();
                }
            }));
        }

        private void UpdateSlider(int position)
        {
            position = -position;
            maxPosition = Math.Max(maxPosition, position + 100);
            minPosition = Math.Min(minPosition, position - 100);

            if (this.positionSlider.Maximum != maxPosition)
                this.positionSlider.Maximum = maxPosition;

            if (this.positionSlider.Minimum != minPosition)
                this.positionSlider.Minimum = minPosition;

            this.positionSlider.Value = position;
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            this.startButton.IsChecked = false;
        }

        private void button1_Click(object sender, RoutedEventArgs e)
        {

        }

        private void recordButton_Checked(object sender, RoutedEventArgs e)
        {
            this.data = new List<string>();
            this.torques = new List<int>();
        }

        private void recordButton_Unchecked(object sender, RoutedEventArgs e)
        {
            IEnumerable<string> dataCopy=this.data;
            this.data=null;
            this.torques = null;
            this.meanTorqueLabel.Content = "";
            string filename=string.Format(@"C:\Users\MREL-USER\Documents\virtual_spring\data\{0:yyyyMMdd-HHmmss}.csv", DateTime.Now);
            File.WriteAllLines(filename, dataCopy);
        }

        private void resetButton_Click(object sender, RoutedEventArgs e)
        {
            pendingReset = true;
            if (pr != null) pr.Write("r\n");
        }

        private void record60Button_Click(object sender, RoutedEventArgs e)
        {

            Thread t = new Thread(() => {
                Dispatcher.BeginInvoke(new Action(() => {
                    this.record60Button.IsEnabled = false;

                    this.recordButton.IsEnabled = false;
                    this.recordButton.IsChecked = true;
                }));

                Thread.Sleep(60000);
                Dispatcher.BeginInvoke(new Action(() =>
                {
                    this.recordButton.IsChecked = false;

                    this.recordButton.IsEnabled = true;
                    this.record60Button.IsEnabled = true;
                }));
            
            });
            t.Start();
            
        }
    }
}
