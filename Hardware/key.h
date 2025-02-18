#ifndef __KEY_H
#define __KEY_H

#include "my_config.h"

void key_config(void);
void key_scan_10ms_isr(void);
void key_event_handle(void);

#endif
