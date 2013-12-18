#include <pebble.h>
#include "tasklists.h"
#include "common.h"

static Window *wndTasklists;
static MenuLayer *mlTasklists;

static void tl_draw_row_cb(GContext *ctx, const Layer *cell_layer, MenuIndex *idx, void *context) {
	menu_cell_title_draw(ctx, cell_layer, "Hello World");
}
static uint16_t tl_get_num_rows_cb(MenuLayer *ml, uint16_t section_index, void *context) {
	return 2;
}
static void tl_select_click_cb(MenuLayer *ml, MenuIndex *idx, void *context) {
	// TODO: open selected tasklist
}

static void tl_window_load(Window *wnd) {
	Layer *wnd_layer = window_get_root_layer(wnd);
	GRect bounds = layer_get_bounds(wnd_layer);

	mlTasklists = menu_layer_create(bounds);
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

/* Public functions */

void tl_init() {
	wndTasklists = window_create();
	//window_set_click_config_provider(wndTasklists, tl_click_config_provider);
	window_set_window_handlers(wndTasklists, (WindowHandlers) {
		.load = tl_window_load,
		.unload = tl_window_unload,
	});
	LOG("TaskLists module initialized, window is %p", wndTasklists);
}
void tl_show() {
	window_stack_push(wndTasklists, true);
}
void tl_deinit() {
	window_destroy(wndTasklists);
}
