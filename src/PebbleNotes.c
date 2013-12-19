#include <pebble.h>
#include "comm.h"
#include "tasklists.h"
#include "tasks.h"
#include "statusbar.h"

static void init(void) {
	comm_init();
	tl_init();
	ts_init();
	sb_init();

	tl_show();
	// others...
}

static void deinit(void) {
	// others...
	sb_deinit();
	ts_deinit();
	tl_deinit();
	comm_deinit();
}

int main(void) {
  init();

  app_event_loop();
  deinit();
}
