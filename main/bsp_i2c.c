#include "bsp_i2c.h"

#include "bsp_board.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "bsp_i2c";

static i2c_master_bus_handle_t s_touch_i2c_bus = NULL;

bool bsp_i2c_touch_bus_init(void)
{
    if (s_touch_i2c_bus != NULL) {
        return true;
    }

    const i2c_master_bus_config_t i2c_config = {
        .i2c_port = TOUCH_I2C_PORT,
        .sda_io_num = PIN_TOUCH_I2C_SDA,
        .scl_io_num = PIN_TOUCH_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
    };

    ESP_LOGI(TAG, "Touch I2C bus init (SDA=%d, SCL=%d)", PIN_TOUCH_I2C_SDA, PIN_TOUCH_I2C_SCL);
    esp_err_t err = i2c_new_master_bus(&i2c_config, &s_touch_i2c_bus);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "touch I2C bus init failed: %s", esp_err_to_name(err));
        s_touch_i2c_bus = NULL;
        return false;
    }

    return true;
}

i2c_master_bus_handle_t bsp_i2c_touch_bus_get(void)
{
    return s_touch_i2c_bus;
}

void bsp_i2c_touch_bus_deinit(void)
{
    if (s_touch_i2c_bus != NULL) {
        i2c_del_master_bus(s_touch_i2c_bus);
        s_touch_i2c_bus = NULL;
    }
}
