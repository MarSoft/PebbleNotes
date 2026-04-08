#include <pebble.h>
#include "statusbar.h"
#include "misc.h"

static char* sb_buf = NULL;
static TextLayer *tlStatusBar;
static bool maxRectUnknown = true;
static GRect maxRect = {{0, 0}, {144, 168}}; // will be updated later

void sb_init() {
	tlStatusBar = text_layer_create(maxRect); // exact rect doesn't matter now
	assert(tlStatusBar, "OOM while creating statusbar layer");
	text_layer_set_background_color(tlStatusBar, GColorBlack);
	text_layer_set_text_color(tlStatusBar, GColorWhite);
#ifdef PBL_ROUND
	text_layer_set_text_alignment(tlStatusBar, GTextAlignmentCenter);
#endif
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

	// check if we want to determine screen size
	// (for that we need a window, so we do it here, not in init)
	if(maxRectUnknown) {
		maxRect = layer_get_bounds(wnd_layer);
		// update frame which was probably badly initialized on init
		maxRectUnknown = false;
	}

	text_layer_set_text(tlStatusBar, sb_buf);
	LOG("Status: %s %p", sb_buf, (void*)sb_buf);
	// first move layer to top (important for Rounds)
	// and set its size to maximum
	layer_set_frame(text_layer_get_layer(tlStatusBar), maxRect);
#ifdef PBL_ROUND
	text_layer_enable_screen_text_flow_and_paging(tlStatusBar, 5);
#endif
	// now calculate actually occupied size
	GRect new;
	new.size = text_layer_get_content_size(tlStatusBar);
	new.size.h += 5; // enhance for lower parts of letters
#ifdef PBL_ROUND
	new.size.h *= 2;  // is just works - but why?..
	// on round screen we want to properly center content
	// and thus want statusbar to always occupy whole width
	new.size.w = maxRect.size.w;
#endif
	if(new.size.h > maxRect.size.h) // if size exceeds screen, just crop
		new.size.h = maxRect.size.h;
	// move our layer to screen bottom
	new.origin.x = maxRect.origin.x;
	new.origin.y = maxRect.origin.y + maxRect.size.h - new.size.h;
	layer_set_frame(text_layer_get_layer(tlStatusBar), new);
	layer_add_child(wnd_layer, text_layer_get_layer(tlStatusBar));
	// and now we can (on round pebble) finally set proper flow
#ifdef PBL_ROUND
	text_layer_enable_screen_text_flow_and_paging(tlStatusBar, 5);
#endif
}
void sb_show(char *text) {
	LOG("Status bar: %s", text);
	sb_hide(); // free buffer in advance
	sb_buf = malloc(strlen(text)+1);
	assert(sb_buf, "OOM while allocating statusbar buffer");
	strcpy(sb_buf, text);
	sb_show_do();
}
char* sb_printf_alloc(int size) {
	if(sb_buf)
		free(sb_buf);
	return sb_buf = malloc(size+1);
}
char *sb_printf_get() {
	if(!sb_buf)
		APP_LOG(APP_LOG_LEVEL_ERROR, "NOTICE: buffer was not allocated");
	return sb_buf;
}
void sb_printf_update() {
	sb_show_do();
}
void sb_hide() {
	layer_remove_from_parent(text_layer_get_layer(tlStatusBar));
	text_layer_set_size(tlStatusBar, maxRect.size);
	text_layer_restore_default_text_flow_and_paging(tlStatusBar);
	if(sb_buf) {
		free(sb_buf);
		sb_buf = NULL;
	}
}
void sb_window_disappear_cb(Window *wnd) {
	sb_hide();
}
