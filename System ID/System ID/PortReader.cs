using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO.Ports;
using System.Threading;

namespace System_ID
{
    class PortReader
    {
        private long stop = 0;
        SerialPort port;

        public PortReader()
        {
            port = new SerialPort("COM9", 115200);
            port.Open();
        }

        public void loop()
        {
            
            while (true)
            {
                String s=port.ReadLine();
                Console.WriteLine("new data: {0}", s);
                OnNewString(s);
                //port.Write(s+'\n');

                if (Interlocked.Read(ref stop) == 1)
                    break;
            }
            port.Close();
        }

        public event EventHandler<PortEventArgs> NewString;

        private void OnNewString(String s)
        {
            if(NewString!=null)
                NewString(this, new PortEventArgs(s));
        }

        public void Stop()
        {
            stop = 1;
        }

        public void Write(string data)
        {
            if(port.IsOpen)
                port.WriteLine(data);
        }

    }

    public class PortEventArgs : EventArgs
    {
        public PortEventArgs(String data)
        {
            this.Data = data;
        }
        public String Data { get; private set; }
    }
 }