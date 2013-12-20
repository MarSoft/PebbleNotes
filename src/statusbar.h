#ifndef _STATUSBAR_H
#define _STATUSBAR_H

#include <pebble.h>

void sb_init();
void sb_deinit();

void sb_show(char*);
char* sb_printf_alloc(int); // allocate buffer
char* sb_printf_get(); // get (already allocated) buffer
void st_printf_update();
void sb_hide();
void sb_window_disappear_cb(Window*); // must be called in every window's onDisappear

#endif
