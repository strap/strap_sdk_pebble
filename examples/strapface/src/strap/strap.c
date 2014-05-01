#include <pebble.h>
#include "strap.h"



#define TupletStaticCString(_key, _cstring, _length) \
((const Tuplet) { .type = TUPLE_CSTRING, .key = _key, .cstring = { .data = _cstring, .length = _length + 1 }})

#define KEY_OFFSET 48000
#define T_TIME_BASE 1000  // string
#define T_TS 1         // ints
#define T_X 2          // ints
#define T_Y 3          // ints
#define T_Z 4          // ints
#define T_DID_VIBRATE 5 // string T/F
#define T_ACTIVITY 2000
#define T_LOG 3000

#define NUM_SAMPLES 10
static AccelData accl_data[NUM_SAMPLES];
static int report_accl = 0;
static char cur_activity[15];
static int retryCount = 0;
static int retryMax = 10;

static void send_accl_data();
static void app_timer_accl_stop(void*);
static void app_timer_accl_start(void*);
static void accl_new_data(AccelData*, uint32_t);
static void log_action(void*);

#ifdef STRAP_DEBUG
static char* translate_error(AppMessageResult);
static void strap_applog(char*);

static char* translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

static void strap_applog(char* message)
{
    app_log(APP_LOG_LEVEL_INFO, "strap.c", 0, message);
}
#endif

static void send_accl_data()
{
    bool haveBT = bluetooth_connection_service_peek();
    if (!haveBT) {
        app_log(APP_LOG_LEVEL_INFO, "C", 0, "ERROR No Bluetooth connection!");
    }

    if(!report_accl || !haveBT)  
        return;
        
    if(!bluetooth_connection_service_peek()) {
        // if bluetooth connection is down, retry in 30 seconds
        app_timer_register(1 * 30 * 1000, app_timer_accl_start,NULL);
        return;
    }

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    
    uint16_t ms;
    time_t now;
    time_ms(&now, &ms);
    char buffer[15];
    snprintf(buffer, sizeof(buffer) - 1, "%lu%03d", now, ms);

    Tuplet t = TupletStaticCString(KEY_OFFSET + T_TIME_BASE, buffer, strlen(buffer));
    dict_write_tuplet(iter, &t);
       
    Tuplet act = TupletStaticCString(KEY_OFFSET + T_ACTIVITY, cur_activity, strlen(cur_activity));
    if(dict_write_tuplet(iter, &act) != DICT_OK) {
        app_log(APP_LOG_LEVEL_INFO, "C", 0, "ERROR Writing to dict.");
    }
    
   
    long long nowz = now;
    nowz = nowz * 1000 + ms;
   
    
    for(int i = 0; i < NUM_SAMPLES; i++) {
        int point = KEY_OFFSET + (10 * i);
        
        Tuplet ts = TupletInteger(point + T_TS, (int)(nowz - accl_data[i].timestamp));
        if(dict_write_tuplet(iter, &ts) != DICT_OK) {
            app_log(APP_LOG_LEVEL_INFO, "C", 0, "ERROR Writing to dict.");
        }

        Tuplet x = TupletInteger(point + T_X, accl_data[i].x);
        if(dict_write_tuplet(iter, &x) != DICT_OK) {
            app_log(APP_LOG_LEVEL_INFO, "C", 0, "ERROR Writing to dict.");
        }

        Tuplet y = TupletInteger(point + T_Y, accl_data[i].y);
        if(dict_write_tuplet(iter, &y) != DICT_OK) {
            app_log(APP_LOG_LEVEL_INFO, "C", 0, "ERROR Writing to dict.");
        }
        
        Tuplet z = TupletInteger(point + T_Z, accl_data[i].z);
        if(dict_write_tuplet(iter, &z) != DICT_OK) {
            app_log(APP_LOG_LEVEL_INFO, "C", 0, "ERROR Writing to dict.");
        }
        
        Tuplet dv = TupletStaticCString(point + T_DID_VIBRATE, accl_data[i].did_vibrate?"1":"0", 1);
        if(dict_write_tuplet(iter, &dv) != DICT_OK) {
            app_log(APP_LOG_LEVEL_INFO, "C", 0, "ERROR Writing to dict.");
        }
    }
    
    dict_write_end(iter);
    
    app_message_outbox_send();
}

static void app_timer_accl_stop(void* data) {
    // set report flag to false to indicate we want to pause reporting accl data
    report_accl = 0;
    
    // set timer that will start reporting accl data after two minutes
    app_timer_register(2 *60 * 1000, app_timer_accl_start,NULL);
}

static void app_timer_accl_start(void* data) {
    // set report flag to true to indicate we want to report accl data

    report_accl = 1;
    send_accl_data();
    
    // set timer that will stop reporting accl data after one minute
    app_timer_register(1 * 60 * 1000, app_timer_accl_stop,NULL);
}

static void accl_new_data(AccelData *data, uint32_t num_samples) {
    for(uint32_t i = 0; i < num_samples; i++) {
        accl_data[i].x = data[i].x;
        accl_data[i].y = data[i].y;
        accl_data[i].z = data[i].z;
        accl_data[i].timestamp = data[i].timestamp;
        accl_data[i].did_vibrate = data[i].did_vibrate;
    }
}

void strap_out_sent_handler(DictionaryIterator *iter, void *context)
{
    send_accl_data();
}

void strap_out_failed_handler(DictionaryIterator *iter, AppMessageResult result, void *context)
{
    // if a lot of failures start happening, we may have lost network 
    // or connection to the Pebble app on the phone
    // in this case, we need to stop accl from reporting
    // otherwise the pebble will reset because we'll thrash the memory

    retryCount++;
    if (retryCount == retryMax) {
        report_accl = 0;
        return;
    }
    send_accl_data();
}

void strap_init() {
    memset(cur_activity, 0, sizeof(cur_activity));
    strap_set_activity("UNKNOWN");
    //int in_size = app_message_inbox_size_maximum();
    //int out_size = app_message_outbox_size_maximum();
    app_message_register_outbox_sent(strap_out_sent_handler);
    app_message_register_outbox_failed(strap_out_failed_handler);

    //app_message_open(in_size, out_size);

    accel_data_service_subscribe(NUM_SAMPLES, (AccelDataHandler)accl_new_data);
    accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);

    // start sending accl data in 30 seconds
    app_timer_register(30 * 1000, app_timer_accl_start,NULL);
    strap_log_action("STRAP_START");
}

void strap_deinit() {
    accel_data_service_unsubscribe();
    strap_log_action("STRAP_FINISH");
}

void strap_log_action(char* path) {
    app_timer_register(1 * 1000, log_action, path);
}

static void log_action(void* vpath) {
    
    if(!bluetooth_connection_service_peek()) {
        // if bluetooth connection is down, drop log_visit
        return;
    }
    
    char* path = (char*)vpath;
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    Tuplet t = TupletStaticCString(KEY_OFFSET + T_LOG, path, strlen(path));
    
    if(dict_write_tuplet(iter, &t) == DICT_OK) {
        dict_write_end(iter);
        app_message_outbox_send();
    }
}

void strap_set_activity(char* act) {
    strncpy(cur_activity, act, sizeof(cur_activity) - 1);
}
