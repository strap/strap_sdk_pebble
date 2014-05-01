#include "pebble.h"
#include "strap/strap.h"

// This simple watchface demonstrates how to integrate Strap at the most basic level. 
// Strap will send up all of the important information to calculate user counts, location, duration, etc
// Plus Strap handles the accelerometer data automatically, so user activity (ie jogging, sitting, walking) is calculated on the dashboard automatically.

Window *window;
TextLayer *text_web_layer;
TextLayer *text_era_layer;
TextLayer *text_time_layer;
Layer *line_layer;


BitmapLayer *strap_logo_layer;
static GBitmap *strap_logo;

char timeText[] = "00:00";
char time_text[] = "00:00";
char countText[4];
enum {
  KEY_THEME = 0x0,
  KEY_LABEL = 0x1,
  KEY_MONTH = 0x2,
  KEY_DAY = 0x3,
  KEY_YEAR = 0x4,
  KEY_HOUR = 0x5,
  KEY_MINUTE = 0x6
};


void line_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.
  // static char date_text[] = "Xxxxxxxxx 00";

  char *time_format;


  // TODO: Only update the date when it's changed.
  // strftime(date_text, sizeof(date_text), "%B %e", tick_time);
  // text_layer_set_text(text_date_layer, date_text);


  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  text_layer_set_text(text_time_layer, time_text);

}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  strap_log_action("/up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  strap_log_action("/down");
}
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  strap_log_action("/select");
}


static void click_config_provider(void *context) {
  window_set_click_context(BUTTON_ID_UP, context);
  window_set_click_context(BUTTON_ID_DOWN, context);
  window_set_click_context(BUTTON_ID_SELECT, context);

  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();
  bitmap_layer_destroy(strap_logo_layer);

  // handle deinitialization of Strap
  strap_deinit();

}

void handle_init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, true);
  
  window_stack_push(window, true /* Animated */);


  Layer *window_layer = window_get_root_layer(window);

  window_set_click_config_provider(window ,click_config_provider);

  strap_logo_layer = bitmap_layer_create(GRect(30, -5, 144, 168));
  strap_logo = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STRAP_LOGO);
  bitmap_layer_set_alignment(strap_logo_layer, GAlignLeft);
  layer_add_child(window_layer, bitmap_layer_get_layer(strap_logo_layer));
  bitmap_layer_set_bitmap(strap_logo_layer, strap_logo); 

  text_web_layer = text_layer_create(GRect(20, 1, 144-8, 168-68));
  text_layer_set_text(text_web_layer, "straphq.com");
  text_layer_set_text_color(text_web_layer, GColorWhite);
  text_layer_set_text_alignment(text_web_layer, GAlignCenter);
  text_layer_set_background_color(text_web_layer, GColorClear);
  text_layer_set_font(text_web_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21)));
  layer_add_child(window_layer, text_layer_get_layer(text_web_layer));

  text_time_layer = text_layer_create(GRect(10, 115, 144-7, 168-92));
  text_layer_set_text_color(text_time_layer, GColorWhite);
  text_layer_set_background_color(text_time_layer, GColorClear);
  text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49)));
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

  GRect line_frame = GRect(8, 97, 139, 2);
  line_layer = layer_create(line_frame);
  layer_set_update_proc(line_layer, line_layer_update_callback);
  // layer_add_child(window_layer, line_layer);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  // TODO: Update display here to avoid blank display on launch?



  // open AppMessage for Strap
  int in_size = app_message_inbox_size_maximum();
  int out_size = app_message_outbox_size_maximum();
  app_message_open(in_size, out_size);
  
  // handle initialization of Strap
  strap_init();

}


int main(void) {
  handle_init();

  app_event_loop();
  
  handle_deinit();
}
