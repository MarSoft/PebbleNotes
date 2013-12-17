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
  
  LOG("Sending message..");
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
	Tuple *tCode, *tScope, *tCount, *tItem;

	tCode = dict_find(iter, KEY_CODE);
	assert(tCode, "Received message without code!");
	assert(tCode->type == TUPLE_INT, "Bad type for Code field: %d", tCode->type);
	assert(tCode->length == 4, "Strange code length: %d", tCode->length);
	int nCode = (int)tCode->value->int32;
	LOG("Message code: %d", nCode);

	if(nCode == CODE_ERROR) {
		Tuple *tError = dict_find(iter, KEY_ERROR);
		char* szError = "Unknown error";
		if(tError && tError->type == TUPLE_CSTRING)
			szError = tError->value->cstring;
		LOG("Error received: %s", szError); // TODO: display error msg
		return;
	}

	tScope = dict_find(iter, KEY_SCOPE);
	assert(tScope, "Received message without scope! Code=%d", nCode);
	assert(tScope->type == TUPLE_INT, "Bad type for Scope field: %d", tScope->type);
	assert(tScope->length == 4, "Strange scope length!");
	int nScope = (int)tScope->value->int32;
	LOG("Message scope: %d", nScope);

	switch(nCode) {
		case CODE_ARRAY_START:
			tCount = dict_find(iter, KEY_COUNT);
			int nCount = (int)tCount->value->int32;
			LOG("Items count: %d", nCount);
			break;
		case CODE_ARRAY_ITEM:
			tItem = dict_find(iter, KEY_ITEM);
			int nItem = (int)tItem->value->int32;
			int nListId = (int)dict_find(iter, KEY_LISTID)->value->int32;
			char* szTitle = dict_find(iter, KEY_TITLE)->value->cstring;
			LOG("Item No: %d. Id=%d", nItem, nListId);
  			text_layer_set_text(text_layer, szTitle);
			break;
		case CODE_ARRAY_END:
			break;
		case CODE_ERROR: // Impossible! See above.
		default:
			LOG("Unknown message code received: %d", nCode);
			break;
	}
}
void in_dropped_handler(AppMessageResult reason, void *context) {
	LOG("Message dropped: reason=%d", reason);
}
void out_sent_handler(DictionaryIterator *sent, void *context) {
	LOG("Message sent");
}
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	LOG("Message send failed");
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
