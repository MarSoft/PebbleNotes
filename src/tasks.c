#include <pebble.h>
#include "tasks.h"
#include "comm.h"
#include "misc.h"

static Window *wndTasks;
static MenuLayer *mlTasks;

static int listId = -1;
static char* listTitle = "?!?";
static int ts_count = -1;
static int ts_max_count = -1;
static TS_Item *ts_items = NULL;

static void ts_draw_row_cb(GContext *ctx, const Layer *cell_layer, MenuIndex *idx, void *context) {
	char *title;
	if(ts_count < 0) // didn't receive any data yet
		title = "Loading...";
	else if(ts_max_count == 0) // empty list
		title = "No tasks in this list! You may create one...";
	else if(idx->row > ts_count) // that row is not loaded yet; must be an ellipsis row
		title = "...";
	else
		title = ts_items[idx->row].title;
	menu_cell_title_draw(ctx, cell_layer, title);
}
static uint16_t ts_get_num_rows_cb(MenuLayer *ml, uint16_t section_index, void *context) {
	if(ts_count < 0) // not initialized
		return 1;
	else if(ts_count == 0) // no data
		return 1;
	else if(ts_count < ts_max_count) // not all data loaded, show ellipsis
		return ts_count+1;
	else // all data loaded
		return ts_count;
}
static void ts_select_click_cb(MenuLayer *ml, MenuIndex *idx, void *context) {
	// TODO: open selected task details
}

static void ts_window_load(Window *wnd) {
	Layer *wnd_layer = window_get_root_layer(wnd);
	GRect bounds = layer_get_bounds(wnd_layer);

	mlTasks = menu_layer_create(bounds);
	menu_layer_set_callbacks(mlTasks, NULL, (MenuLayerCallbacks) {
		.draw_row = ts_draw_row_cb,
		.get_num_rows = ts_get_num_rows_cb,
		.select_click = ts_select_click_cb,
	});
	menu_layer_set_click_config_onto_window(mlTasks, wnd);
	layer_add_child(wnd_layer, menu_layer_get_layer(mlTasks));
}
static void ts_window_unload(Window *wnd) {
	menu_layer_destroy(mlTasks);
}

/* Public functions */

void ts_init() {
	wndTasks = window_create();
	window_set_window_handlers(wndTasks, (WindowHandlers) {
		.load = ts_window_load,
		.unload = ts_window_unload,
	});
	LOG("TaskLists module initialized, window is %p", wndTasks);
}
void ts_deinit() {
	window_destroy(wndTasks);
	for(int i=0; i<ts_count; i++)
		free(ts_items[i].title);
	free(ts_items);
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

	LOG("pushing");
	window_stack_push(wndTasks, true);
	LOG("pushed");
	if(ts_count < 0)
		comm_query_tasks(id);
	LOG("queried");
}
bool ts_is_active() {
	return window_stack_get_top_window() == wndTasks;
}
void ts_set_count(int count) {
	LOG("Setting count: %d", count);
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
