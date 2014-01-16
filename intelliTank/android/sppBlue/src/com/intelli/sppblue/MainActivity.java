/*
 * Intelli-Tank 2 app
 * Will recieve data from tank unit in format of TLV
 * to be defined-
 * This is best because I can upgrade and improve on BT transfer without 
 * affecting the normal data transition.
 * 
 * Water temp TLV
 * type = 0x01;
 * length = *
 * value = single unsigned byte (temp should always be around 11 - 35C)
 */


package com.intelli.sppblue;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.content.DialogInterface;
import android.content.Intent;
import android.view.Menu;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity {


	//HANDLER CODES
	public final static int ERROR_BLUETOOTH_ADAPTER = 73594;
	public final static String ERROR_MESSAGE_STRING = "errormsg";
	
	public final static int BLUETOOTH_BYTES_AVAILABLE = 45813;
	public final static String BLUETOOTH_BYTES_AVAIL_KEY = "byteavail";
	
	public final static int BLUETOOTH_CONNECTED = 54849465;
	
	public final static int WATERTEMP_READING = 124565;
	public final static String WATERTEMP_VALUE = "watertemp";
	
	
	//tlv types
	private final byte WATERTEMP_TYPE = 0x01;


	ProgressDialog mProgressDialog;
	BluetoothAdapter mBluetoothAdapter;
	public static Handler mHandler;
	//ConnectedThread ct;
	int REQUEST_ENABLE_BT = 10294;
	BTcomms mBTcomms;
	
	static boolean BLUETOOTH_ON = false;//flag to show bluetooth state
	
	private TextView tvWaterTemp;
	
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		//setup button
		final Button mButton = (Button)findViewById(R.id.btn_update);
		mButton.setOnClickListener(new View.OnClickListener() 
		{	
			@Override
			public void onClick(View v) {
				startBT();				
			}
		});
		mButton.setText("Connect");
		
		
		mHandler = new Handler(){
			@Override
			public void handleMessage(Message msg) 
			{
				switch (msg.what)
				{
				case ERROR_BLUETOOTH_ADAPTER:
						showError(msg.getData().getString(ERROR_MESSAGE_STRING));
						mProgressDialog.hide();
					break;
				case BLUETOOTH_BYTES_AVAILABLE:
					//we have some byte here do whatever with them
					processIncomingData(msg.getData().getByteArray(BLUETOOTH_BYTES_AVAIL_KEY));
					break;
				case BLUETOOTH_CONNECTED:
					mButton.setText("Connected");
					mButton.setEnabled(false);
					Toast.makeText(getApplicationContext(), "Exit App to disconnect.", Toast.LENGTH_LONG).show();
					mProgressDialog.hide();
					break;
				case WATERTEMP_READING:
					int waterTemp = msg.getData().getInt(WATERTEMP_VALUE);
					//now update the display
					String wt = String.valueOf(waterTemp) + " \u2103";
					tvWaterTemp.setText(wt);
					break;
				default:
					break;
				}
			}
		};

		//start the bluetooth actions
		mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
		if (mBluetoothAdapter == null) {
		    //handler
			
		}
		
		if (!mBluetoothAdapter.isEnabled()) {
		    Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
		    startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
		}else
		{
			BLUETOOTH_ON = true;
		}
		
		//setup text inputs for ui
		initTextInputs();
		
		//quick test
		//byte[] test = new byte[]{0x01,0x01,0x14};
		
		//processIncomingData(test);
	}
	
	private void initTextInputs()
	{
		tvWaterTemp = (TextView)findViewById(R.id.edt_waterTemp);
		tvWaterTemp.setText("--C");
		
		//progress spinner
		mProgressDialog = new ProgressDialog(this);
		mProgressDialog.setMessage("Connecting to Intelli-Tank...");
		mProgressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
		
	}
	
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		
		if(requestCode == REQUEST_ENABLE_BT)
		{
			if(resultCode == RESULT_OK)
			{
				//bluetooth is on
				BLUETOOTH_ON = true;
			}
		}
		super.onActivityResult(requestCode, resultCode, data);
	}
	

	private void startBT()
	{
		if(mBluetoothAdapter != null)
		{
			mProgressDialog.show();
			mBTcomms = new BTcomms(mBluetoothAdapter);
		}else
		{
			sendMessage(ERROR_BLUETOOTH_ADAPTER, "Problem with your bluetooth adapter.");
		}
	}
	
	
	@Override
	protected void onPause() {
		if(mBTcomms != null)
		{
			mBTcomms.closeConnection();
			Toast.makeText(getApplicationContext(), "Disconnected from tank", Toast.LENGTH_LONG).show();
		}
		super.onPause();
	}


	public void sendMessage(int what, String text)
	{
		Message msg = new Message();
		Bundle bun = new Bundle();
		msg.what = what;
		bun.putString(ERROR_MESSAGE_STRING, text);
		msg.setData(bun);
		mHandler.sendMessage(msg);
	}
	
	private void showError(String error)
	{
		AlertDialog.Builder adb = new AlertDialog.Builder(this);
		adb.setTitle("There was a problem");
		adb.setMessage(error);
		adb.setPositiveButton("OK", new DialogInterface.OnClickListener() {
			
			@Override
			public void onClick(DialogInterface dialog, int which) {
				// TODO Auto-generated method stub
				
			}
		});
		adb.create().show();
	}
	

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}


	//here we process our incoming data
	//calls the handler with the appropriate action
	private void processIncomingData(byte[] data)
	{
		if((data != null) && (data.length > 0))//due diligence on data
		{
			//get the first byte
			int cursor = 0;
			while(cursor < data.length)
			{
				//go round our data
				byte type = data[cursor];
				cursor++;
				byte length = data[cursor];
				cursor ++;
				//now use the length to get the relevant bytes
				byte[] TLVdata = new byte[length];
				for(int a = 0; a < length; a++)
				{
					TLVdata[a] = data[cursor];
					cursor++;
				}
				//here we should have the type, length, data AND a cursor thats in the right place to process the next lot of data
				//but process this chunk first 
				processChunk(type, TLVdata);
				
			}
		}//else we dont care
	}

	private void processChunk(byte type, byte[] data)
	{
		//first check we have a type#
		switch (type)
		{
		case WATERTEMP_TYPE:
			//we have the water temp reading
			//so the data should be just one byte
			int waterTmp = (int)data[0];
			Message msg = new Message();
			msg.what = WATERTEMP_READING;
			Bundle bun = new Bundle();
			bun.putInt(WATERTEMP_VALUE, waterTmp);
			msg.setData(bun);
			mHandler.sendMessage(msg);
			break;

		default:
			break;
		}
	}
}
