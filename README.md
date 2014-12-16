![alt text](https://s3.amazonaws.com/strap-assets/strap-metrics.png "Strap Metrics Logo")

Strap Metrics is a real-time wearable analytics platform for developers. This repository contains the Strap Metrics SDK for Pebble. Strap Metrics is currently in beta, and you'll need an account on the dashboard to use this SDK. Signup today at http://www.straphq.com/register.


##Strap Metrics Pebble SDK Quick Start Guide


We now support PebbleKit JS (below), <a href="https://github.com/strap/strap_sdk_pebble/tree/master/pebblejs">Pebble.js</a>, and companion apps for <a href="https://github.com/strap/strap_sdk_pebble/tree/master/companion/iOS">iOS</a> and <a href="https://github.com/strap/strap_sdk_pebble/tree/master/companion/Android">Android</a>. Strap Metrics utilizes AppMessage to communicate between the watch and the connected phone, and tries to be smart about how often it sends data in order to preserve battery life.

Getting started with the Strap Metrics SDK is pretty straightforward. These steps shouldn't take more than 5 minutes.

** Note: The below shows you how to integrate with a JS project. If you have an iOS or Android companion app, check out the README in the companion directory. The rest of the steps below still apply.

---
1. Login to the <a href="http://www.straphq.com/login">Strap Dashboard</a> and create an app. You'll need your App ID handy for the next step.
2. Add the Strap Metrics SDK to your Pebble project. Run this from the src directory:

   ```
   curl pebble-install-v2.straphq.com.s3-website-us-east-1.amazonaws.com | sh
   ```

3. Navigate to the js folder in your Pebble project and download the Strap Metrics JavaScript module using either of the following commands:
	```
	curl https://raw.githubusercontent.com/strap/strap_sdk_pebble/master/pebblejs/src/js/strap-metrics.js > strap-metrics.js
	curl https://raw.githubusercontent.com/strap/strap_sdk_pebble/master/pebblejs/src/js/strap-metrics.min.js > strap-metrics.js
	```
  * Paste the following Strap Metrics AppMessage handler in your AppMessage event listener. Be sure to insert your App ID in this step! **Important: If you already have an appmessage event listener, you don't need to register it again. Just take the code inside of this function and add it to your existing event listener.**
  ```
    Pebble.addEventListener("appmessage",
        function(e) {
            // Strap API: Developer updates these parameters to fit
            var strap_params = {
                // *** change the app id! *** //
                app_id: "xyzabc123abcxyz",
                resolution: "144x168",
                useragent: "PEBBLE/2.0"
            };

            // -------------------------
            //  Strap API inclusion in appmessage
            //  This allows Strap to process Strap-related messages
            //  DO NOT EDIT
            // -------------------------
            if(strap_api_can_handle_msg(e.payload)) {
                clearTimeout(strap_api_timer_send);
                var params = strap_api_clone(strap_params);
                strap_api_log(e.payload, 200, params);
                strap_api_timer_send = setTimeout(function() {
                    strap_api_log({}, 0, params);
                }, 10 * 1000);
            }
            // -------------------------

        }
    );
    ```

4. Include strap.h in any of your source that contains actions you want to track in Strap. At a minimum, you must include the Strap header and call the init functions below for Strap to track anything. Make sure you correct the path relative to your source.

    `# include "strap/strap.h"`

5. Initialize Strap in your Pebble code

    In a typical Pebble pattern, your main() calls an init() and deinit() function. Here, you'll need to include the strap_init() and strap_deinit() functions, respectively. **Important: Your code must call app_message_open for Strap to communicate!**
    ```
    static void init(void) {

      window = window_create();
      window_set_click_config_provider(window, click_config_provider);
      window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
      });
      const bool animated = true;
      window_stack_push(window, animated);


      int in_size = app_message_inbox_size_maximum();
      int out_size = app_message_outbox_size_maximum();
      app_message_open(in_size, out_size);

      // initialize strap
      strap_init();
    }

    static void deinit(void) {
      // unload strap
      strap_deinit();

      window_destroy(window);
    }

    int main(void) {
      init();
      app_event_loop();
      deinit();
    }
    ```

6. Start tracking events!
    ```
    static void select_button_click_handler(ClickRecognizerRef recognizer, void *context) {
      // do something on your Pebble
      strap_log_event("/select");
    }
    ```

![alt text](http://images.memegenerator.net/images/200x/1031.jpg "Success Kid")

Success! You've successfully integrated Strap into your Pebble application. We'll start crunching the numbers as data starts to flow into Strap, and you'll be seeing reports on the dashboard in a few minutes. We have tested Strap in a variety of app configurations, but your feedback is extremely important to us in this beta period! If you have any questions, concerns, or problems with Strap Metrics, please let us know. You can open an issue on GitHub, visit our community support portal at http://strap.uservoice.com, email us at support@straphq.com, or tweet us @getstrap.

##FAQ


   * Who owns the data?
     - You.
   * Are you selling my data?
     - Nope.
   * How will you make money?
     - Enterprise and Pro features.
   * What is the battery impact?
     - Depends. Usually about 10%, but mileage may vary.
   * What is the memory impact?
     - ~3.5kb with accelerometer collection, ~1.8kb without
   * How do I disable accelerometer collection?
     - uncomment ```#define DISABLE_ACCL``` in strap.h

##User Opt-Out
   ![alt text](https://github.com/cheniel/strap-optout-prompt-test/raw/master/screenshots/screenshot.png "Prompt Screenshot")

   We provide an easy way for you to allow your users to opt-out of being tracked. Use this simple function which will push a new window asking the user for permission. Call it at either the end of init() or put it in a click handler using:
   ```
   prompt_opt_out(default_selection);
   ```
   default_selection is a boolean which, when true, automatically highlights "Yes", and, when false, highlights "No". If the user presses the back button on the prompt, Strap metrics are disabled.

   If you would rather create your own interface for prompting the user, just set the following persistent storage variable and we will not report any data. You could set this through your configuration page, or with a simple UI prompt on the Pebble when the user first loads the app.

   ```
   set_persist_bool(STRAP_OPT_OUT,true);
   ```

