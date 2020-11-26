#pragma once

#include <stdint.h>

//----------------------------------
// define some constants
//----------------------------------

#define LOW     0
#define HIGH    1
#define OUTPUT  0
#define INPUT   1

//----------------------------------
// define display commands
//----------------------------------
#define PANEL_SETTING 0x00
#define POWER_SETTING 0x01
#define POWER_OFF 0x02
#define POWER_OFF_SEQUENCE_SETTING 0x03
#define POWER_ON 0x04
#define DISPLAY_REFRESH 0x12
#define PARTIAL_DISPLAY_REFRESH 0x15
#define DATA_START_TRANSMISSION_1 0x18
#define DATA_START_TRANSMISSION_2 0x1c
#define LUT_FOR_VCOM 0x20
#define LUT_CMD_0x23 0x23
#define LUT_CMD_0x24 0x24
#define LUT_CMD_0x25 0x25
#define LUT_CMD_0x26 0x26
#define PLL_CONTROL 0x30

//----------------------------------
// define pins
//----------------------------------
#define SPI_ENABLE  GPIO_PD2
#define SPI_MOSI    GPIO_PB7
#define IO_RST_N    GPIO_PA6
#define SPI_CLOCK   GPIO_PD7
#define IO_BUSY_N   GPIO_PA5
// Unknown signal, but should be high
#define EPD_TO_PC4  GPIO_PC4

//----------------------------------
// define groups of segments into logical shapes
//----------------------------------
#define TOP_LEFT_1 1
#define TOP_LEFT 2
#define TOP_MIDDLE 3
#define TOP_RIGHT 4
#define BOTTOM_LEFT 5
#define BOTTOM_RIGHT 6
#define BACKGROUND 7
#define BATTERY_LOW 8
#define DASHES 9
#define FACE 10
#define FACE_SMILE 11
#define FACE_FROWN 12
#define FACE_NEUTRAL 13
#define SUN 14
#define FIXED 15
#define FIXED_DEG_C 16
#define FIXED_DEG_F 17
#define MINUS 18
#define ATC 19

typedef struct XiaomiMiaoMiaoCeBT {
    // boolean to indicate if the screen is inverted or not
    uint8_t inverted;
    uint8_t transition;
    // The array in which the segments to be displayed are placed
    uint8_t display_data[18];    
} XiaomiMiaoMiaoCeBT;

// TODO - cleanup - function prototypes from lcd
void show_atc_mac(XiaomiMiaoMiaoCeBT* c);
void show_temp_symbol(XiaomiMiaoMiaoCeBT* c, uint8_t symbol);
void show_big_number(XiaomiMiaoMiaoCeBT* c, int16_t number);
void show_small_number(XiaomiMiaoMiaoCeBT* c, uint16_t number);
void show_battery_symbol(XiaomiMiaoMiaoCeBT* c, uint8_t state);
void show_ble_symbol(XiaomiMiaoMiaoCeBT* c, uint8_t state);
void show_smiley(XiaomiMiaoMiaoCeBT* c, uint8_t state);
// TODO ^^----
void epd_unset_shape(XiaomiMiaoMiaoCeBT* c, uint8_t where);


/**
 * Initialize the display
 * @param redraw    should the screen should have an black-white redraw?
 *                  redraw = 0: no redraw
 *                  redraw != 0: do a redraw 
 *                  normally redrawing is advisable (redraw != 0), but e.g. 
 *                  when using deep-sleep you may want to initialize the 
 *                  screen without the black-white transition
 */
void epd_init(XiaomiMiaoMiaoCeBT* i, uint8_t redraw);

/** display the data
 */
void epd_write_display(XiaomiMiaoMiaoCeBT* i);

/** 
 * Send ready data buffer to the display
 * @param data      the data containing which segments are on or off
*/
void epd_write_display_data(XiaomiMiaoMiaoCeBT* i, uint8_t *data);

/** 
 * Display a digit at a specific place
 * @param number    The digit to be displayed [0 to 9, a - f]
 * @param where     The location where to display the digit, there are 5 
 *                  locations: [TOP_LEFT, TOP_MIDDLE, TOP_RIGHT, 
 *                  BOTTOM_LEFT, BOTTOM_RIGHT]
 */
void epd_set_digit(XiaomiMiaoMiaoCeBT* i, uint8_t digit, uint8_t where);

/** 
 * Display a defined shape
 * @param where     Which shapes should be turned on, there are several 
 *                  pre-defined shapes: [TOP_LEFT_1, BACKGROUND, 
 *                  BATTERY_LOW, DASHES, FACE, FACE_SMILE, FACE_FROWN, 
 *                  FACE_NEUTRAL, SUN, FIXED]
 *                  But if you want you can define your own, see the 
 *                  #defines above and the variables in XiaomiMiaoMiaoCe.cpp
 */
void epd_set_shape(XiaomiMiaoMiaoCeBT* i, uint8_t where);

/**
 * Set a segment to a value
 * The e-ink display has 73 segments (the driver can handle 120) which are
 * distributed over 15 bytes with 8 bits each: 15*8 = 120
 * @param segment_byte  The byte in which the segments is located
 * @param segment_bit   The bit in the byte
 * @param value         0 = off, 1 = on
 */
void epd_set_segment(XiaomiMiaoMiaoCeBT* i, uint8_t segment_byte, uint8_t segment_bit, uint8_t value);

/** 
 * Start building a new display
 * @param inverted  0 = inverted display, 1 = normal
*/
void epd_start_new_screen(XiaomiMiaoMiaoCeBT* i);
void epd_start_new_screen_inverted(XiaomiMiaoMiaoCeBT* i, uint8_t inverted);

/**
 * Transmit data to the display via SPI
 * @param cd    is it a command or data? 0 = command, 1 = data
 * @param data  1 byte of data
 */
void transmit(uint8_t cd, uint8_t data);

/** 
 * Send data to the display
 * The e-ink display has 'waveforms' of voltages to be applied to the 
 * segments when changing from black to white or white to black. These
 * differ between initialising and updating the e-ink display
 * @param dataV     one of the waveforms
 * @param dataKK    one of the waveforms
 * @param dataKW    one of the waveforms
 * @param data      the data containing which segments are on or off
 * @param is_init   is this an initialisation or an update
 */
void send_sequence(XiaomiMiaoMiaoCeBT* i, uint8_t *dataV, uint8_t *dataKK, uint8_t *dataKW,
                   uint8_t *data, uint8_t is_init);