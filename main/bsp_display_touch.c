#include "bsp_display_touch.h"

#include "bsp_display.h"
#include "bsp_touch.h"

#include "esp_log.h"

static const char *TAG = "bsp_display_touch";

void bsp_create_headless_display(void)
{
    bsp_display_create_headless();
}

bool bsp_display_touch_init(void)
{
    if (!bsp_display_init()) {
        return false;
    }

    if (!bsp_touch_init()) {
        ESP_LOGW(TAG, "Touch input not available yet; continuing with display-only mode");
    }

    return true;
}
