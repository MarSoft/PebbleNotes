#include <pebble.h>
#include "tasklists.h"
#include "comm.h"

static void init(void) {
	comm_init();
	tl_init();
	tl_show();
	// others...
}

static void deinit(void) {
	// others...
	tl_deinit();
	comm_deinit();
}

int main(void) {
  init();

  app_event_loop();
  deinit();
}
