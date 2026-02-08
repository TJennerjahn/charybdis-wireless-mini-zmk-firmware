/*
 * Battery print behavior
 *
 * Types the split peripheral battery levels into the host as keystrokes.
 * Intended for dongle-central builds where the host is connected via USB.
 */

#define DT_DRV_COMPAT zmk_behavior_battery_print

#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT) && IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL) &&                               \
    IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
#include <zmk/split/central.h>
#endif

#include <dt-bindings/zmk/keys.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if (!IS_ENABLED(CONFIG_ZMK_SPLIT)) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <zmk/behavior_queue.h>

static void tap_key(struct zmk_behavior_binding_event *event, uint32_t keycode) {
    // Device name for the built-in key press behavior.
    struct zmk_behavior_binding kp = {
        .behavior_dev = "key_press",
        .param1 = keycode,
        .param2 = 0,
    };

    // A small delay keeps the host from missing fast sequences.
    zmk_behavior_queue_add(event, kp, true, 20);
    zmk_behavior_queue_add(event, kp, false, 20);
}

static uint32_t digit_keycode(uint8_t d) {
    switch (d) {
    case 0:
        return N0;
    case 1:
        return N1;
    case 2:
        return N2;
    case 3:
        return N3;
    case 4:
        return N4;
    case 5:
        return N5;
    case 6:
        return N6;
    case 7:
        return N7;
    case 8:
        return N8;
    case 9:
        return N9;
    default:
        return QUESTION;
    }
}

static void tap_u8_dec_0_100(struct zmk_behavior_binding_event *event, uint8_t v) {
    if (v > 100) {
        tap_key(event, QUESTION);
        tap_key(event, QUESTION);
        return;
    }

    if (v == 100) {
        tap_key(event, N1);
        tap_key(event, N0);
        tap_key(event, N0);
        return;
    }

    if (v >= 10) {
        tap_key(event, digit_keycode(v / 10));
        tap_key(event, digit_keycode(v % 10));
        return;
    }

    tap_key(event, digit_keycode(v));
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);

    uint8_t left = 0;
    uint8_t right = 0;
    bool have_left = false;
    bool have_right = false;

#if IS_ENABLED(CONFIG_ZMK_SPLIT) && IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL) &&                               \
    IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
    have_left = (zmk_split_central_get_peripheral_battery_level(0, &left) == 0);
    have_right = (zmk_split_central_get_peripheral_battery_level(1, &right) == 0);
#endif

    // Format: "l:87% r:92%"
    tap_key(&event, L);
    tap_key(&event, COLON);
    if (have_left) {
        tap_u8_dec_0_100(&event, left);
    } else {
        tap_key(&event, QUESTION);
        tap_key(&event, QUESTION);
    }
    tap_key(&event, PRCNT);
    tap_key(&event, SPACE);

    tap_key(&event, R);
    tap_key(&event, COLON);
    if (have_right) {
        tap_u8_dec_0_100(&event, right);
    } else {
        tap_key(&event, QUESTION);
        tap_key(&event, QUESTION);
    }
    tap_key(&event, PRCNT);

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return ZMK_BEHAVIOR_OPAQUE;
}

#else
// Split peripherals don't include the behavior queue (or HID output), so this behavior is a no-op there.
static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return ZMK_BEHAVIOR_OPAQUE;
}
#endif

static const struct behavior_driver_api behavior_battery_print_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define BATT_PRINT_INST(n)                                                                           \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                  \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_battery_print_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BATT_PRINT_INST)
