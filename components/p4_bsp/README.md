# p4_bsp

Local board support component for the ESP32-P4 piano lightbar board.

## Current Scope

- ST7701 MIPI DSI display bring-up
- GT911 touch bring-up with dual-address probe (0x5D / 0x14)
- Shared touch I2C bus management
- LVGL adapter boundary with:
  - active LVGL v8 backend
  - LVGL v9 scaffold hooks for migration

## Public API

- `bsp_display_touch_init()`
- `bsp_create_headless_display()`

Headers are under `include/`.

## Integration

This project links the BSP from the app component via `REQUIRES p4_bsp` in `main/CMakeLists.txt`.

## LVGL 9 Migration Plan

1. Implement the LVGL v9 path in `src/bsp_lvgl_adapter.c`.
2. Keep hardware modules (`bsp_display.c`, `bsp_touch.c`) unchanged.
3. Validate touch/display behavior parity before removing v8 path.

## Future Hardening

- Move board constants to Kconfig/menuconfig options.
- Add runtime deinit lifecycle hooks for display/touch.
- Publish as a standalone repository once API and config are stable.

## TODO

- [ ] Move all board pin/timing constants from `include/bsp_board.h` to Kconfig options.
- [ ] Add optional backlight brightness API (not just on/off).
- [ ] Add full display/touch deinit APIs and lifecycle tests.
- [ ] Implement LVGL v9 backend in `src/bsp_lvgl_adapter.c` and keep parity with v8 behavior.
- [ ] Add standalone BSP example app (`display+touch smoke test`).
- [ ] Add API docs for each public header and expected call order.
- [ ] Verify component manager publishing metadata and versioning strategy.
