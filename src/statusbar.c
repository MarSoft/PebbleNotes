#include <pebble.h>
#include "statusbar.h"
#include "misc.h"

static TextLayer *tlStatusBar;
static GRect maxRect = {{0, 0}, {144, 168}}; // FIXME

void sb_init() {
	tlStatusBar = text_layer_create(maxRect);
	text_layer_set_background_color(tlStatusBar, GColorBlack);
	text_layer_set_text_color(tlStatusBar, GColorWhite);
}
void sb_deinit() {
	text_layer_destroy(tlStatusBar);
}

void sb_show(char *text) {
	LOG("Status bar: %s", text);
	Window *wnd = window_stack_get_top_window();
	if(!wnd) {
		// TODO: create new window?
		APP_LOG(APP_LOG_LEVEL_ERROR, "No active window! Want to say: %s", text);
		return;
	}
	Layer *wnd_layer = window_get_root_layer(wnd);
	text_layer_set_text(tlStatusBar, text);
	GRect bounds = layer_get_bounds(wnd_layer);
	GRect new;
	new.size = text_layer_get_content_size(tlStatusBar);
	if(new.size.h > bounds.size.h) // if size exceeds screen, just crop
		new.size.h = maxRect.size.h;
	// move our layer to screen bottom
	new.origin.x = bounds.origin.x;
	new.origin.y = bounds.origin.y + bounds.size.h - new.size.h;
	layer_set_frame(text_layer_get_layer(tlStatusBar), new);
	layer_add_child(wnd_layer, text_layer_get_layer(tlStatusBar));
}
void sb_hide() {
	layer_remove_from_parent(text_layer_get_layer(tlStatusBar));
	text_layer_set_size(tlStatusBar, maxRect.size);
}
