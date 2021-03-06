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
import java.util.Calendar;
import java.util.zip.Inflater;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
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
	
	public final static int ALARM_TIME = 455564;
	public final static String ALARM_VALUES = "alrmval";
	
	public final static int SET_TIME = 574325;
	public final static int GET_ALARM = 582468;
	
	
	//tlv types
	private final byte WATERTEMP_TYPE = 0x01;
	private final byte SET_TIME_TYPE = (byte) 0xF1;
	private final byte GET_ALARM_TYPE = (byte) 0xA1;
	
	
	TextView dg_ON;
	TextView dg_OFF;
	
	AlertDialog ALD_TimeSetter;
	
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
		
		LayoutInflater inflater = (LayoutInflater)getSystemService
			      (Context.LAYOUT_INFLATER_SERVICE);
		
		
		//ALD_TimeSetter.setView(inflater.inflate(R.layout.autolight_lo, null));
		//ALD_TimeSetter.
		
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
		
		dg_ON = (TextView)findViewById(R.id.clk_ON);
		dg_OFF = (TextView)findViewById(R.id.clk_OFF);
		dg_ON.setText("00:00");
		dg_OFF.setText("00:00");
		
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
					float waterTemp = msg.getData().getFloat(WATERTEMP_VALUE);
					//now update the display
					String wt = String.valueOf(waterTemp) + " \u2103";
					tvWaterTemp.setText(wt);
					break;
				case SET_TIME:
					Calendar c = Calendar.getInstance();
					//set up TLV
					byte[] tlv = new byte[4];
					tlv[0] = SET_TIME_TYPE;
					tlv[1] = 0x02;
					tlv[2] = (byte) c.get(Calendar.MINUTE);
					tlv[3] = (byte) c.get(Calendar.HOUR_OF_DAY);
					if(mBTcomms != null)
					{
						mBTcomms.sendBytesSPP(tlv);
					}else
					{
						Toast.makeText(getApplicationContext(), "Not set", Toast.LENGTH_LONG).show();
					}
					break;
				case GET_ALARM:
						byte[] tlv1 = new byte[3];
						tlv1[0] = (byte) 0xF2;
						tlv1[1] = 0x01;
						tlv1[2] = 0x11;
						if(mBTcomms != null)
						{
							mBTcomms.sendBytesSPP(tlv1);
						}else
						{
							Toast.makeText(getApplicationContext(), "Not set", Toast.LENGTH_LONG).show();
						}
					break;
				case ALARM_TIME:
					byte[] alrms = msg.getData().getByteArray(ALARM_VALUES);
					String onTime = String.valueOf(alrms[0]) + ":" + String.valueOf(alrms[1]);
					String offTime = String.valueOf(alrms[2]) + ":" + String.valueOf(alrms[3]);
					dg_ON.setText(onTime);
					dg_OFF.setText(offTime);
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
	public boolean onMenuItemSelected(int featureId, MenuItem item) {
		if(item.getItemId() == R.id.action_time)
		{
			mHandler.sendEmptyMessage(SET_TIME);
		}else if(item.getItemId() == R.id.action_alarm)
		{
			mHandler.sendEmptyMessage(GET_ALARM);
		}
		return super.onMenuItemSelected(featureId, item);
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
		if((data != null) && (data.length > 2))//due diligence on data
		{
			//get the first byte
			int cursor = 0;
			while(cursor < data.length)
			{
				try{
					
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
					
					processChunk(type, TLVdata);
					
				}catch(Exception ex)
				{
					Log.i("INTTEL", "Error parsing TLV");
				}
				//here we should have the type, length, data AND a cursor thats in the right place to process the next lot of data
				//but process this chunk first 
				
				
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
			//int tempInt = ((data[1] << 8) | data[0]);
			
			int d0 = data[0];
			d0 &= 0xFF;
			int d1 = data[1];

			int tempInt = (d1 << 8) | d0;
		
			//tempInt = ((0x00FFFF << 16) & tempInt);
		    float tempD = ((6 * tempInt) + tempInt / 4);
		    tempD = tempD / 100;
			Message msg = new Message();
			msg.what = WATERTEMP_READING;
			Bundle bun = new Bundle();
			bun.putFloat(WATERTEMP_VALUE, tempD);
			msg.setData(bun);
			mHandler.sendMessage(msg);
			break;
		case GET_ALARM_TYPE:
			Message msg1 = new Message();
			msg1.what = ALARM_TIME;
			Bundle bun1 = new Bundle();
			bun1.putByteArray(ALARM_VALUES, data);
			msg1.setData(bun1);
			mHandler.sendMessage(msg1);
			break;
			
		default:
			break;
		}
	}
}
