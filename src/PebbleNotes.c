#include <pebble.h>
#include "consts.h"

#define LOG(args...) APP_LOG(APP_LOG_LEVEL_DEBUG, args)
#define assert(e, msg...) if(!e) { APP_LOG(APP_LOG_LEVEL_ERROR, msg); return; }

static Window *window;
static TextLayer *text_layer;

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Sending query");
  
  LOG("Sending message..\n");
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  Tuplet code = TupletInteger(KEY_CODE, CODE_GET);
  dict_write_tuplet(iter, &code);
  Tuplet scope = TupletInteger(KEY_SCOPE, SCOPE_LISTS);
  dict_write_tuplet(iter, &scope);
  app_message_outbox_send();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Press a button");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

void in_received_handler(DictionaryIterator *iter, void *context) {
	Tuple *code = dict_find(iter, KEY_CODE);
	assert(code, "Received message without code!\n");
	assert(code->type == TUPLE_INT, "Bad type for Code field: %d\n", code->type);
	assert(code->length == 4, "Strange code length: %d\n", code->length);
	LOG("Message code: %d\n", (int)code->value->int32);
}
void in_dropped_handler(AppMessageResult reason, void *context) {
	// incoming message dropped
}
void out_sent_handler(DictionaryIterator *sent, void *context) {
	// outgoing message was delivered
}
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	// outgoing message failed
}

static void init(void) {
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);

	app_message_open(app_message_inbox_size_maximum(), APP_MESSAGE_OUTBOX_SIZE_MINIMUM); // We only need large buffer for inbox

  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
