Getting Started with the StrapMetrics Pebble SDK for iOS Companion Apps
========================
1. Follow the instructions in the main quick start guide for implementing StrapMetrics into your Pebble C.
2. Include libStrapMetrics.a in your Xcode project.
3. Add StrapMetrics.h to your Xcode project.
4. Import StrapMetrics.h in the implementation which contains your Pebble logic.
5. Paste in the StrapMetrics init code:

		// Strap API Setup:

		// Developer updates these parameters to fit
		// Make sure you change the app ID!
		NSDictionary * STRAP_PARAMS = @{
		                        @"app_id":@"abc123xyz",
		                        @"resolution":@"144x168",
		                        @"useragent":@"PEBBLE/2.0"
		                        };
		[StrapMetrics strap_init];

		// End Strap API Setup
		
6. Setup an AppMessage event handler:

		PBWatch *connectedWatch = [[PBPebbleCentral defaultCentral] lastConnectedWatch];

		[connectedWatch appMessagesAddReceiveUpdateHandler:^BOOL(PBWatch *watch, NSDictionary *update) {
		    NSLog(@"Received message: %@", update);
		    
		    // -------------------------
		    //  Strap API inclusion in appmessage
		    //  This allows Strap to process Strap-related messages
		    //  DO NOT EDIT
		    // -------------------------
		    
		    if([StrapMetrics strap_api_can_handle_msg:update]) {
		        [StrapMetrics strap_api_log:update minReadings:60 logParameters:STRAP_PARAMS serialNo:watch.serialNumber];
		    }
		    
		    return YES;
		}];