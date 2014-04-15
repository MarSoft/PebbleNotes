#include <pebble.h>
#include "taskinfo.h"
#include "tasks.h"
#include "comm.h"
#include "misc.h"
#include "statusbar.h"
#include "options.h"

static Window *wndTaskInfo;
// ...

static int taskId = -1;
static TS_Item currentTask;

/* Private functions */

static void ti_window_load(Window *wnd) {
}
static void ti_window_unload(Window *wnd) {
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
