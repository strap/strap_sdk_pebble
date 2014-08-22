Getting Started with the Strap Metrics Pebble SDK for Android Companion Apps
========================
1. Follow the instructions in the main quick start guide for implementing Strap Metrics into your Pebble C.
2. Include the library StrapMetrics.jar in your Eclipse project.
3. Your AppManifest.xml should declare the following permissions:

		<uses-sdk android:minSdkVersion="9" />
		<uses-permission android:name="android.permission.INTERNET"/>

3. Add the following imports in the class which contains your Pebble logic.
		
		import com.straphq.strap.StrapMetrics;
		import java.util.Properties;
		import java.io.IOException;
		import org.json.JSONException;

4. Paste in the Strap Metrics init code:

		// instantiate StrapMetrics
        // change the appID to match yours!
        
        final StrapMetrics sm = new StrapMetrics("rdjYKgrfeAPeMSjQ4");
        final Properties lp = new Properties();
        lp.put("resolution", "144x168");
        lp.put("useragent", "PEBBLE/2.0");
		
5. Setup an AppMessage data received event handler:

		BroadcastReceiver pebblebr;

		pebblebr = PebbleKit.registerReceivedDataHandler(this, 
		new PebbleKit.PebbleDataReceiver(PEBBLE_APP_UUID) {

            @Override
            public void receiveData(final Context context, final int transactionId, 
            final PebbleDictionary data) {
              
              // comment this line to prevent log overload
              // requires additional import
              
              Log.i(getLocalClassName(), "Received value=" + data.toJsonString());

              // is this a message for strap?
              if (sm.canHandleMsg(data)) {
	              try {
	              	// send strap related data to strap
					sm.processReceiveData(data, 200, lp);
				} catch (JSONException e) {
					e.printStackTrace();
				} catch (IOException e) {
					e.printStackTrace();
				}

	              PebbleKit.sendAckToPebble(getApplicationContext(), transactionId);
	            }
	          }
        });