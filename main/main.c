#include <stdint.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

#include "bsp_display_touch.h"
#include "song_player.h"
#include "ui.h"

static const char *TAG = "piano_light_bar";

#define ENABLE_LVGL_RUNTIME 1
#define ENABLE_PANEL_INIT 1
#define ENABLE_UI_RUNTIME 1

void app_main(void)
{
	ESP_LOGI(TAG, "Starting Piano Light Bar milestone firmware");

	if (ENABLE_LVGL_RUNTIME) {
		lv_init();

		if (ENABLE_UI_RUNTIME) {
			if (ENABLE_PANEL_INIT) {
				if (!bsp_display_touch_init()) {
					ESP_LOGW(TAG, "Running with headless LVGL display fallback.");
					bsp_create_headless_display();
				}
			} else {
				ESP_LOGW(TAG, "Panel init disabled for staged bring-up; running LVGL headless mode");
				bsp_create_headless_display();
			}
		} else {
			ESP_LOGW(TAG, "UI runtime disabled; running LVGL headless timer test mode");
			bsp_create_headless_display();
		}
	} else {
		ESP_LOGW(TAG, "LVGL runtime disabled for crash isolation; display/UI updates are paused");
	}

	song_player_t player;
	song_player_load_test_song(&player);

	ui_ctx_t *ui = NULL;
	if (ENABLE_LVGL_RUNTIME && ENABLE_UI_RUNTIME) {
		ui = ui_create(&player);
		if (ui == NULL) {
			ESP_LOGE(TAG, "UI creation failed");
			return;
		}
	} else if (ENABLE_LVGL_RUNTIME) {
		ESP_LOGW(TAG, "UI runtime disabled for staged bring-up");
	}

	// TODO: Plug in LedStripRenderer on GPIO26/GPIO27 when physical strip arrives.
	// TODO: Add WebSocket/MQTT control path from Docker-hosted backend.
	// TODO: Load songs from JSON/MIDI-converted files instead of hardcoded note events.
	// TODO: Add JP1 physical button input for -10s, Play/Pause, Restart, +10s.

	while (true) {
		uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
		song_player_update(&player, now_ms);

		if (ENABLE_LVGL_RUNTIME) {
			if (ENABLE_UI_RUNTIME && ui != NULL) {
			ui_update(ui);
			}

			lv_tick_inc(16);
			lv_timer_handler();
		}

		vTaskDelay(pdMS_TO_TICKS(16));
	}
}
