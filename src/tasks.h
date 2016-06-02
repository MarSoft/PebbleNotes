#ifndef _TASKS_H
#define _TASKS_H

typedef struct {
	int id;
	bool done;
	char* title;
	char* notes;
} TS_Item;

void ts_init();
void ts_deinit();
void ts_show(int, char*);
bool ts_is_active();
int ts_current_listId();
int ts_current_if_complete();
void ts_set_count(int);
void ts_set_item(int, TS_Item);
void ts_append_item(TS_Item);
void ts_update_item_state_by_id(int, bool);

#endif
