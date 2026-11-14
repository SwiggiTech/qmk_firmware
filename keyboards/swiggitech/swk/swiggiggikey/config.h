#pragma once

//
// Normal QMK defines
//

// https://mm.digikey.com/Volume0/opasdata/d220001/medias/docus/8903/5962_XL-2020RGBC-WS2812B.pdf

// WS2812 configuration that is not in keyboard.json
#define WS2812_TIMING   1250
#define WS2812_T0H      250
#define WS2812_T1H      850
#define WS2812_TRST_US  100
#define WS2812_BYTE_ORDER WS2812_BYTE_ORDER_GRB

//
// SWK defines (see swk.c)
//

// Use custom direct scanning with inverted pin B8
#define SWK_SCAN_INV_PINS { B8 }

// Enable custom keycodes for brightness/saturation cycling and MOTAP
#define SWK_PROCESS_CUSTOM_KEYCODES

// Treat keys with the same keycode as a single key
#define SWK_MERGE_DUPLICATE_KEYS

// Enable layer indication using these RGB matrix LEDs
#define SWK_LAYER_LED_INDICES { 1, 2, 3, 4 }
