#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool bsp_display_init(void);
void bsp_display_create_headless(void);

#ifdef __cplusplus
}
#endif
