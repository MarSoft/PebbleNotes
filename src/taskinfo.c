#include <pebble.h>
#include "taskinfo.h"
#include "tasks.h"
#include "comm.h"
#include "misc.h"
#include "statusbar.h"
#include "options.h"

static Window *wndTaskInfo;
static ScrollLayer *slScroll;
static TextLayer *tlTitle;
static TextLayer *tlNotes;

static int taskId = -1;
static TS_Item currentTask;

/* Private functions */

static void ti_window_load(Window *wnd) {
	Layer *wnd_layer = window_get_root_layer(wnd);
	GRect bounds = layer_get_bounds(wnd_layer);
	GRect max_text_bounds = GRect(0,0, bounds.size.w,2000);

	slScroll = scroll_layer_create(bounds);
	scroll_layer_set_click_config_onto_window(slScroll, wnd);

	tlTitle = text_layer_create(max_text_bounds);
	tlNotes = text_layer_create(max_text_bounds);
	text_layer_set_font(tlTitle, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_font(tlNotes, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text(tlTitle, "Title");
	text_layer_set_text(tlNotes, "Notes");
	scroll_layer_add_child(slScroll, text_layer_get_layer(tlTitle));
	scroll_layer_add_child(slScroll, text_layer_get_layer(tlNotes));

	layer_add_child(wnd_layer, scroll_layer_get_layer(slScroll));
}
static void ti_window_unload(Window *wnd) {
	text_layer_destroy(tlNotes);
	text_layer_destroy(tlTitle);
	scroll_layer_destroy(slScroll);
}

/* Public functions */

void ti_init() {
	wndTaskInfo = window_create();
	window_set_window_handlers(wndTaskInfo, (WindowHandlers) {
			.load = ti_window_load,
			.disappear = sb_window_disappear_cb,
			.unload = ti_window_unload,
	});
	LOG("TaskInfo module initialized, window is %p", wndTaskInfo);
}
void ti_deinit() {
	window_destroy(wndTaskInfo);
}
void ti_show(int listId, TS_Item task) {
	taskId = task.id;
	LOG("Showing task for listId=%d, taskId=%d",
		   	listId, taskId);
	currentTask = task;

	window_stack_push(wndTaskInfo, true);
}
bool ti_is_active() {
	return window_stack_get_top_window() == wndTaskInfo;
}
int ti_current_taskId() {
	return currentTask.id;
}
