using UnityEngine;
using UnityEngine.UI;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.IO;
using System.Threading;
using System;

public class TextManager : MonoBehaviour {

    public Text GestureObj;

	// Use this for initialization
	void Start () {
        GestureObj.text = "C";
        Thread doChat = new Thread(SocketListener);
        doChat.Start();
    }
	
	// Update is called once per frame
	void Update () {
		
	}

    void SocketListener() {
        TcpListener server = new TcpListener(IPAddress.Loopback, 9999);
        TcpClient client = default(TcpClient);

        try
        {
            server.Start();
            Console.WriteLine("Server started");
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex.ToString());
        }


        Console.WriteLine("Accepting Client...");
        client = server.AcceptTcpClient();
        Console.WriteLine("Client Accepted.");

        //StreamReader reader = new StreamReader(client.GetStream());
        NetworkStream ns = client.GetStream();
        while (client.Connected)
        {
            byte[] msg = new byte[1024];
            ns.Read(msg, 0, msg.Length);
            //string message = reader.ReadLine(); //If the string is null, the connection has been lost.
            //Console.WriteLine(Convert.ToChar(msg[0]));
            //GestureObj.text = Convert.ToString(Convert.ToString(msg[0]));
            string test = Encoding.ASCII.GetString(msg);
            GestureObj.text = test;


        }
    }
}
