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
            this.startButton.IsChecked = true;
        }

        void pr_NewString(object sender, PortEventArgs e)
        {

            Dispatcher.BeginInvoke(new Action(() =>
            {
                this.positionLabel.Content = e.Data;
            }));
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
            else if(pr!=null)
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


        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            this.startButton.IsChecked = false;
        }

        private void slider1_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if(pr!=null) pr.Write(string.Format("v{0}", (int)slider1.Value));
        }

        private void resetOrigin_Clicked(object sender, RoutedEventArgs e)
        {
            if(pr!=null) pr.Write("r\n");
        }

        private void stiffnessSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            
            if (pr != null)
            {
                this.stiffness.Content = stiffnessSlider.Value;
                pr.Write(string.Format("s{0}", (int)stiffnessSlider.Value));
            }
        }

        private void toggleButton1_Click(object sender, RoutedEventArgs e)
        {
            int positionOffset=0;
            if(int.TryParse(this.positionOffsetTextBox.Text, out positionOffset))
            {
                pr.Write(string.Format("p{0}", positionOffset));
                this.positionOffsetTextBox.Text = "";
            }
            else MessageBox.Show("Invalid Position Offset");
        }


    }
}


