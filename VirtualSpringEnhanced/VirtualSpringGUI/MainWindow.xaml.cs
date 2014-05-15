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
        Dictionary<Slider, Tuple<Label, string, string>> sliderActions = new Dictionary<Slider, Tuple<Label, string, string>>();

        private PortReader pr;
        public MainWindow()
        {
            InitializeComponent();
            this.startButton.IsChecked = true;
            sliderActions.Add(this.coulombSlider, new Tuple<Label,string, string>(this.coulombLabel,"c{0}", "{0}%"));
            sliderActions.Add(this.kSlider, new Tuple<Label, string, string>(this.kLabel, "s{0}", "{0}"));
            sliderActions.Add(this.c1Slider, new Tuple<Label, string, string>(this.c1Label, "d{0}", "{0}%"));
            sliderActions.Add(this.c2Slider, new Tuple<Label, string, string>(this.c2Label, "q{0}", "{0}%"));
            sliderActions.Add(this.c3Slider, new Tuple<Label, string, string>(this.c3Label, "b{0}", "{0}%"));

            sliderActions.Add(this.overrideSlider, new Tuple<Label, string, string>(this.overrideSliderLabel, "v{0}", "{0}/4096"));
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


        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            this.startButton.IsChecked = false;
        }


        private void resetOrigin_Clicked(object sender, RoutedEventArgs e)
        {
            if (pr != null) pr.Write("r\n");
        }


        private void genericSliderValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            Slider slider = (Slider)sender;
            int newValue = (int)slider.Value;
            Tuple<Label, string, string> actionParameters;
            if(!sliderActions.TryGetValue(slider, out actionParameters))
            {
                return;
            }

            if (pr != null)
            {
                actionParameters.Item1.Content = string.Format(actionParameters.Item3,newValue);
                pr.Write(string.Format(actionParameters.Item2, newValue));
            }
        }

        private void positionButtons_Click(object sender, RoutedEventArgs e)
        {
            int positionOffset = 0;
            if (int.TryParse(this.positionOffsetTextBox.Text, out positionOffset))
            {
                positionOffset = (sender == this.upButton) ? -positionOffset : +positionOffset;
                pr.Write(string.Format("p{0}", positionOffset));
                //this.positionOffsetTextBox.Text = "";
            }
            else MessageBox.Show("Invalid Position Offset");
        }


        


        #region animations
        private void PresetClick(object sender, RoutedEventArgs e)
        {
            SetPresets(false);
            if (sender == this.preset400)
            {
                ApplyValues(140, 530);
            }
            else if (sender == this.preset600)
            {
                ApplyValues(96, 575);
            }
            else if (sender == this.preset1000)
            {
                ApplyValues(50, 575);
            }
            else if (sender == this.preset1600)
            {
                ApplyValues(33, 575);
            }
            else if (sender == this.preset2000)
            {
                ApplyValues(26, 575);
            }
        }

        private void SetPresets(bool enable)
        {
            this.preset600.IsEnabled = enable;
            this.preset400.IsEnabled = enable;
            this.preset1000.IsEnabled = enable;
            this.preset1600.IsEnabled = enable;
            this.preset2000.IsEnabled = enable;
        }


        private void ApplyValues(int stiffness, int damping)
        {
            new Task(() =>
            {
                Console.WriteLine("Starting stiffness:{0} damping:{1}", stiffness, damping);
                bool stop = false;
                while (!stop)
                {
                    Thread.Sleep(100);
                    Dispatcher.BeginInvoke(new Action(() =>
                    {
                        stop = (this.kSlider.Value == stiffness && this.c1Slider.Value == damping);


                        this.kSlider.Value += Sign((int)(stiffness - this.kSlider.Value));
                        this.c1Slider.Value += Sign((int)(damping - this.c1Slider.Value));
                    }));

                }

                Console.WriteLine("Terminating stiffness:{0} damping:{1}", stiffness, damping);
                Dispatcher.BeginInvoke(new Action(() =>
                {
                    SetPresets(true);
                }));
            }).Start();

        }

        private void AnimateSlider(Slider slider, int stopValue, int delay, Action endAction)
        {
            new Task(() =>
            {
                Console.WriteLine("Starting animate slider:{0} to stop value:{1}", slider, stopValue);
                bool stop = false;

                while (!stop)
                {
                    Thread.Sleep(delay);
                    Dispatcher.BeginInvoke(new Action(() =>
                    {
                        stop = (slider.Value == stopValue);

                        slider.Value += Sign((int)(stopValue - slider.Value));
                    }));
                }
                Console.WriteLine("Starting animate slider:{0} to stop value:{1}", slider, stopValue);
                Dispatcher.BeginInvoke(endAction);
            }).Start();
        }

        private int Sign(int value)
        {
            return value > 0 ? 1 : value < 0 ? -1 : 0;
        }

        private volatile bool jumpstarting = false;
        private volatile int initialCoulomb = 0;
        private void jumpstartButton_MouseDown(object sender, MouseButtonEventArgs e)
        {
            jumpstarting = true;
            int coulomb = 0;

            if (!int.TryParse(this.jumpstartTextBox.Text, out coulomb))
            {
                MessageBox.Show("Invalid Jumpstart Value");
                return;
            }

            int initialCoulomb = (int)this.coulombSlider.Value;
            new Task(() =>
            {
                Console.WriteLine("Starting jumpstart with coulomb:{0}", coulomb);
                bool stop = false;
                while (jumpstarting && !stop)
                {
                    Thread.Sleep(100);
                    Dispatcher.BeginInvoke(new Action(() =>
                    {
                        stop = (this.coulombSlider.Value == coulomb);


                        this.coulombSlider.Value += Sign((int)(coulomb - this.coulombSlider.Value));
                    }));

                }

                Console.WriteLine("Reached jumpstart with coulomb:{0}", coulomb);
                while (jumpstarting)
                {
                    Thread.Sleep(100);
                }

                Dispatcher.BeginInvoke(new Action(() => {
                    this.jumpstartButton.IsEnabled = false;
                }));

                Console.WriteLine("Unrolling jumpstart with coulomb:{0}", coulomb);
                stop = false;
                while (!jumpstarting && !stop)
                {
                    Thread.Sleep(100);
                    Dispatcher.BeginInvoke(new Action(() =>
                    {
                        stop = (this.coulombSlider.Value == initialCoulomb);

                        this.coulombSlider.Value += Sign((int)(initialCoulomb - this.coulombSlider.Value));
                    }));

                }

                Dispatcher.BeginInvoke(new Action(() =>
                {
                    this.jumpstartButton.IsEnabled = true;
                }));

                Console.WriteLine("Terminated jumpstart with coulomb:{0}", coulomb);

            }).Start();
        }

        private void jumpstartButton_MouseUp(object sender, MouseButtonEventArgs e)
        {
            jumpstarting = false;
        }

        private void chokeButton_Click(object sender, RoutedEventArgs e)
        {
            this.chokeButton.IsEnabled = false;
            this.coulomb40Preset.IsEnabled = false;
            AnimateSlider(this.coulombSlider, 0, 50, new Action(() => {
                this.chokeButton.IsEnabled = true;
                this.coulomb40Preset.IsEnabled = true;
            }));
        }

        private void coulomb40Preset_Click(object sender, RoutedEventArgs e)
        {
            this.chokeButton.IsEnabled = false;
            this.coulomb40Preset.IsEnabled = false;
            AnimateSlider(this.coulombSlider, 40, 50, new Action(() =>
            {
                this.chokeButton.IsEnabled = true;
                this.coulomb40Preset.IsEnabled = true;
            }));
        }

        #endregion


    }
}


