#include <pebble.h>
#include "optout.h"


#define PROMPT_ANIMATED true
#define PROMPT_USER "Allow this app to gather anonymous usage metrics?\n\n\n\n\n\n\nstraphq.com"
#define PROMPT_YES_BUTTON "Yes"
#define PROMPT_NO_BUTTON "No"
static Window* prompt;
static bool prompt_selected_yes;
static bool prompt_select_pressed = false;
static TextLayer *prompt_question;
static TextLayer *prompt_yes;
static TextLayer *prompt_no;

static void highlight_prompt_selection();
static void prompt_load(Window *window);
static void prompt_unload(Window *window);
static void prompt_click_config_provider(void *context);
static void prompt_select_click_handler(ClickRecognizerRef recognizer,
                                        void *context);
static void prompt_up_click_handler(ClickRecognizerRef recognizer,
                                    void *context);
static void prompt_down_click_handler(ClickRecognizerRef recognizer,
                                      void *context);

static void highlight_prompt_selection() {
    TextLayer *to_highlight = prompt_selected_yes ? prompt_yes : prompt_no;
    TextLayer *unhighlight = prompt_selected_yes ? prompt_no : prompt_yes;

    text_layer_set_background_color(to_highlight, GColorBlack);
    text_layer_set_background_color(unhighlight, GColorWhite);

    text_layer_set_text_color(to_highlight, GColorWhite);
    text_layer_set_text_color(unhighlight, GColorBlack);
}

static void prompt_load(Window *window) {
    prompt_select_pressed = false;
    Layer *window_layer = window_get_root_layer(window);

    prompt_question = text_layer_create((GRect) {
        .origin = { 9, 8 },
        .size = { 126, 150 }
    });
    text_layer_set_text(prompt_question, PROMPT_USER);
    text_layer_set_text_alignment(prompt_question, GTextAlignmentCenter);

    prompt_yes = text_layer_create((GRect) {
        .origin = { 52, 80 },
        .size = { 40, 20 }
    });
    text_layer_set_text(prompt_yes, PROMPT_YES_BUTTON);
    text_layer_set_text_alignment(prompt_yes, GTextAlignmentCenter);

    prompt_no = text_layer_create((GRect) {
        .origin = { 52, 100 },
        .size = { 40, 20 }
    });
    text_layer_set_text(prompt_no, PROMPT_NO_BUTTON);
    text_layer_set_text_alignment(prompt_no, GTextAlignmentCenter);

    highlight_prompt_selection();

    layer_add_child(window_layer, text_layer_get_layer(prompt_question));
    layer_add_child(window_layer, text_layer_get_layer(prompt_yes));
    layer_add_child(window_layer, text_layer_get_layer(prompt_no));
}

static void prompt_unload(Window *window) {

    // if the user used the back button, opt out
    if (!prompt_select_pressed) {
        persist_write_bool(STRAP_OPT_OUT, true);

    // if user used the select button, set to user's selection
    } else {
        persist_write_bool(STRAP_OPT_OUT, !prompt_selected_yes);
    }

    text_layer_destroy(prompt_question);
    text_layer_destroy(prompt_yes);
    text_layer_destroy(prompt_no);
    window_destroy(window);
}

static void prompt_click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT,
                                  prompt_select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, prompt_up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, prompt_down_click_handler);
}

static void prompt_select_click_handler(ClickRecognizerRef recognizer,
                                        void *context) {
    prompt_select_pressed = true;
    window_stack_pop(PROMPT_ANIMATED);
}

static void prompt_up_click_handler(ClickRecognizerRef recognizer,
                                    void *context) {
    prompt_selected_yes = true;
    highlight_prompt_selection();
}

static void prompt_down_click_handler(ClickRecognizerRef recognizer,
                                      void *context) {
    prompt_selected_yes = false;
    highlight_prompt_selection();
}

void prompt_opt_out(bool default_selection) {
	prompt = window_create();
    prompt_selected_yes = default_selection;

    window_set_click_config_provider(prompt, prompt_click_config_provider);
    window_set_window_handlers(prompt, (WindowHandlers) {
        .load = prompt_load,
        .unload = prompt_unload,
    });

    if (!persist_exists(STRAP_OPT_OUT)) {
        window_stack_push(prompt, PROMPT_ANIMATED);
    }
}
