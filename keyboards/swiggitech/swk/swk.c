#include QMK_KEYBOARD_H

void board_init(void)
{
    // Remap PA11->PA9 and PA12->PA10 for USB
    // This is REQUIRED FOR THE STM32F042 FAMILY
    SYSCFG->CFGR1 |= SYSCFG_CFGR1_PA11_PA12_RMP;
}

//
// Use custom scanning if inverted pins are specified. Inverted pins will be
// pulldown instead of pullup.
//
// Custom scanning only works with direct pins.
//
// Example:
//      #define SWK_SCAN_INV_PINS { B8 }
//
#ifdef SWK_SCAN_INV_PINS

// 2D array of all direct pins
static pin_t const pins[MATRIX_ROWS][MATRIX_COLS] = DIRECT_PINS;

// 1D array of pins that should be inverted (pulldown)
static pin_t const inv_pins[MATRIX_ROWS] = SWK_SCAN_INV_PINS;

// Array of bits to start with for each row when scanning. The bit for an
// inverted pin is a 0 instead of a 1.
//
// The appropriate bits are set to 1 in matrix_init_custom().
static matrix_row_t initial_row_bits[MATRIX_ROWS] = { 0 };

// Returns true if the provided pin is found in inv_pins (should be inverted)
static bool should_invert_pin(pin_t pin)
{
    for (int i = 0; i < sizeof(inv_pins) / sizeof(inv_pins[0]); i++) {
        if (pin == inv_pins[i])
            return true;
    }
    return false;
}

// CUSTOM_LITE init function
void matrix_init_custom(void) {
    // For each row, column...
    for (int r = 0; r < MATRIX_ROWS; r++) {
        for (int c = 0; c < MATRIX_COLS; c++) {
            if (pins[r][c] == NO_PIN)
                continue;
            if (should_invert_pin(pins[r][c])) { // Inverted (pulldown)
                gpio_set_pin_input_low(pins[r][c]);
            }
            else { // Not inverted (pullup)
                gpio_set_pin_input_high(pins[r][c]);
                initial_row_bits[r] |= 1 << c; // Bit in row starts as 1
            }
        }
    }
}

// CUSTOM_LITE scan function
bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool changed = false;
    for (int r = 0; r < MATRIX_ROWS; r++) {
        // initial_row_bits[r] has 0s instead of 1s for each inverted pin so
        // that we don't need to do anything special when we read them
        matrix_row_t row = initial_row_bits[r];
        for (int c = 0; c < MATRIX_COLS; c++) {
            if (pins[r][c] == NO_PIN)
                continue;
            // Flip this bit if the pin is high
            row ^= ((matrix_row_t)!!gpio_read_pin(pins[r][c])) << c;
        }
        changed = changed || (row != current_matrix[r]); // Check if row state changed
        current_matrix[r] = row; // Output new row state
    }
    return changed;
}

#endif

//
// Handling for SWK custom keycodes
//
#ifdef SWK_PROCESS_CUSTOM_KEYCODES

// Handle SWK MOTAP keycodes. These enable the next layer while held and switch
// to the layer in their name (0/1/2/3) when tapped.
static bool swk_pck_process_record_motap(uint16_t keycode, keyrecord_t *record)
{
    static uint16_t prev_pressed_keycode = 0;
    static uint16_t prev_pressed_time = 0;

    uint8_t tap_dest; // Destination if we tapped this key
    switch (keycode) {
    case SWK_LAYER_MOTAP0:
        tap_dest = 0;
        break;
    case SWK_LAYER_MOTAP1:
        tap_dest = 1;
        break;
    case SWK_LAYER_MOTAP2:
        tap_dest = 2;
        break;
    case SWK_LAYER_MOTAP3:
        tap_dest = 3;
        break;
    default:
        // Return early if this is not a MOTAP keycode
        prev_pressed_keycode = keycode;
        prev_pressed_time = record->event.time;
        return true;
    }

    // This press/release was a MOTAP keycode

    if (record->event.pressed) {
        // On initial keypress, activate the layer above the this key's layer
        layer_on(layer_switch_get_layer(record->event.key) + 1);
    }
    else {
        // TODO Customize delay? Use
        if (prev_pressed_keycode != keycode || record->event.time - prev_pressed_time > 250) {
            // Another key was pressed, or this key was held too long.
            // Deactivate the layer above the this key's layer.
            layer_off(layer_switch_get_layer(record->event.key) + 1);
        }
        else {
            // This was a tap. Move to the tap destination layer.
            layer_move(tap_dest);
        }
    }

    prev_pressed_keycode = keycode;
    prev_pressed_time = record->event.time;
    return false;
}

// Handler for custom keycodes
static bool swk_pck_process_record_kb(uint16_t keycode, keyrecord_t *record)
{
    // MOTAP codes get their own function
    if (!swk_pck_process_record_motap(keycode, record))
        return false;

    switch (keycode) {
    case SWK_CYCLE_HSV_VAL:
        if (record->event.pressed) {
            rgb_matrix_sethsv(rgb_matrix_get_hue(), rgb_matrix_get_sat(), (rgb_matrix_get_val() + 16) % 256);
        }
        return false;
    case SWK_CYCLE_HSV_SAT:
        if (record->event.pressed) {
            rgb_matrix_sethsv(rgb_matrix_get_hue(), (rgb_matrix_get_sat() + 16) % 256, rgb_matrix_get_val());
        }
        return false;
    case SWK_CYCLE_ANIM_SPEED:
        if (record->event.pressed) {
            rgb_matrix_set_speed((rgb_matrix_get_speed() + 16) % 256);
        }
        return false;
    default:
        // Process all other keycodes normally
        return true;
    }
}

#endif

//
// Treat keys that have the same keycode like a single key. Only send press
// events if the keycode is not already pressed. Only send release events when
// the last key is released.
//
#ifdef SWK_MERGE_DUPLICATE_KEYS

static bool swk_mdk_process_record_kb(uint16_t keycode, keyrecord_t *record) {
    // TODO Is it okay to use 0 to indicate not pressed?
    static uint16_t active_keycodes[MATRIX_ROWS][MATRIX_COLS] = { 0 };

    if (record->event.pressed)
        active_keycodes[record->event.key.row][record->event.key.col] = keycode;
    else
        active_keycodes[record->event.key.row][record->event.key.col] = 0;

    for (int r = 0; r < MATRIX_ROWS; r++) {
        for (int c = 0; c < MATRIX_COLS; c++) {
            if (r == record->event.key.row && c == record->event.key.col)
                continue; // Skip this key
            if (active_keycodes[r][c] == keycode)
                return false; // Keycode pressed by other key, ignore event
        }
    }
    return true;
}

#endif

//
// Display layer changes using RGB matrix LEDs
//
#ifdef SWK_LAYER_LED_INDICES

// TODO Configure in a .h file somewhere?
#define LAYER_LED_TIMEOUT 1024
#define LAYER_LED_BLINK_INTERVAL 256
static uint8_t const layer_led_indices[] = SWK_LAYER_LED_INDICES;
static uint32_t layer_leds_time = 0;
static bool layer_leds_on = false;

layer_state_t layer_state_set_kb(layer_state_t state)
{
    layer_leds_on = true;
    layer_leds_time = timer_read32();
    return state;
}

bool rgb_matrix_indicators_kb(void) {
    if (!rgb_matrix_indicators_user()) {
        return false;
    }

    if (!layer_leds_on)
        return true;

    // TODO Is this too much logic here?
    uint32_t t = timer_elapsed32(layer_leds_time);
    if (t >= LAYER_LED_TIMEOUT) {
        layer_leds_on = false;
    }
    else {
        // TODO Could we make this customizable?

        // Decide color for blinking LED(s)
        rgb_t rgb = { 0, 0, 0 }; // Off during even interval
        if ((t / (LAYER_LED_BLINK_INTERVAL / 2)) & 1) {
            // On during odd interval
            hsv_t hsv = { rgb_matrix_get_hue(), rgb_matrix_get_sat(), rgb_matrix_get_val() };
            if (hsv.v < 255 - 64) {
                // Blink brighter if there is headroom
                hsv.v += 64;
            }
            else if (hsv.v < 32) {
                // Blink a minimum amount if brightness is low
                hsv.v = 32;
            }
            rgb = hsv_to_rgb(hsv);
        }

        // Apply the selected color LEDs
        for (uint8_t i = 0; i < sizeof(layer_led_indices) / sizeof(layer_led_indices[0]); i++) {
            if (IS_LAYER_ON(i)) {
                rgb_matrix_set_color(layer_led_indices[i], rgb.r, rgb.b, rgb.b);
            }
        }
    }

    return true;
}

#endif

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {

#ifdef SWK_MERGE_DUPLICATE_KEYS
    if (swk_mdk_process_record_kb(keycode, record) == false)
        return false;
#endif

#ifdef SWK_PROCESS_CUSTOM_KEYCODES
    if (swk_pck_process_record_kb(keycode, record) == false)
        return false;
#endif

    // Process event as normal
    return true;
}
