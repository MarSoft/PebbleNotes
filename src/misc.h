#ifndef _COMMON_H
#define _COMMON_H

#define LOG(args...) APP_LOG(APP_LOG_LEVEL_DEBUG, args)
#define assert(e, msg...) if(!(e)) { APP_LOG(APP_LOG_LEVEL_ERROR, msg); return; }
#define assert_oom(e, msg...) if(!(e)) { APP_LOG(APP_LOG_LEVEL_ERROR, msg); sb_show("OOM"); }

#endif
