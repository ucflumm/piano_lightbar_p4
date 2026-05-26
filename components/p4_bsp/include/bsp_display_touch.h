#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool bsp_display_touch_init(void);
void bsp_create_headless_display(void);

#ifdef __cplusplus
}
#endif
