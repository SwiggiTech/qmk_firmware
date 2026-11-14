// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
     * ┌───┬───┐
     * │lyr│esc│
     * ├───┼───┤
     * │ Z │ X │
     * └───┴───┘
     */
    [0] = LAYOUT(
        TO(1),      KC_ESCAPE,
        KC_Z,       KC_X
    ),
    [1] = LAYOUT(
        TO(2),      KC_ESCAPE,
        KC_LEFT,    KC_RIGHT
    ),
    [2] = LAYOUT(
        TO(3),      KC_ESCAPE,
        KC_VOLD,    KC_VOLU
    ),
    [3] = LAYOUT(
        SWK_MT0,    SWK_CS,
        SWK_CV,     QK_RGB_MATRIX_HUE_UP
    ),
    [4] = LAYOUT(
        _______,    QK_RGB_MATRIX_TOGGLE,
        SWK_CD,     QK_RGB_MATRIX_MODE_NEXT
    )
};
