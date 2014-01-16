package com.intelli.sppblue;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Set;
import java.util.UUID;

import android.os.Build;
import android.os.Bundle;
import android.os.Debug;
import android.os.Handler;
import android.os.Message;
import android.annotation.TargetApi;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.util.Log;
import android.view.Menu;
import android.widget.Toast;

public class MainActivity extends Activity {

	BluetoothAdapter mBluetoothAdapter;

	Handler mHandler;
	ConnectedThread ct;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		
		mHandler = new Handler(){
			@Override
			public void handleMessage(Message msg) 
			{
				String re = msg.getData().getString("st");
				Log.i("BT", re);
				byte[] reply = "Hello from Droid".getBytes();
				ct.write(reply);
				//super.handleMessage(msg);
			}
		};
		
		startBT();
		
		
	}

	
	private void startBT()
	{
		mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
		if (mBluetoothAdapter == null) {
		    Toast.makeText(this, "Nada Bluetooth!", Toast.LENGTH_LONG).show();
		}
		
		Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();
		// If there are paired devices
		BluetoothDevice deviceHP = null;
		 for (BluetoothDevice device : pairedDevices) {
		        // Add the name and address to an array adapter to show in a ListView
		        //Log.i(">",device.getName());
			 	if(device.getAddress().equals("00:80:25:48:01:02"))//address of stollman
			 	{
			 		deviceHP = device;
			 	}
		    }
		
		ConnectThread con = new ConnectThread(deviceHP);
		con.start();
		
	}
	
	
	@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
	private class ConnectThread extends Thread {
		
	    private final BluetoothSocket mmSocket;
	    private final BluetoothDevice mmDevice;
	 
	    public ConnectThread(BluetoothDevice device) {
	        // Use a temporary object that is later assigned to mmSocket,
	        // because mmSocket is final
	        BluetoothSocket tmp = null;
	        mmDevice = device;
	        
	        // Get a BluetoothSocket to connect with the given BluetoothDevice
	        try {
	            // MY_UUID is the app's UUID string, also used by the server code
	        	UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");//SPP UUID
	            tmp = device.createRfcommSocketToServiceRecord(MY_UUID);
	        } catch (IOException e) { }
	        mmSocket = tmp;
	    }
	 
	    public void run() {
	        // Cancel discovery because it will slow down the connection
	        //mBluetoothAdapter.cancelDiscovery();
	 
	        try {
	            // Connect the device through the socket. This will block
	            // until it succeeds or throws an exception
	        	if(mmSocket.isConnected() == false)
	        	{
	        		mmSocket.connect();
	        	}
	        } catch (IOException connectException) {
	            // Unable to connect; close the socket and get out
	            try {
	                mmSocket.close();
	            } catch (IOException closeException) { }
	            return;
	        }
	 
	        // Do work to manage the connection (in a separate thread)
	        manageConnectedSocket(mmSocket);
	    }
	 
	    /** Will cancel an in-progress connection, and close the socket */
	    public void cancel() {
	        try {
	            mmSocket.close();
	        } catch (IOException e) { }
	    }
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}


	public void manageConnectedSocket(BluetoothSocket mmSocket)
	{
		ct = new ConnectedThread(mmSocket);
		ct.run();
	}

	private class ConnectedThread extends Thread {
	    private final BluetoothSocket mmSocket;
	    private final InputStream mmInStream;
	    private final OutputStream mmOutStream;
	 
	    public ConnectedThread(BluetoothSocket socket) {
	        mmSocket = socket;
	        InputStream tmpIn = null;
	        OutputStream tmpOut = null;
	 
	        // Get the input and output streams, using temp objects because
	        // member streams are final
	        try {
	            tmpIn = socket.getInputStream();
	            tmpOut = socket.getOutputStream();
	        } catch (IOException e) { }
	 
	        mmInStream = tmpIn;
	        mmOutStream = tmpOut;
	    }
	 
	    public void run() {
	        byte[] buffer = new byte[1024];  // buffer store for the stream
	        int bytes; // bytes returned from read()
	 
	        // Keep listening to the InputStream until an exception occurs
	        while (true) {
	            try {
	                // Read from the InputStream
	                if(mmInStream.available() > 0)
	                {
		            	bytes = mmInStream.read(buffer);
		            	//mmOutStream.write(0x30);
		            	
		                // Send the obtained bytes to the UI activity
		                String m = new String(buffer);
		                Message msg = new Message();
		                Bundle bun = new Bundle();
		                bun.putString("st", m);
		                msg.setData(bun);
		                mHandler.sendMessage(msg);
	                }
	            } catch (IOException e) {
	                break;
	            }
	        }
	    }
	 
	    /* Call this from the main activity to send data to the remote device */
	    public void write(byte[] bytes) {
	        try {
	            mmOutStream.write(bytes);
	        } catch (IOException e) { }
	    }
	 
	    /* Call this from the main activity to shutdown the connection */
	    public void cancel() {
	        try {
	            mmSocket.close();
	        } catch (IOException e) { }
	    }
	}
	
}
