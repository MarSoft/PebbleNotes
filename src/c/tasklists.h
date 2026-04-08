#ifndef _TASKLISTS_H
#define _TASKLISTS_H

typedef struct {
	int id;
	char* title;
} TL_Item;

void tl_init();
void tl_deinit();
void tl_show();
bool tl_is_active();
void tl_set_count(int);
void tl_set_item(int, TL_Item);

#endif
