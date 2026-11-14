// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
     * ┌───┬───┬───┬───┐
     * │lyr│   │   │esc│
     * ├───┼───┼───┼───┤
     * │ Z │ X │ C │ V │
     * └───┴───┴───┴───┘
     */
    [0] = LAYOUT(
        TO(1),                              KC_ESCAPE,
        KC_Z,       KC_X,       KC_C,       KC_V
    ),
    [1] = LAYOUT(
        TO(2),                              KC_ESCAPE,
        KC_LEFT,    KC_RIGHT,   KC_UP,      KC_DOWN
    ),
    [2] = LAYOUT(
        TO(3),                                          KC_ESCAPE,
        KC_VOLD,    KC_VOLU,    KC_MEDIA_PLAY_PAUSE,    KC_MEDIA_NEXT_TRACK
    ),
    [3] = LAYOUT(
        SWK_MT0,                                        SWK_CD,
        SWK_CV,     QK_RGB_MATRIX_HUE_UP,   SWK_CS,     QK_RGB_MATRIX_MODE_NEXT
    ),
    [4] = LAYOUT(
        KC_TRANSPARENT,                                         QK_RGB_MATRIX_TOGGLE,
        KC_F21,         KC_F22,                 KC_F23,         KC_F24
    )
};
