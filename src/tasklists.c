#include <pebble.h>
#include "tasklists.h"
#include "comm.h"
#include "tasks.h"
#include "statusbar.h"
#include "misc.h"

static Window *wndTasklists;
static MenuLayer *mlTasklists;

static int tl_count = -1; // how many items were loaded ATM
static int tl_max_count = -1; // how many items we are expecting (i.e. buffer size)
static TL_Item *tl_items = NULL; // buffer for items

static void tl_draw_row_cb(GContext *ctx, const Layer *cell_layer, MenuIndex *idx, void *context) {
	char *title;
	if(tl_max_count == 0) // empty list
		title = "No tasklists! Please create one from phone/PC";
	else if(idx->row >= tl_count) // no such item (yet?)
		title = "<...>";
	else
		title = tl_items[idx->row].title;
#ifdef PBL_ROUND
	// this uses smaller font but properly centers text
	menu_cell_basic_draw(ctx, cell_layer, title, NULL, NULL);
#else
	// this doesn't center text but uses bigger font which is good
	menu_cell_title_draw(ctx, cell_layer, title);
#endif
}
static uint16_t tl_get_num_rows_cb(MenuLayer *ml, uint16_t section_index, void *context) {
	if(tl_count < 0) // not initialized
		return 0; // statusbar must already contain "Connecting..." message
	else if(tl_count == 0) // no data
		return 1;
	else
		return tl_count;
}
static void tl_select_click_cb(MenuLayer *ml, MenuIndex *idx, void *context) {
	assert(idx->row < tl_count, "Invalid index!"); // this will fire when there are no any lists loaded
	TL_Item sel = tl_items[idx->row];
	if(sel.id == ts_current_listId() || comm_is_available()) // already loaded or may be loaded
		ts_show(sel.id, sel.title);
	// or else message will be shown
}

static void tl_window_load(Window *wnd) {
	Layer *wnd_layer = window_get_root_layer(wnd);
	GRect bounds = layer_get_bounds(wnd_layer);

	mlTasklists = menu_layer_create(bounds);
	assert_oom(mlTasklists, "OOM while creating menu layer");
	menu_layer_set_callbacks(mlTasklists, NULL, (MenuLayerCallbacks) {
		.draw_row = tl_draw_row_cb,
		.get_num_rows = tl_get_num_rows_cb,
		.select_click = tl_select_click_cb,
	});
	menu_layer_set_click_config_onto_window(mlTasklists, wnd);
	layer_add_child(wnd_layer, menu_layer_get_layer(mlTasklists));
}
static void tl_window_unload(Window *wnd) {
	menu_layer_destroy(mlTasklists);
}
static void tl_free_items() {
	for(int i=0; i<tl_count; i++)
		free(tl_items[i].title);
	free(tl_items);
	tl_items = NULL;
}

/* Public functions */

void tl_init() {
	wndTasklists = window_create();
	assert_oom(wndTasklists, "OOM while creating window");
	//window_set_click_config_provider(wndTasklists, tl_click_config_provider);
	window_set_window_handlers(wndTasklists, (WindowHandlers) {
		.load = tl_window_load,
		.disappear = sb_window_disappear_cb,
		.unload = tl_window_unload,
	});
	LOG("TaskLists module initialized, window is %p", wndTasklists);
}
void tl_deinit() {
	window_destroy(wndTasklists);
	tl_free_items();
}
void tl_show() {
	window_stack_push(wndTasklists, true);
	sb_show("Starting...");
	if(tl_count < 0)
		comm_query_tasklists();
}
bool tl_is_active() {
	return window_stack_get_top_window() == wndTasklists;
}
void tl_set_count(int count) {
	LOG("Setting count: %d", count);
	if(tl_items)
		tl_free_items();
	tl_items = malloc(sizeof(TL_Item)*count);
	tl_max_count = count;
	tl_count = 0;
	if(!tl_items) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "OOM while allocating tasklists items");
		sb_show("OOM");
		tl_max_count = 0;
	}
}
void tl_set_item(int i, TL_Item data) {
	LOG("New item %d", i);
	assert(tl_max_count > 0, "Trying to set item while not initialized!");
	assert(tl_max_count > i, "Unexpected item index: %d, max count is %d", i, tl_max_count);
	
	tl_items[i].id = data.id;
	tl_items[i].size = data.size;
	tl_items[i].title = malloc(strlen(data.title)+1);
	if(tl_items[i].title) {
		strcpy(tl_items[i].title, data.title);
	} else {
		assert_oom(false, "OOM while allocating tasklist item title");
	}
	tl_count++;
	menu_layer_reload_data(mlTasklists);
	LOG("Current count is %d", tl_count);
}
