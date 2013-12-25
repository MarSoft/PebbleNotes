#include <pebble.h>
#include "tasks.h"
#include "comm.h"
#include "misc.h"
#include "statusbar.h"
#include "options.h"

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
#define ICON_START GPoint(0, 3)
#endif

static Window *wndTasks;
static MenuLayer *mlTasks;
static GBitmap *bmpTasks[2];
static GFont *menuFont;

static int listId = -1;
static char* listTitle = "?!?";
static int ts_count = -1;
static int ts_max_count = -1;
static TS_Item *ts_items = NULL;

static uint16_t ts_get_num_rows_cb(MenuLayer *ml, uint16_t section_index, void *context) {
	if(ts_count < 0) // not initialized
		return 1; // there must be a message in statusbar
	else if(ts_count == 0) // no data
		return 1;
	else
		return ts_count;
}
static int16_t ts_get_header_height_cb(MenuLayer *ml, uint16_t section, void *context) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}
static void ts_draw_header_cb(GContext *ctx, const Layer *cell_layer, uint16_t section, void *context) {
	char *header;
	if(section == 0)
		header = listTitle;
	else
		header = "**unexpected header**";
	menu_cell_basic_header_draw(ctx, cell_layer, header);
}
/**
 * Draw text spanning two lines
 * with a small icon to the left of the first line (if provided)
 * Height is assumed to be defaul 44px.
 */
static void ts_twoline_cell_draw(GContext *ctx, const Layer *layer, char *title, GBitmap *icon) {
	char *buf = NULL;
	if(icon) {
		buf = malloc(strlen(title) + ICON_SPACES);
		memset(buf, ' ', ICON_SPACES);
		strcpy(buf+ICON_SPACES, title);
	} else {
		buf = title;
	}
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, buf, menuFont, ITEM_RECT,
		   GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	if(icon) {
		graphics_draw_bitmap_in_rect(ctx, icon, (GRect){ .origin = ICON_START, .size = icon->bounds.size });
		free(buf);
	}
}
static void ts_draw_row_cb(GContext *ctx, const Layer *cell_layer, MenuIndex *idx, void *context) {
	char *title;
	GBitmap *icon = NULL;
	if(ts_max_count == 0) // empty list
		title = "No tasks in this list!";
	else if(idx->row >= ts_count) // no such item (yet?)
		title = "<...>";
	else if(ts_max_count == 1 && ts_items[idx->row].title[0] == '\0') // the only item which is empty
		title = "<empty>";
	else {
		title = ts_items[idx->row].title;
		icon = bmpTasks[ts_items[idx->row].done];
	}
	if(options_large_font())
		menu_cell_basic_draw(ctx, cell_layer, title, NULL, icon); // use default func, big font
	else
		ts_twoline_cell_draw(ctx, cell_layer, title, icon); // use custom func, condensed font
}
static void ts_select_click_cb(MenuLayer *ml, MenuIndex *idx, void *context) {
	TS_Item task = ts_items[idx->row];
	comm_update_task_status(listId, task.id, !task.done);
}

static void ts_window_load(Window *wnd) {
	Layer *wnd_layer = window_get_root_layer(wnd);
	GRect bounds = layer_get_bounds(wnd_layer);

	mlTasks = menu_layer_create(bounds);
	menu_layer_set_callbacks(mlTasks, NULL, (MenuLayerCallbacks) {
		.get_num_rows = ts_get_num_rows_cb,
		.get_header_height = ts_get_header_height_cb,
		.draw_header = ts_draw_header_cb,
		.draw_row = ts_draw_row_cb,
		.select_click = ts_select_click_cb,
	});
	menu_layer_set_click_config_onto_window(mlTasks, wnd);
	layer_add_child(wnd_layer, menu_layer_get_layer(mlTasks));
}
static void ts_window_unload(Window *wnd) {
	menu_layer_destroy(mlTasks);
}
static void ts_free_items() {
	for(int i=0; i<ts_count; i++)
		free(ts_items[i].title);
	free(ts_items);
}

/* Public functions */

void ts_init() {
	wndTasks = window_create();
	window_set_window_handlers(wndTasks, (WindowHandlers) {
		.load = ts_window_load,
		.disappear = sb_window_disappear_cb,
		.unload = ts_window_unload,
	});
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
		ts_items = NULL;
		ts_count = -1;
		ts_max_count = -1;
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
void ts_set_count(int count) {
	LOG("Setting count: %d", count);
	if(ts_items)
		ts_free_items();
	ts_items = malloc(sizeof(TS_Item)*count);
	ts_max_count = count;
	ts_count = 0;
}
void ts_set_item(int i, TS_Item data) {
	LOG("New item %d", i);
	assert(ts_max_count > 0, "Trying to set item while not initialized!");
	assert(ts_max_count > i, "Unexpected item index: %d, max count is %d", i, ts_max_count);
	
	ts_items[i].id = data.id;
	ts_items[i].done = data.done;
	ts_items[i].title = malloc(strlen(data.title)+1);
	strcpy(ts_items[i].title, data.title);
	if(data.notes) {
		ts_items[i].notes = malloc(strlen(data.notes)+1);
		strcpy(ts_items[i].notes, data.notes);
	} else
		ts_items[i].notes = NULL;
	ts_count++;
	menu_layer_reload_data(mlTasks);
	LOG("Current count is %d", ts_count);
}
void ts_update_item_state_by_id(int id, bool state) {
	LOG("Updating state for itemId %d", id);
	for(int i=0; i<ts_count; i++) {
		if(ts_items[i].id == id) {
			assert(ts_items[i].done != state, "Tried to update with the old state");
			ts_items[i].done = state;
			menu_layer_reload_data(mlTasks);
			return;
		}
	}
	APP_LOG(APP_LOG_LEVEL_ERROR, "NOTICE: Couldn't find desired item ID %d to update", id);
}
