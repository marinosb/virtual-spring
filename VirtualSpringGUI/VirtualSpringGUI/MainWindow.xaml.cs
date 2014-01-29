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
using System.Threading.Tasks;

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
                //this.positionOffsetTextBox.Text = "";
            }
            else MessageBox.Show("Invalid Position Offset");
        }

        private void dampingSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (pr != null)
            {
                this.dampingValue.Content = dampingSlider.Value;
                pr.Write(string.Format("d{0}", (int)dampingSlider.Value));
            }
        }

        private void coulombSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (pr != null)
            {
                this.coulombValue.Content = coulombSlider.Value;
                pr.Write(string.Format("c{0}", (int)coulombSlider.Value));
            }
        }

        private void PresetClick(object sender, RoutedEventArgs e)
        {
            this.preset400.IsEnabled = false;
            this.preset600.IsEnabled = false;
            if (sender == this.preset400)
            {
                ApplyValues(140, 530);
            }
            else if (sender == this.preset600)
            {
                ApplyValues(96, 575);
            }
        }
        


        private void ApplyValues(int stiffness, int damping)
        {
            new Task(() => {
                Console.WriteLine("Starting stiffness:{0} damping:{1}", stiffness, damping);
                bool stop=false;
                while (!stop)
                {
                    Thread.Sleep(100);
                    Dispatcher.BeginInvoke(new Action(() => {
                        stop = (this.stiffnessSlider.Value == stiffness && this.dampingSlider.Value == damping);
                            

                        this.stiffnessSlider.Value += Sign((int)(stiffness - this.stiffnessSlider.Value));
                        this.dampingSlider.Value += Sign((int)(damping - this.dampingSlider.Value));
                    }));
                    
                }

                Console.WriteLine("Terminating stiffness:{0} damping:{1}", stiffness, damping);
                Dispatcher.BeginInvoke(new Action(() => {
                    this.preset400.IsEnabled = true;
                    this.preset600.IsEnabled = true;
                }));
            }).Start();
            
        }

        private int Sign(int value)
        {
            return value > 0 ? 1 : value < 0 ? -1 : 0;
        }


    }
}


