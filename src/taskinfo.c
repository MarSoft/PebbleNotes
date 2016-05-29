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
static TextLayer *tlStrike;

static int listId = -1;
static TS_Item currentTask;

/* Private functions */

static void ti_select_click(ClickRecognizerRef r, void *ctx) {
	comm_update_task_status(listId, currentTask.id, !currentTask.done);
}
static void ti_click_config_provider(void* ctx) {
	window_single_click_subscribe(BUTTON_ID_SELECT, ti_select_click);
}
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
#ifdef PBL_ROUND
	// it just works (hopefully)
	max_size_t.h *= 2;
	max_size_n.h *= 2;
#else
	max_size_n.h += 100;
#endif
	GRect bounds_t = GRect(0, 0, bounds.size.w, max_size_t.h);
	GRect bounds_n = GRect(0, max_size_t.h, bounds.size.w, max_size_n.h);
	layer_set_frame(text_layer_get_layer(tlTitle), bounds_t);
	layer_set_frame(text_layer_get_layer(tlNotes), bounds_n);
	text_layer_set_size(tlTitle, bounds_t.size);
	text_layer_set_size(tlNotes, bounds_n.size);
	scroll_layer_set_content_size(slScroll,
		   GSize(bounds.size.w, max_size_t.h + max_size_n.h + 4));
	layer_set_hidden(text_layer_get_layer(tlStrike), !currentTask.done);
#ifdef PBL_COLOR
	text_layer_set_text_color(tlTitle, currentTask.done ? GColorGreen : GColorBlack);
#endif
	text_layer_enable_screen_text_flow_and_paging(tlTitle, 3);
	text_layer_enable_screen_text_flow_and_paging(tlNotes, 3);
}
static void ti_window_load(Window *wnd) {
	Layer *wnd_layer = window_get_root_layer(wnd);
	GRect bounds = layer_get_bounds(wnd_layer);
	GRect max_text_bounds = GRect(0,0, bounds.size.w,2000);

	slScroll = scroll_layer_create(bounds);
	assert_oom(slScroll, "OOM while creating scroll layer");
	scroll_layer_set_callbacks(slScroll, (ScrollLayerCallbacks){
			.click_config_provider = ti_click_config_provider,
	});
	scroll_layer_set_click_config_onto_window(slScroll, wnd);

	tlTitle = text_layer_create(max_text_bounds);
	assert_oom(tlTitle, "OOM while creating title layer");
	tlNotes = text_layer_create(max_text_bounds);
	assert_oom(tlNotes, "OOM while creating notes layer");
	text_layer_set_font(tlTitle, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_font(tlNotes, fonts_get_system_font(FONT_KEY_GOTHIC_18));

#ifdef PBL_ROUND
	text_layer_set_text_alignment(tlTitle, GTextAlignmentCenter);
	text_layer_set_text_alignment(tlNotes, GTextAlignmentCenter);
	scroll_layer_set_paging(slScroll, true);
#endif

	tlStrike = text_layer_create(GRect(0, 17, bounds.size.w, 1));
	assert_oom(tlStrike, "OOM while creating strike layer");
	text_layer_set_background_color(tlStrike, GColorBlack);

	scroll_layer_add_child(slScroll, text_layer_get_layer(tlTitle));
	scroll_layer_add_child(slScroll, text_layer_get_layer(tlNotes));
	scroll_layer_add_child(slScroll, text_layer_get_layer(tlStrike));

	layer_add_child(wnd_layer, scroll_layer_get_layer(slScroll));

	ti_show_current_task();

}
static void ti_window_unload(Window *wnd) {
	text_layer_destroy(tlStrike);
	text_layer_destroy(tlNotes);
	text_layer_destroy(tlTitle);
	scroll_layer_destroy(slScroll);
}

/* Public functions */

void ti_init() {
	wndTaskInfo = window_create();
	assert_oom(wndTaskInfo, "OOM while creating task info window");
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
		if(currentTask.title) {
			strcpy(currentTask.title, task.title);
		} else {
			APP_LOG(APP_LOG_LEVEL_ERROR, "OOM while allocating title");
			sb_show("OOM");
		}
	}
	if(task.notes) {
		currentTask.notes = malloc(strlen(task.notes)+1);
		if(currentTask.notes) {
			strcpy(currentTask.notes, task.notes);
		} else {
			APP_LOG(APP_LOG_LEVEL_ERROR, "OOM while allocating notes");
			sb_show("OOM");
		}
	}

	if(ti_is_active())
		ti_show_current_task();
	else
		// it will call window_load, which will call show_current_task
		window_stack_push(wndTaskInfo, true);
}
bool ti_is_active() {
	return window_stack_get_top_window() == wndTaskInfo;
}
int ti_current_taskId() {
	return currentTask.id;
}
