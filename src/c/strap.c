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

#include <pebble.h>
#include "strap.h"
#include "accl.h"

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
static int report_accl = 0;
static char cur_activity[15];


#define LOG_ROWS 30
#define LOG_COLS 50
static char logqueue[LOG_ROWS][LOG_COLS];
static int curLog = 0;
static int curFreq = 1;  // frequency multiplier

static AppTimer* acclStop = NULL;
static AppTimer* acclStart = NULL;
static AppTimer* acclRetry = NULL;
static AppTimer* battTimer = NULL;

static void send_accl_data(void*);
static void send_accl_data_core(void*);
static void app_timer_accl_stop(void*);
static void app_timer_accl_start(void*);
static void accl_new_data(AccelData*, uint32_t);
static void log_action(void*);
static void app_timer_battery(void*);
static void appendLog(char*);
static void send_next_log(void*);
static bool is_accl_available();
static bool is_log_available();

#ifdef DEBUG
static char* translate_error(AppMessageResult);

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

#endif

static void app_timer_accl_stop(void* data) {

    if(acclStart != NULL){
        if(app_timer_reschedule(acclStart, 5000)) {
            app_timer_cancel(acclStart);
        }
        acclStart = NULL;
    }
    
    if(acclStop != NULL){
        acclStop = NULL;
    }

    // set report flag to false to indicate we want to pause reporting accl data
    report_accl = 0;
    accl_deinit();
    
    // set timer that will start reporting accl data after two minutes
    acclStart = app_timer_register(curFreq * 2 * 60 * 1000, app_timer_accl_start,NULL);
}

static void app_timer_accl_start(void* data) {
    
    if(acclStop != NULL){
        if(app_timer_reschedule(acclStop, 5000)) {
            app_timer_cancel(acclStop);
        }
        acclStop = NULL;
    }
    
    if(acclStart != NULL){
        acclStart = NULL;
    }



    accl_init();
    
    // set timer that will stop reporting accl data after about one minute
    acclStop = app_timer_register(80 * 1000, app_timer_accl_stop,NULL);
}

static void app_timer_battery(void* data) {
    if(battTimer != NULL){
        if(app_timer_reschedule(battTimer, 5000)) {
            app_timer_cancel(battTimer);
        }
        battTimer = NULL;
    }

    char buffer[25];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer) - 1, "STRAP_API_BATTERY/%d", battery_state_service_peek().charge_percent);

    strap_log_action(buffer);
    
    battTimer = app_timer_register(curFreq * 5 * 60 * 1000, app_timer_battery,NULL);
}

static bool is_log_available() {
    if(curLog > 0){
        return true;
    }
    return false;
}

void strap_out_sent_handler(DictionaryIterator *iter, void *context)
{
        if(is_log_available()) {
        send_next_log(NULL);
    }
}

void strap_out_failed_handler(DictionaryIterator *iter, AppMessageResult result, void *context)
{
    if(is_log_available()) {
        send_next_log(NULL);
    }
#ifdef DEBUG
    app_log(APP_LOG_LEVEL_INFO, "outfailed", 0, translate_error(result));
#endif
}

void strap_init() {
    memset(cur_activity, 0, sizeof(cur_activity));
    strap_set_activity("UNKNOWN");
    app_message_register_outbox_sent(strap_out_sent_handler);
    app_message_register_outbox_failed(strap_out_failed_handler);

    // start sending accl data in 30 seconds
    #ifndef DISABLE_ACCL
        acclStart = app_timer_register(30 * 1000, app_timer_accl_start,NULL);
    #endif
    battTimer = app_timer_register(1 * 10 * 1000, app_timer_battery,NULL);
    //app_timer_register(30 * 1000,log_timer, NULL);
    app_timer_register(1  * 1000,log_action,"STRAP_START");
}

void strap_deinit() {
    strap_log_action("STRAP_FINISH");
    accl_deinit();
}

// deprecated
void strap_log_action(char* path) {
    log_action(path);
}

void strap_log_event(char* path) {
    log_action(path);
}

static void appendLog(char* path){
    if(curLog == LOG_ROWS){
        // logqueue is full
        return;
    }
    memset(logqueue[curLog],0,LOG_COLS);
    strncpy(logqueue[curLog], path, LOG_COLS - 1);
    curLog++;
}

#ifdef DEBUG
static void plogs() {
    for(int i = 0; i < curLog; i++){
        app_log(APP_LOG_LEVEL_INFO, "log", 0, logqueue[i]);
    }
}
#endif

static void send_next_log(void* data) {
    int tmpLog = curLog;
    curLog = 0;
    char buffer[LOG_COLS];
    
    if(tmpLog > 0){
        strcpy(buffer, logqueue[0]);
        
        for(int i = 1; i < tmpLog; i++){
            strcpy(logqueue[i - 1], logqueue[i]);
            curLog++;
        }
        
        log_action(buffer);
    }
    
}

static void log_action(void* vpath) {
    char* path = (char*)vpath;
    
    if(vpath == NULL){
#ifdef DEBUG
        app_log(APP_LOG_LEVEL_INFO, "vpath", 0, "vpath is NULL");
#endif
        path = "";
    }
    
    if(!bluetooth_connection_service_peek()) {
#ifdef DEBUG
        app_log(APP_LOG_LEVEL_INFO, "btdropmsg", 0, (char*)vpath);
#endif
        return;
    }
    
#ifdef DEBUG
    app_log(APP_LOG_LEVEL_INFO, "action", 0, path);
#endif

    DictionaryIterator *iter;
    AppMessageResult amr = app_message_outbox_begin(&iter);
    if(amr != APP_MSG_OK){
#ifdef DEBUG
        app_log(APP_LOG_LEVEL_INFO, "logdropmsg", 0, translate_error(amr));
        app_log(APP_LOG_LEVEL_INFO, "logdropmsg", 0, path);
#endif
        if(amr == APP_MSG_BUSY){
            appendLog(path);
#ifdef DEBUG
            app_log(APP_LOG_LEVEL_INFO, "appendLog", 0, path);
            plogs();
#endif
        }
        return;
    }
#ifdef DEBUG
    app_log(APP_LOG_LEVEL_INFO, "wasokay", 0, path);
#endif
    Tuplet t = TupletStaticCString(KEY_OFFSET + T_LOG, path, strlen(path));
    
    if(dict_write_tuplet(iter, &t) == DICT_OK) {
        if(dict_write_end(iter) != 0){
            app_message_outbox_send();
#ifdef DEBUG
            app_log(APP_LOG_LEVEL_INFO, "osend", 0, path);
#endif
        }
    }
    else {
#ifdef DEBUG
        app_log(APP_LOG_LEVEL_INFO, "dictbad", 0, path);
#endif
    }
}

void strap_set_activity(char* act) {
    strncpy(cur_activity, act, sizeof(cur_activity) - 1);
}

void strap_set_freq(int freq) {
    curFreq = freq;
}
