#include <pebble.h>
#include "tasks.h"
#include "taskinfo.h"
#include "comm.h"
#include "misc.h"
#include "statusbar.h"
#include "options.h"
#include "consts.h"

#ifdef BIGGER_FONT
#define CUSTOM_FONT "RESOURCE_ID_GOTHIC_24_BOLD"
// how many spaces should we skip to fit icon
#define ICON_SPACES 5
#define ITEM_RECT GRect(0, -6, 144, 48)
#define ICON_START GPoint(0, 3)
#else
#define CUSTOM_FONT "RESOURCE_ID_GOTHIC_18_BOLD"
#define ICON_SPACES 5
#define ITEM_RECT GRect(0, 0, 144, 44)
#ifdef PBL_ROUND
 #define ICON_START GPoint(4, 3)
#else
 #define ICON_START GPoint(0, 3)
#endif
#endif

static Window *wndTasks;
static MenuLayer *mlTasks;
static GBitmap *bmpTasks[2];
static GFont menuFont;

static int listId = -1;
static char* listTitle = "?!?";
static int ts_count = -1;
static int ts_max_count = -1;
static TS_Item *ts_items = NULL;

#ifdef PBL_MICROPHONE
static DictationSession *session;

static void ts_create_task_cb(DictationSession *session, DictationSessionStatus status, char *transcription, void *ctx) {
	if(status != DictationSessionStatusSuccess) {
		LOG("Dictation session failed with status %d", status);
		dictation_session_destroy(session);
		return;
	}

	// for now, text recognized goes to title, and notes are left empty
	comm_create_task(listId, transcription, NULL);

	dictation_session_destroy(session);
}
static void ts_create_task() {
	// for now, only dictation is supported,
	// thus in #ifdef block
	session = dictation_session_create(0, ts_create_task_cb, NULL);
	assert_oom(session, "Could not create dictation session");
	dictation_session_enable_confirmation(session, true);
	dictation_session_enable_error_dialogs(session, true);
	dictation_session_start(session);
}
#endif

static uint16_t ts_get_num_sections_cb(MenuLayer *ml, void *context) {
#ifdef PBL_MICROPHONE
	if(options_task_actions_position() == TaskActionsPositionNone)
		return 1;
	else if(ts_count > 0 && ts_count == ts_max_count)
		return 2; // tasks + actions
#endif
	return 1;
}
static uint16_t ts_get_num_rows_cb(MenuLayer *ml, uint16_t section_index, void *context) {
#ifdef PBL_MICROPHONE
	int act_section = options_task_actions_position() - 1;
	/* if -1 (disabled) then will never match */
	if(section_index == act_section && ts_count > 0 && ts_count == ts_max_count) // actions
		return 1;
#endif

	// else section is 0 -> main
	if(ts_count < 0) // not initialized
		return 1; // there must be a message in statusbar
	else if(ts_count == 0) // no data
		return 1;
	else if(!ts_items) // OOM
		return 1;
	else
		return ts_count;
}
static int16_t ts_get_header_height_cb(MenuLayer *ml, uint16_t section, void *context) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}
static void ts_draw_header_cb(GContext *ctx, const Layer *cell_layer, uint16_t section, void *context) {
	char *header;
#ifdef PBL_MICROPHONE
	if(section == options_task_actions_position() - 1 && ts_count > 0 && ts_count == ts_max_count)
		header = "Actions";
	else
#endif
		header = listTitle;

#ifdef PBL_ROUND
	//graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, header,
			fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
			layer_get_bounds(cell_layer),
			GTextOverflowModeTrailingEllipsis,
			GTextAlignmentCenter,
			NULL);
#else
	menu_cell_basic_header_draw(ctx, cell_layer, header);
#endif
}
/**
 * Draw text spanning two lines
 * with a small icon to the left of the first line (if provided)
 * Height is assumed to be defaul 44px.
 */
static void ts_twoline_cell_draw(GContext *ctx, const Layer *layer, char *title, GBitmap *icon, bool is_done) {
	char *buf = NULL;
	if(icon) {
		buf = malloc(strlen(title) + ICON_SPACES + 1);
		assert_oom(buf, "OOM while allocating draw buffer!");
		if(!buf)
			LOG("Used: %d, free: %d", heap_bytes_used(), heap_bytes_free());
		memset(buf, ' ', ICON_SPACES);
		strcpy(buf+ICON_SPACES, title);
	} else {
		buf = title;
	}
	graphics_context_set_text_color(ctx, menu_cell_layer_is_highlighted(layer) ? GColorWhite : GColorBlack);
#ifdef PBL_ROUND
	GColor color_fg = menu_cell_layer_is_highlighted(layer) ? GColorWhite : GColorBlack;
	graphics_context_set_text_color(ctx, is_done ? GColorGreen : color_fg);
	graphics_draw_text(ctx, buf, menuFont, ITEM_RECT,
		   GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	if(is_done) {
		graphics_context_set_stroke_color(ctx, color_fg);
		GRect bounds = layer_get_bounds(layer);
		graphics_draw_line(ctx,
				GPoint(0, bounds.origin.y + 11),
				GPoint(bounds.size.w, bounds.origin.y + 11));
	}
#else
	graphics_draw_text(ctx, buf, menuFont, ITEM_RECT,
		   GTextOverflowModeFill, GTextAlignmentLeft, NULL);
#endif
	if(icon) {
		graphics_draw_bitmap_in_rect(ctx, icon, (GRect){ .origin = ICON_START, .size = gbitmap_get_bounds(icon).size });
		free(buf);
	}
}
static void ts_draw_row_cb(GContext *ctx, const Layer *cell_layer, MenuIndex *idx, void *context) {
#ifdef PBL_MICROPHONE
	if(idx->section == options_task_actions_position() - 1 && ts_count > 0 && ts_count == ts_max_count) {
		// actions
		// i.e. "Add task" action
		menu_cell_basic_draw(ctx, cell_layer, "Create Task", NULL, NULL);
		return;
	}
#endif

	char *title;
	GBitmap *icon = NULL;
	bool is_done = false;
	if(ts_max_count == 0) // empty list
		title = "No tasks in this list!";
	else if(idx->row >= ts_count) // no such item (yet?)
		title = "<...>";
	else if(ts_max_count == 1 && ts_items[idx->row].title[0] == '\0') // the only item which is empty
		title = "<empty>";
	else if(!ts_items)
		title = "<OOM>";
	else {
		title = ts_items[idx->row].title;
		if(!title)
			title = "<OOM>";
		icon = bmpTasks[ts_items[idx->row].done];
		is_done = ts_items[idx->row].done;
	}
	if(options_large_font())
		menu_cell_basic_draw(ctx, cell_layer, title, NULL, icon); // use default func, big font
	else
		ts_twoline_cell_draw(ctx, cell_layer, title, icon, is_done); // use custom func, condensed font
}
static void ts_select_click_cb(MenuLayer *ml, MenuIndex *idx, void *context) {
#ifdef PBL_MICROPHONE
	if(idx->section == options_task_actions_position() - 1) {
		// actions
		// create task
		ts_create_task();
		return;
	}
#endif

	if(ts_max_count == 0 || idx->row >= ts_count)
		return; // don't do anything if we have no data for this row
	TS_Item task = ts_items[idx->row];
	comm_update_task_status(listId, task.id, !task.done);
}
static void ts_select_long_click_cb(MenuLayer *ml, MenuIndex *idx, void *context) {
#ifdef PBL_MICROPHONE
	if(idx->section == options_task_actions_position() - 1) // actions
		return;
#endif

	if(ts_max_count == 0 || idx->row >= ts_count)
		return; // don't do anything if we have no data for this row
	if(heap_bytes_free() < OOM_MIN_TASKINFO) {
		sb_show("Not enough memory");
		return;
	}
	TS_Item task = ts_items[idx->row];
	ti_show(listId, task);
}

static void ts_window_load(Window *wnd) {
	Layer *wnd_layer = window_get_root_layer(wnd);
	GRect bounds = layer_get_bounds(wnd_layer);

	mlTasks = menu_layer_create(bounds);
	assert_oom(mlTasks, "OOM while creating menu layer");
	menu_layer_set_callbacks(mlTasks, NULL, (MenuLayerCallbacks) {
		.get_num_sections = ts_get_num_sections_cb,
		.get_num_rows = ts_get_num_rows_cb,
		.get_header_height = ts_get_header_height_cb,
		.draw_header = ts_draw_header_cb,
		.draw_row = ts_draw_row_cb,
		.select_click = ts_select_click_cb,
		.select_long_click = ts_select_long_click_cb,
	});
	menu_layer_set_click_config_onto_window(mlTasks, wnd);
	layer_add_child(wnd_layer, menu_layer_get_layer(mlTasks));
}
static void ts_window_unload(Window *wnd) {
	menu_layer_destroy(mlTasks);
}
static void ts_free_items() {
	LOG("Used: %d, free: %d", heap_bytes_used(), heap_bytes_free());
	LOG("Freeing items");
	for(int i=0; i<ts_count; i++)
	{
		free(ts_items[i].title);
		if(ts_items[i].notes)
			free(ts_items[i].notes);
	}
	free(ts_items);
	ts_items = NULL;
	LOG("Used: %d, free: %d", heap_bytes_used(), heap_bytes_free());
}

/* Public functions */

void ts_init() {
	wndTasks = window_create();
	assert_oom(wndTasks, "OOM while creating tasks window");
	window_set_window_handlers(wndTasks, (WindowHandlers) {
		.load = ts_window_load,
		.disappear = sb_window_disappear_cb,
		.unload = ts_window_unload,
	});
	LOG("Tasks module init - loading resources...");
	bmpTasks[0] = gbitmap_create_with_resource(RESOURCE_ID_TASK_UNDONE);
	bmpTasks[1] = gbitmap_create_with_resource(RESOURCE_ID_TASK_DONE);
	menuFont = fonts_get_system_font(CUSTOM_FONT);
	LOG("Tasks module initialized, window is %p", wndTasks);
}
void ts_deinit() {
	window_destroy(wndTasks);
	ts_free_items();
	gbitmap_destroy(bmpTasks[0]);
	gbitmap_destroy(bmpTasks[1]);
}
void ts_show(int id, char* title) {
	LOG("Showing tasks for listId=%d", id);
	if(id != listId) { // not the same list; clearing and will reload
		if(ts_items)
			ts_free_items();
		ts_count = -1;
		ts_max_count = -1;
	} else if(options_task_actions_position() == 1) {
		// FIXME doesn't work for some reason
		menu_layer_set_selected_index(mlTasks, MenuIndex(1, 0),
				PBL_IF_ROUND_ELSE(MenuRowAlignCenter, MenuRowAlignTop),
				false); // not animated
	}
	listId = id;
	listTitle = title;

	window_stack_push(wndTasks, true);
	if(ts_count < 0)
		comm_query_tasks(id);
}
bool ts_is_active() {
	return window_stack_get_top_window() == wndTasks;
}
int ts_current_listId() {
	return listId;
}
int ts_current_if_complete() {
	if(listId != -1 && ts_count > 0 && !ts_items) {
		// OOM state
		return -1;
	}
	return listId;
}
void ts_set_count(int count) {
	LOG("Setting count: %d", count);
	if(ts_items)
		ts_free_items();
	ts_items = malloc(sizeof(TS_Item)*count);
	ts_count = 0;
	ts_max_count = count;
	if(count>0 && !ts_items) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "OOM while allocating items!");
		ts_max_count = 0;
	}
}
void ts_set_item(int i, TS_Item data) {
	LOG("New item %d", i);
	assert(ts_max_count > 0, "Trying to set item while not initialized!");
	assert(ts_max_count > i, "Unexpected item index: %d, max count is %d", i, ts_max_count);
	
	ts_items[i].id = data.id;
	ts_items[i].done = data.done;
	int tlen = strlen(data.title);
	if(heap_bytes_free() - tlen > OOM_SAFEGUARD) {
		ts_items[i].title = malloc(tlen+1);
		if(ts_items[i].title) {
			strcpy(ts_items[i].title, data.title);
		} else {
			APP_LOG(APP_LOG_LEVEL_ERROR, "OOM while allocating title!");
			sb_show("OOM");
		}
	} else
		ts_items[i].title = "<OOM>";
	if(data.notes) {
		int nlen = strlen(data.notes);
		if(heap_bytes_free() - nlen > OOM_SAFEGUARD) {
			ts_items[i].notes = malloc(nlen+1);
			if(ts_items[i].notes) {
				strcpy(ts_items[i].notes, data.notes);
			} else {
				APP_LOG(APP_LOG_LEVEL_ERROR, "OOM while allocating notes!");
				sb_show("OOM");
			}
		} else
			ts_items[i].notes = "<OOM>";
	} else
		ts_items[i].notes = NULL;
	ts_count++;
	menu_layer_reload_data(mlTasks);
	LOG("Current count is %d", ts_count);
	if(ts_count == ts_max_count && options_task_actions_position() == 1) {
		menu_layer_set_selected_index(mlTasks, MenuIndex(1, 0),
				PBL_IF_ROUND_ELSE(MenuRowAlignCenter, MenuRowAlignTop),
				false); // not animated
	}
}
void ts_append_item(TS_Item data) {
	LOG("Additional item with id %d", data.id);
	assert(ts_max_count >= 0, "Trying to append item while not initialized!");
	assert(ts_max_count == ts_count, "Trying to add task while not fully loaded!");
	assert_oom(heap_bytes_free() > OOM_SAFEGUARD, "Almost OOM - ignoring item!");
	ts_count++;
	ts_max_count++;
	// increase array memory
	ts_items = realloc(ts_items, sizeof(TS_Item)*ts_count);
	int i = ts_max_count-1; // last task
	ts_items[i].id = data.id;
	ts_items[i].done = data.done;
	ts_items[i].title = malloc(strlen(data.title)+1);
	if(ts_items[i].title)
		strcpy(ts_items[i].title, data.title);
	else
		APP_LOG(APP_LOG_LEVEL_ERROR, "OOM while allocating title");
	if(data.notes) {
		ts_items[i].notes = malloc(strlen(data.notes)+1);
		if(ts_items[i].notes)
			strcpy(ts_items[i].notes, data.notes);
		else
			APP_LOG(APP_LOG_LEVEL_ERROR, "OOM while allocating notes");
	} else
		ts_items[i].notes = NULL;
	menu_layer_reload_data(mlTasks);
	LOG("Task appended, new count is %d", ts_count);
}
void ts_update_item_state_by_id(int id, bool state) {
	LOG("Updating state for itemId %d", id);
	for(int i=0; i<ts_count; i++) {
		if(ts_items[i].id == id) {
			assert(ts_items[i].done != state, "Tried to update with the old state");
			ts_items[i].done = state;
			if(ts_is_active()) {
				menu_layer_reload_data(mlTasks);
			} else if(ti_is_active()) {
				if(ti_current_taskId() == id)
					ti_show(listId, ts_items[i]);
				else
					LOG("Skipping update: this task is not active");
			} else {
				LOG("Skipping update: neither tasks nor taskinfo is active");
			}
			return;
		}
	}
	APP_LOG(APP_LOG_LEVEL_ERROR, "NOTICE: Couldn't find desired item ID %d to update", id);
}
