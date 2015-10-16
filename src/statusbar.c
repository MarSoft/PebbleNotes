#include <pebble.h>
#include "statusbar.h"
#include "misc.h"

static char* sb_buf = NULL;
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

static void sb_show_do() { // show current buffer
	assert(sb_buf, "No message to show!");
	Window *wnd = window_stack_get_top_window();
	if(!wnd) {
		// TODO: create new window?
		APP_LOG(APP_LOG_LEVEL_ERROR, "No active window! Want to say: %s", sb_buf);
		return;
	}
	Layer *wnd_layer = window_get_root_layer(wnd);
	text_layer_set_text(tlStatusBar, sb_buf);
	LOG("Status: %s %p", sb_buf, (void*)sb_buf);
	GRect bounds = layer_get_bounds(wnd_layer);
	GRect new;
	new.size = text_layer_get_content_size(tlStatusBar);
	new.size.h += 5; // enhance for lower parts of letters
	if(new.size.h > bounds.size.h) // if size exceeds screen, just crop
		new.size.h = maxRect.size.h;
	// move our layer to screen bottom
	new.origin.x = bounds.origin.x;
	new.origin.y = bounds.origin.y + bounds.size.h - new.size.h;
	layer_set_frame(text_layer_get_layer(tlStatusBar), new);
	layer_add_child(wnd_layer, text_layer_get_layer(tlStatusBar));
}
void sb_show(char *text) {
	LOG("Status bar: %s", text);
	sb_hide(); // free buffer in advance
	sb_buf = malloc(strlen(text)+1);
	strcpy(sb_buf, text);
	sb_show_do();
}
char* sb_printf_alloc(int size) {
	if(sb_buf)
		free(sb_buf);
	return sb_buf = malloc(size+1);
}
char *sb_printf_get(int size) {
	if(!sb_buf) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Allocating status bar buffer for %d chars", size);
		sb_buf = malloc(size+1);
	}
	return sb_buf;
}
void sb_printf_update() {
	sb_show_do();
}
void sb_hide() {
	layer_remove_from_parent(text_layer_get_layer(tlStatusBar));
	text_layer_set_size(tlStatusBar, maxRect.size);
	if(sb_buf) {
		free(sb_buf);
		sb_buf = NULL;
	}
}
void sb_window_disappear_cb(Window *wnd) {
	sb_hide();
}
