#include <pebble.h>
#include "options.h"
#include "consts.h"
#include "misc.h"

static bool large_font = false;
static int task_actions_position = TaskActionsPositionBottom;

void options_init() {
	if(persist_exists(OPTION_LARGE_FONT))
		large_font = persist_read_bool(OPTION_LARGE_FONT);
	if(persist_exists(OPTION_TASK_ACTIONS_POSITION))
		task_actions_position = persist_read_int(OPTION_TASK_ACTIONS_POSITION);
}
void options_deinit() {
}
void options_update(int key, int val) {
	switch(key) {
		case OPTION_LARGE_FONT:
			if(val < false || val > true) {
				LOG("Wrong value, trimming");
				val %= 2;
			}
			large_font = val;
			persist_write_bool(key, val);
			break;
		case OPTION_TASK_ACTIONS_POSITION:
			if(val < 0 || val > 2) {
				LOG("Wrong value, trimming");
				val %= 3;
			}
			task_actions_position = val;
			persist_write_int(key, val);
			break;
		default:
			LOG("Unknown key %d", key);
			break;
	}
}

bool options_large_font() {
	return large_font;
}

int options_task_actions_position() {
	return task_actions_position;
}
