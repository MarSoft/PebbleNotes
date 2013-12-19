#include <pebble.h>
#include "comm.h"
#include "tasklists.h"
#include "statusbar.h"

static void init(void) {
	comm_init();
	tl_init();
	tl_show();
	sb_init();
	// others...
}

static void deinit(void) {
	// others...
	sb_deinit();
	tl_deinit();
	comm_deinit();
}

int main(void) {
  init();

  app_event_loop();
  deinit();
}
