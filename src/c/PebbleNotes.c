#include <pebble.h>
#include "comm.h"
#include "tasklists.h"
#include "tasks.h"
#include "taskinfo.h"
#include "statusbar.h"
#include "options.h"

static void init(void) {
	options_init();
	comm_init();
	tl_init();
	ts_init();
	ti_init();
	sb_init();

	tl_show();
	// others...
}

static void deinit(void) {
	// others...
	sb_deinit();
	ti_deinit();
	ts_deinit();
	tl_deinit();
	comm_deinit();
	options_deinit();
}

int main(void) {
  init();

  app_event_loop();
  deinit();
}
