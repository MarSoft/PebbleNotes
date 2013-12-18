#include <pebble.h>
#include "consts.h"
#include "misc.h"
#include "tasklists.h"
#include "comm.h"

void in_dropped_handler(AppMessageResult reason, void *context) {
	LOG("Message dropped: reason=%d", reason);
}
void out_sent_handler(DictionaryIterator *sent, void *context) {
	LOG("Message sent");
}
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	LOG("Message send failed: reason=%d", reason);
}

static void init(void) {
	app_message_register_inbox_received(comm_in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);

	app_message_open(app_message_inbox_size_maximum(), APP_MESSAGE_OUTBOX_SIZE_MINIMUM); // We only need large buffer for inbox

	tl_init();
	tl_show();
	// others...
}

static void deinit(void) {
	// others...
	tl_deinit();
}

int main(void) {
  init();

  app_event_loop();
  deinit();
}
