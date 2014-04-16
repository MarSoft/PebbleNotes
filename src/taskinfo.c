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
static InverterLayer *ilStrike;

static int listId = -1;
static TS_Item currentTask;

/* Private functions */

static void ti_show_current_task() {
	GRect bounds = layer_get_bounds(window_get_root_layer(wndTaskInfo));

	char* title = currentTask.title;
	char* notes = currentTask.notes;

	if(!title)
		title = "<No title>";
	if(!notes)
		notes = "";
	text_layer_set_text(tlTitle, title);
	text_layer_set_text(tlNotes, notes);
	GSize max_size_t = text_layer_get_content_size(tlTitle);
	GSize max_size_n = text_layer_get_content_size(tlNotes);
	GRect bounds_t = GRect(0, 0, bounds.size.w, max_size_t.h);
	GRect bounds_n = GRect(0, max_size_t.h, bounds.size.w, max_size_n.h);
	layer_set_frame(text_layer_get_layer(tlTitle), bounds_t);
	layer_set_frame(text_layer_get_layer(tlNotes), bounds_n);
	text_layer_set_size(tlTitle, bounds_t.size);
	text_layer_set_size(tlNotes, bounds_n.size);
	scroll_layer_set_content_size(slScroll,
		   GSize(bounds.size.w, max_size_t.h + max_size_n.h + 4));
	layer_set_hidden(inverter_layer_get_layer(ilStrike), !currentTask.done);
}
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

	GRect inv_rect = GRect(0, 17, bounds.size.w, 1);
	ilStrike = inverter_layer_create(inv_rect);

	ti_show_current_task();

	scroll_layer_add_child(slScroll, text_layer_get_layer(tlTitle));
	scroll_layer_add_child(slScroll, text_layer_get_layer(tlNotes));
	scroll_layer_add_child(slScroll, inverter_layer_get_layer(ilStrike));

	layer_add_child(wnd_layer, scroll_layer_get_layer(slScroll));
}
static void ti_window_unload(Window *wnd) {
	inverter_layer_destroy(ilStrike);
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
	memset(&currentTask, 0, sizeof(TS_Item));
	LOG("TaskInfo module initialized, window is %p", wndTaskInfo);
}
void ti_deinit() {
	window_destroy(wndTaskInfo);
}
void ti_show(int aListId, TS_Item task) {
	listId = aListId;
	LOG("Showing task for listId=%d, taskId=%d, done=%c",
		   	listId, task.id, task.done);
	if(currentTask.title) {
		free(currentTask.title);
		currentTask.title = NULL;
	}
	if(currentTask.notes) {
		free(currentTask.notes);
		currentTask.notes = NULL;
	}
	currentTask.id = task.id;
	currentTask.done = task.done;
	if(task.title) {
		currentTask.title = malloc(strlen(task.title)+1);
		strcpy(currentTask.title, task.title);
	}
	if(task.notes) {
		currentTask.notes = malloc(strlen(task.notes)+1);
		strcpy(currentTask.notes, task.notes);
	}

	// it will call window_load, which will call show_task
	window_stack_push(wndTaskInfo, true);
}
bool ti_is_active() {
	return window_stack_get_top_window() == wndTaskInfo;
}
int ti_current_taskId() {
	return currentTask.id;
}
