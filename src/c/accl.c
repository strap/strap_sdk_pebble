#include <pebble.h>

#define TupletStaticCString(_key, _cstring, _length) \
((const Tuplet) { .type = TUPLE_CSTRING, .key = _key, .cstring = { .data = _cstring, .length = _length + 1 }})

uint8_t sample_freq = ACCEL_SAMPLING_10HZ;

uint16_t sample_count=0;
uint16_t acc_count=0;
uint16_t ack_count=0;
uint16_t fail_count=0;
int16_t *acc_data;
time_t   acc_time;
uint8_t num_samples = 10; 
static DictionaryIterator dict_iter, *iter = &dict_iter;
char *xyz_str = "X,Y,Z:                      ";
bool waiting_data = false;
bool msg_run = false;

#define timer_interval 100
AppTimer *timer;

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

static char cur_activity[15];

static AccelData accl_data[NUM_SAMPLES];

void request_send_acc(void) {
	
	uint16_t ms;
	time_t now;
	time_ms(&now, &ms);
	char buffer[15];
	snprintf(buffer, sizeof(buffer) - 1, "%lu%03d", now, ms);
	
	// snprintf(xyz_str,22 ,"X,Y,Z: %d,%d,%d",acc_data[0],acc_data[1],acc_data[2] );
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "%s",xyz_str);
	app_message_outbox_begin(&iter);

    long long nowz = now;
    nowz = nowz * 1000 + ms;

    

	Tuplet t = TupletStaticCString(KEY_OFFSET + T_TIME_BASE, buffer, strlen(buffer));
	dict_write_tuplet(iter, &t);
	Tuplet act = TupletStaticCString(KEY_OFFSET + T_ACTIVITY, cur_activity, strlen(cur_activity));
	dict_write_tuplet(iter, &act);
    
    for(int i = 0; i < NUM_SAMPLES; i++) {
    
        int point = KEY_OFFSET + (10 * i);
        
        Tuplet ts = TupletInteger(point + T_TS, (int)(nowz - accl_data[i].timestamp));
        dict_write_tuplet(iter, &ts);
        
        Tuplet x = TupletInteger(point + T_X, accl_data[i].x);
        dict_write_tuplet(iter, &x);

        Tuplet y = TupletInteger(point + T_Y, accl_data[i].y);
        dict_write_tuplet(iter, &y);

        Tuplet z = TupletInteger(point + T_Z, accl_data[i].z);
        dict_write_tuplet(iter, &z);

        Tuplet dv = TupletStaticCString(point + T_DID_VIBRATE, accl_data[i].did_vibrate?"1":"0", 1);
        dict_write_tuplet(iter, &dv);

	}
	
	app_message_outbox_send();
	waiting_data = false;
	msg_run = true;
	acc_count++;
}

void timer_callback (void *data) {
	
	if ((waiting_data) && (msg_run==false))
		request_send_acc(); 
	timer = app_timer_register(timer_interval, timer_callback, NULL);
}
void handle_second_tick(struct tm *tick_time, TimeUnits units_changed)
{
	// Need to be static because they're used by the system later.
	static char count_text[] = "                                                                    ";

	snprintf(count_text,sizeof(count_text) ,"sample:%03d \n   sent:  %03d \n   ack:   %03d \n   faild:  %03d", 
		sample_count, acc_count, ack_count, fail_count);
	if (acc_count %100==0)
		APP_LOG(APP_LOG_LEVEL_INFO, "sample:%03d sent: %03d  ack: %03d  faild: %03d", 
			sample_count, acc_count, ack_count, fail_count);

}
void accl_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send(%3d)! error: 0x%02X ",++fail_count,reason);
	msg_run = false;

}
void accl_out_received_handler(DictionaryIterator *iterator, void *context) {
	//APP_LOG(APP_LOG_LEVEL_INFO, "App Message sent");
	ack_count++;
	msg_run = false;
}
void accel_data_handler(AccelData *data, uint32_t num_samples) {

    for(uint32_t i = 0; i < num_samples; i++) {
        accl_data[i].x = data[i].x;
        accl_data[i].y = data[i].y;
        accl_data[i].z = data[i].z;
        accl_data[i].timestamp = data[i].timestamp;
        accl_data[i].did_vibrate = data[i].did_vibrate;
    }
	acc_time=time(NULL);
	waiting_data = true;

	sample_count++;
}

void accl_init(void) {
	tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
	accel_data_service_subscribe(10, &accel_data_handler);
	accel_service_set_sampling_rate(sample_freq); //This is the place that works
	
	app_message_register_outbox_failed(accl_out_failed_handler);
	app_message_register_outbox_sent(accl_out_received_handler);

    timer = app_timer_register(timer_interval, timer_callback, NULL);
	app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
}

void accl_deinit(void) {
	tick_timer_service_unsubscribe();
	app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);
}
