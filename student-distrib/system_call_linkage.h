#ifndef SYSTEM_CALL_LINKAGE_H
#define SYSTEM_CALL_LINKAGE_H

#include "system_call.h"
#include "x86_desc.h"

extern void system_call_link(void);

extern void switch_to_user(uint32_t);

extern void flush_TLB(void);

#endif
