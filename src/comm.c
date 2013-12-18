#include <pebble.h>
#include "comm.h"
#include "misc.h"
#include "consts.h"
#include "tasklists.h"

void comm_query_tasklists() {
	LOG("Querying tasklists");
	DictionaryIterator *iter;
	Tuplet code = TupletInteger(KEY_CODE, CODE_GET);
	Tuplet scope = TupletInteger(KEY_SCOPE, SCOPE_LISTS);

	app_message_outbox_begin(&iter);
	dict_write_tuplet(iter, &code);
	dict_write_tuplet(iter, &scope);
	app_message_outbox_send();
}
void comm_query_tasks(int listId) {
	LOG("Querying tasks for %d (not implemented)", listId);
}
void comm_query_task_details(int listId, int taskId) {
	LOG("Querying task details for %d, %d (not implemented)", listId, taskId);
}

void comm_in_received_handler(DictionaryIterator *iter, void *context) {
	Tuple *tCode, *tMessage, *tScope, *tCount;

	tCode = dict_find(iter, KEY_CODE);
	int code = (int)tCode->value->int32;
	LOG("Message code: %d", code);

	if(code == CODE_ERROR) {
		tMessage = dict_find(iter, KEY_ERROR);
		char* message = "Unknown error";
		if(tMessage && tMessage->type == TUPLE_CSTRING)
			message = tMessage->value->cstring;
		LOG("Error received: %s", message);
		// TODO: display error message
		return;
	}

	tScope = dict_find(iter, KEY_SCOPE);
	int scope = (int)tScope->value->int32;
	LOG("Message scope: %d", scope);

	if(code == CODE_ARRAY_START) {
		tCount = dict_find(iter, KEY_COUNT);
		int count = (int)tCount->value->int32;
		LOG("Items count: %d", count);
		if(scope == SCOPE_LISTS)
			tl_set_count(count);
		else // others...
			LOG("Unexpected scope: %d", scope);
	} else if(code == CODE_ARRAY_ITEM) {
		int i = (int)dict_find(iter, KEY_ITEM)->value->int32;
		int listId = (int)dict_find(iter, KEY_LISTID)->value->int32;
		char *title = dict_find(iter, KEY_TITLE)->value->cstring;
		LOG("Item No: %d, Id=%d", i, listId);
		tl_set_item(i, (TL_Item){
			.id = listId,
			.title = title,
			.size = -1,
		});
	} else if(code == CODE_ARRAY_END) {
	} else
		LOG("Unexpected message code: %d", code);
}
