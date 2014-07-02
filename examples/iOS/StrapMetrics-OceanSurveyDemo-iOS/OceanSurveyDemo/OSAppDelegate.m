//
//  OSAppDelegate.m
//
//  Created by Martijn The on 8/13/13.
//  Copyright (c) 2013 Pebble Technology Corp. All rights reserved.
//

#import "OSAppDelegate.h"

#import "OSDataLoggingViewController.h"

#import <PebbleKit/PebbleKit.h>

#import "StrapMetrics.h"


@interface OSAppDelegate () <PBPebbleCentralDelegate>
@end

@implementation OSAppDelegate {
  OSDataLoggingViewController *_dataLoggingViewController;
}

PBWatch *connectedWatch;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
  [PBPebbleCentral setDebugLogsEnabled:YES];
  
  uuid_t myAppUUIDbytes;
  NSUUID *myAppUUID = [[NSUUID alloc] initWithUUIDString:@"0A5399D9-5693-4F3E-B768-9C99B5F5DCEA"];
  [myAppUUID getUUIDBytes:myAppUUIDbytes];
  [[PBPebbleCentral defaultCentral] setAppUUID:[NSData dataWithBytes:myAppUUIDbytes length:16]];
  [[PBPebbleCentral defaultCentral] setDelegate:self];

    // Strap API Setup:
    
    // Developer updates these parameters to fit
    // Make sure you change the app ID!
    NSDictionary * STRAP_PARAMS = @{
                                    @"app_id":@"nxnwoBwsa2379DEJb",
                                    @"resolution":@"144x168",
                                    @"useragent":@"PEBBLE/2.0"
                                    };
    [StrapMetrics strap_init];
    
    // End Strap API Setup

    
    connectedWatch = [[PBPebbleCentral defaultCentral] lastConnectedWatch];
    NSLog(@"Last connected watch: %@", connectedWatch);
    
    [connectedWatch appMessagesLaunch:^(PBWatch *watch, NSError *error) {
        if (!error) {
            NSLog(@"Successfully launched app.");
        }
        else {
            NSLog(@"Error launching app - Error: %@", error);
        }
    }
     ];

    
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
    

  self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
  _dataLoggingViewController = [[OSDataLoggingViewController alloc] initWithNibName:nil bundle:nil];
  self.window.rootViewController = [[UINavigationController alloc] initWithRootViewController:_dataLoggingViewController];
  [self.window makeKeyAndVisible];

  return YES;
}

@end
