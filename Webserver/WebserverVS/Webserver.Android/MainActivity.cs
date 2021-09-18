using Android.App;
using Android.Widget;
using Android.OS;
using System;
using System.Runtime.InteropServices;
using Java.Lang;

namespace Webserver.Android
{
    class RunAsync : AsyncTask
    {
        SocketServer server;
        protected override Java.Lang.Object DoInBackground(params Java.Lang.Object[] objects)
        {
            server = new SocketServer();
            server.Start(8080, (ip, port) =>
            {
                string res = ip + ':' + port;
            });
            return null;
        }
    }

    class SocketServer : IDisposable
    {
        public delegate void Connection(string ip, int port);

        private IntPtr obj;
        [DllImport("Webserver_Library_Android")]
        private static extern IntPtr Initialize();

        [DllImport("Webserver_Library_Android", CharSet = CharSet.Ansi)]
        private static extern string Start(IntPtr obj, int port, Connection onconection);

        [DllImport("Webserver_Library_Android")]
        private static extern void Cancel(IntPtr obj);

        [DllImport("Webserver_Library_Android")]
        private static extern void Destroy(IntPtr obj);

        public SocketServer()
        {
            obj = Initialize();
        }

        public void Start(int port, Connection onconection)
        {
            var res = Start(obj, port, onconection);
        }

        public void Cancel()
        {
            Cancel(obj);
        }

        public void Dispose()
        {
            Destroy(obj);
        }
    }

    [Activity(Label = "Webserver.Android", MainLauncher = true, Icon = "@drawable/icon")]
    public class MainActivity : Activity
    {
        private RunAsync task;

        //[DllImport("ssl")]
        //static private extern int SSL_write(IntPtr ssl, byte[] data, int len);

        //[DllImport("ssl")]
        //static private extern IntPtr TLS_server_method();

        //[DllImport("ssl")]
        //static private extern IntPtr SSL_CTX_new(IntPtr method);

        protected override void OnCreate(Bundle bundle)
        {
            base.OnCreate(bundle);
            //var meth = TLS_server_method();
            //var sslctx = SSL_CTX_new(meth);
            //int res = SSL_write((IntPtr)2, new byte[3] { 12, 43, 14 }, 3);
            // throw new System.Exception("Hallo Welt:" + sslctx);
            //System.Console.WriteLine(meth.ToString() + ":" + sslctx.ToString());
            task = new RunAsync();
            task.Execute();
            // Set our view from the "main" layout resource
             SetContentView (Resource.Layout.Main);
        }

        protected override void OnDestroy()
        {
            //server.Cancel();
            //server.Dispose();
            //task.Dispose();
            base.OnDestroy();
        }
    }
}

