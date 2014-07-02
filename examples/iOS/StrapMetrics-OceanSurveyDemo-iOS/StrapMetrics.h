/*
Copyright 2014 EnSens, LLC D/B/A Strap

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#import <UIKit/UIKit.h>

@interface StrapMetrics : UIResponder <UIApplicationDelegate,NSURLConnectionDelegate> {
    NSMutableData *responseData;
}

+(void) strap_init;
+(void) strap_api_log: (NSDictionary*)data minReadings:(int)min_readings logParameters:(NSDictionary*)log_parameters serialNo:(NSString*)serial;
+(bool) strap_api_can_handle_msg: (NSDictionary*) data;
+(void) strap_api_cleanup: (NSTimer *) theTimer logParameters:(NSDictionary *) params;
+ (NSDictionary*) strap_api_clone:(NSDictionary*) obj;

@property (strong, nonatomic) UIWindow *window;



@end
