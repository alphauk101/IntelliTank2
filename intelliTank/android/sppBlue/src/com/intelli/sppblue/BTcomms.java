package com.intelli.sppblue;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Array;
import java.util.Arrays;
import java.util.Set;
import java.util.UUID;

import android.R.array;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.os.Bundle;
import android.os.Message;


public class BTcomms {
	
	ConnectedThread mConnectedThread;
	
	public BTcomms(BluetoothAdapter mBluetoothAdapter)
	{

		Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();
		// If there are paired devices
		BluetoothDevice deviceHP = null;
		 for (BluetoothDevice device : pairedDevices) {
		        // Add the name and address to an array adapter to show in a ListView
		        //Log.i(">",device.getName());
			 	if(device.getAddress().equals("00:13:12:13:61:77"))// 00:13:12:13:61:77
			 	{
			 		deviceHP = device;
			 	}
		    }
		if(deviceHP != null)
		{
			ConnectThread con = new ConnectThread(deviceHP);
			con.start();
		}else
		{
			sendMessage(MainActivity.ERROR_BLUETOOTH_ADAPTER,"Unable to find the Intelli-tank please ensure you have paired with it.");
		}

	}

	/*
	 * Sends a message to the main activities handler
	 */
	public void sendMessage(int what, String text)
	{
		Message msg = new Message();
		Bundle bun = new Bundle();
		msg.what = what;
		bun.putString(MainActivity.ERROR_MESSAGE_STRING, text);
		msg.setData(bun);
		MainActivity.mHandler.sendMessage(msg);
	}
	
	
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
	                sendMessage(MainActivity.ERROR_BLUETOOTH_ADAPTER, "Unable to connect to Intelli-tank, try restarting the app and tank.");
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


	public void manageConnectedSocket(BluetoothSocket mmSocket)
	{
		mConnectedThread = new ConnectedThread(mmSocket);
		mConnectedThread.start();
		MainActivity.mHandler.sendEmptyMessage(MainActivity.BLUETOOTH_CONNECTED);
	}

	//makes cancelling the socket visible
	public void closeConnection()
	{
		if(mConnectedThread != null)
		{
			mConnectedThread.cancel();
		}
	}
	
	public void sendBytesSPP(byte[] dataToSend)
	{
		if(mConnectedThread != null)
		{
			mConnectedThread.write(dataToSend);
		}
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
	        int index = 0;
	        int timeout = 2000;
	        // Keep listening to the InputStream until an exception occurs
	        while (true) {
	            try {
	                // Read from the InputStream
	                if(mmInStream.available() > 0)
	                {
	                	
	                	byte sData = (byte) mmInStream.read();
	                	Arrays.fill(buffer, (byte) 0x00);
	                	index = 0;
	                	boolean err = false;
	                	if(sData == (byte)0xAA)
		            	{
	                		buffer[index] = sData;//put the FF in our data
	                		timeout = 2000;
	                		index++;
	                		//were in business
	                		do
	                		{
	                			sData = (byte) mmInStream.read();
	                			if((sData != (byte) 0xFF) && (sData != (byte) 0xAA))
	                			{
		                			buffer[index] = sData;
		                			index++;
		                			timeout--;
	                			}else
	                			{
	                				err = true;
	                			}
	                		}while((sData != (byte)0xFE) && (timeout > 0) && (! err));
	                		
	                		if((timeout > 0) && (! err))//we did not time out
	                		{
	                	 		byte[] da = new byte[index-2];
	                			System.arraycopy(buffer, 1, da, 0, (index-2));
	                			// Send the obtained bytes to the UI activity
	    		                Message msg = new Message();
	    		                msg.what = MainActivity.BLUETOOTH_BYTES_AVAILABLE;
	    		                Bundle bun = new Bundle();
	    		                bun.putByteArray(MainActivity.BLUETOOTH_BYTES_AVAIL_KEY,da);
	    		                msg.setData(bun);
	    		                MainActivity.mHandler.sendMessage(msg);
	                		}
		            	}
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
