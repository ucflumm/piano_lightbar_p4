#pragma once

#include <stdbool.h>

#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

bool bsp_i2c_touch_bus_init(void);
i2c_master_bus_handle_t bsp_i2c_touch_bus_get(void);
void bsp_i2c_touch_bus_deinit(void);

#ifdef __cplusplus
}
#endif
