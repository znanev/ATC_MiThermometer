#include "epd.h"
#include "tl_common.h"
#include "drivers/8258/pm.h"
#include "drivers/8258/timer.h"
#include "app_config.h"

uint8_t T_DTM_init[18] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t T_DTM2_init[18] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

//----------------------------------
// LUTV, LUT_KK and LUT_KW values taken from the actual device with a
// logic analyzer
//----------------------------------
uint8_t T_LUTV_init[15] = {0x47, 0x47, 0x01, 0x87, 0x87, 0x01, 0x47, 0x47, 0x01, 0x87, 0x87, 0x01, 0x81, 0x81, 0x01};
uint8_t T_LUT_KK_init[15] = {0x87, 0x87, 0x01, 0x87, 0x87, 0x01, 0x47, 0x47, 0x01, 0x47, 0x47, 0x01, 0x81, 0x81, 0x01};
uint8_t T_LUT_KW_init[15] = {0x47, 0x47, 0x01, 0x47, 0x47, 0x01, 0x87, 0x87, 0x01, 0x87, 0x87, 0x01, 0x81, 0x81, 0x01};
uint8_t T_LUT_KK_update[15] = {0x87, 0x87, 0x01, 0x87, 0x87, 0x01, 0x87, 0x87, 0x01, 0x87, 0x87, 0x01, 0x81, 0x81, 0x01};
uint8_t T_LUT_KW_update[15] = {0x47, 0x47, 0x01, 0x47, 0x47, 0x01, 0x47, 0x47, 0x01, 0x47, 0x47, 0x01, 0x81, 0x81, 0x01};

//----------------------------------
// define segments
// the data in the arrays consists of {byte, bit} pairs of each segment
//----------------------------------
uint8_t top_left_1[2] = {12, 3};
uint8_t top_left[22] = {16, 7, 15, 4, 14, 1, 14, 7, 12, 5, 12, 4, 13, 3, 15, 7, 15, 6, 15, 5, 14, 0};
uint8_t top_middle[22] = {15, 0, 15, 1, 14, 6, 13, 0, 13, 5, 13, 4, 14, 5, 14, 4, 15, 3, 15, 2, 14, 3};
uint8_t top_right[22] = {13, 1, 13, 7, 12, 1, 12, 7, 11, 5, 11, 2, 12, 6, 12, 0, 13, 6, 13, 2, 12, 2};
uint8_t bottom_left[22] = {9, 1, 9, 7, 8, 5, 1, 1, 0, 3, 1, 4, 9, 4, 10, 0, 10, 6, 10, 3, 8, 2};
uint8_t bottom_right[22] = {7, 7, 6, 5, 2, 0, 2, 3, 0, 2, 1, 7, 2, 6, 7, 4, 7, 1, 8, 6, 6, 2};
uint8_t battery_low[2] = {16, 4};
uint8_t dashes[4] = {16, 6, 14, 2};
uint8_t face[14] = {3, 5, 5, 3, 5, 6, 4, 1, 4, 4, 4, 7, 3, 2};
uint8_t face_smile[8] = {4, 1, 5, 6, 3, 2, 4, 7};
uint8_t face_frown[6] = {5, 6, 4, 4, 4, 1};
uint8_t face_neutral[6] = {5, 6, 4, 1, 4, 7};
uint8_t sun[2] = {5, 0};
uint8_t fixed[2] = {16, 5};
uint8_t fixed_deg_C[2] = {14, 2};
uint8_t fixed_deg_F[2] = {16, 6};
uint8_t minus[2] = {14, 0};
uint8_t Atc[48] = {16, 7, 15, 4, 14, 1, 14, 7, 12, 4, 13, 3, 15, 7, 15, 6, 15, 5, 14, 0, 13, 5, 13, 4, 14, 5, 14, 4, 15, 3, 15, 2, 14, 3, 13, 1, 11, 5, 11, 2, 12, 6, 12, 0, 13, 6, 13, 2};

// These values closely reproduce times captured with logic analyser
uint8_t delay_SPI_clock_pulse = 4;
uint8_t delay_SPI_end_cycle = 8;

/*

Now define how each digit maps to the segments:

          1
 10 :-----------
    |           |
  9 |           | 2
    |     11    |
  8 :-----------: 3
    |           |
  7 |           | 4
    |     5     |
  6 :----------- 

*/

int digits[16][11] = {
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0},  // 0
    {2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0},   // 1
    {1, 2, 3, 5, 6, 7, 8, 10, 11, 0, 0}, // 2
    {1, 2, 3, 4, 5, 6, 10, 11, 0, 0, 0}, // 3
    {2, 3, 4, 8, 9, 10, 11, 0, 0, 0, 0}, // 4
    {1, 3, 4, 5, 6, 8, 9, 10, 11, 0, 0}, // 5
    {1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0}, // 6
    {1, 2, 3, 4, 10, 0, 0, 0, 0, 0, 0},  // 7
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, // 8
    {1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 0}, // 9
    {1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 0}, // A
    {3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 0}, // b
    {5, 6, 7, 8, 11, 0, 0, 0, 0, 0, 0},  // c
    {2, 3, 4, 5, 6, 7, 8, 11, 0, 0, 0},  // d
    {1, 5, 6, 7, 8, 9, 10, 11, 0, 0, 0}, // E
    {1, 6, 7, 8, 9, 10, 11, 0, 0, 0, 0}  // F
};

// Wrap some Telink GPIO-related functions into Arduino-like functions,
// in order to keep the code close to the original at:
// https://github.com/znanev/MHO-C401
void pinMode(uint16_t pin, uint8_t mode)
{
    gpio_set_func(pin, AS_GPIO);    //enable GPIO func
    if (mode == OUTPUT)
    {
        gpio_set_input_en(pin, 0);  // disable input
        gpio_set_output_en(pin, 1); // enable output
    }
    else if (mode == INPUT)
    {
        gpio_set_input_en(pin, 1);  //enable input
        gpio_set_output_en(pin, 0); //disable output
    }
}

void digitalWrite(uint16_t pin, uint8_t level)
{
    gpio_write(pin, level);    
}

uint8_t digitalRead(uint16_t pin)
{
    return gpio_read(pin);
}

_attribute_ram_code_ void pm_wait_ms(uint16_t milliseconds)
{
    cpu_stall_wakeup_by_timer0(milliseconds * CLOCK_SYS_CLOCK_1MS);
}

_attribute_ram_code_ void pm_wait_us(uint16_t microseconds)
{
    cpu_stall_wakeup_by_timer0(microseconds * CLOCK_SYS_CLOCK_1US);
}

//----------------------------------
// replace functions from LCD driver (ATC's codebase)
// with EPD-specific implementation
//----------------------------------
void show_atc_mac(XiaomiMiaoMiaoCeBT* c)
{
	extern u8 mac_public[6];
    epd_start_new_screen(c);
    epd_set_shape(c, ATC);
    epd_set_digit(c, mac_public[2] & 0x0f, BOTTOM_RIGHT);
    epd_set_digit(c, mac_public[2] >> 4, BOTTOM_LEFT);
    epd_write_display(c);
    pm_wait_ms(1800);
    epd_start_new_screen(c);
    epd_set_shape(c, ATC);
    epd_write_display(c);
    pm_wait_ms(200);
    epd_set_digit(c, mac_public[1] & 0x0f, BOTTOM_RIGHT);
    epd_set_digit(c, mac_public[1] >> 4, BOTTOM_LEFT);
    epd_write_display(c);
    pm_wait_ms(1800);
    epd_start_new_screen(c);
    epd_set_shape(c, ATC);
    epd_write_display(c);
    pm_wait_ms(200);
    epd_set_digit(c, mac_public[0] & 0x0f, BOTTOM_RIGHT);
    epd_set_digit(c, mac_public[0] >> 4, BOTTOM_LEFT);
    epd_write_display(c);
    pm_wait_ms(1800);
}

void show_temp_symbol(XiaomiMiaoMiaoCeBT* c, uint8_t symbol){/*1 = C, 2 = F*/
	if (symbol == 1) {
        epd_unset_shape(c, FIXED_DEG_F);
        epd_set_shape(c, FIXED_DEG_C);
    }
	else if (symbol == 2) {
        epd_unset_shape(c, FIXED_DEG_C);
        epd_set_shape(c, FIXED_DEG_F);
    }
}

void show_big_number(XiaomiMiaoMiaoCeBT* c, int16_t number)
{
    if (number > 1999) return;
    if (number < -99) return;
    
    epd_set_shape(c, FIXED);
    if (number > 999)
        epd_set_shape(c, TOP_LEFT_1);
    if (number < 0) {
        number = -number;
        epd_set_shape(c, MINUS);
    }
    if (number > 99)
        epd_set_digit(c, number / 100 % 10, TOP_LEFT);
    if (number > 9)
        epd_set_digit(c, number / 10 % 10, TOP_MIDDLE);
    if (number < 9)
        epd_set_digit(c, 0, TOP_MIDDLE);
    epd_set_digit(c, number % 10, TOP_RIGHT);
}

void show_small_number(XiaomiMiaoMiaoCeBT* c, uint16_t number)
{
    if (number > 99) return;

    epd_set_shape(c, FIXED);
    if (number > 9)
        epd_set_digit(c, number / 10 % 10, BOTTOM_LEFT);
    epd_set_digit(c, number % 10, BOTTOM_RIGHT);
}

void show_battery_symbol(XiaomiMiaoMiaoCeBT* c, uint8_t state)
{
    if (state)
        epd_set_shape(c, BATTERY_LOW);
    else
        epd_unset_shape(c, BATTERY_LOW);
}

void show_ble_symbol(XiaomiMiaoMiaoCeBT* c, uint8_t state)
{
    if(state)
        epd_set_shape(c, SUN);
    else
        epd_unset_shape(c, SUN);
}

void show_smiley(XiaomiMiaoMiaoCeBT* c, uint8_t state){/*0=off, 1=happy, 2=sad*/
        epd_unset_shape(c, FACE_FROWN);
        epd_unset_shape(c, FACE_SMILE);
        if (state == 1) {
            epd_unset_shape(c, FACE_FROWN);
            epd_set_shape(c, FACE_SMILE);
        }
        else if (state == 2) {
            epd_unset_shape(c, FACE_SMILE);
            epd_set_shape(c, FACE_FROWN);
        }
}

void epd_init(XiaomiMiaoMiaoCeBT* c, uint8_t redraw)
{
    // set the pin modes (note: no hardware SPI is used)

    // set pull-up resistors on output pins
    gpio_setup_up_down_resistor(SPI_ENABLE, PM_PIN_PULLUP_10K);
    gpio_setup_up_down_resistor(SPI_MOSI, PM_PIN_PULLUP_10K);
    gpio_setup_up_down_resistor(IO_RST_N, PM_PIN_PULLUP_10K);
    gpio_setup_up_down_resistor(SPI_CLOCK, PM_PIN_PULLUP_10K);
    gpio_setup_up_down_resistor(EPD_TO_PC4, PM_PIN_PULLUP_10K);

    // set all outputs to 1
    digitalWrite(SPI_ENABLE, HIGH); 
    digitalWrite(SPI_MOSI, HIGH);
    digitalWrite(IO_RST_N, HIGH); 
    digitalWrite(SPI_CLOCK, HIGH);
    digitalWrite(EPD_TO_PC4, HIGH);

    // enable GPIO
    pinMode(SPI_ENABLE, OUTPUT);
    pinMode(SPI_MOSI, OUTPUT);
    pinMode(SPI_CLOCK, OUTPUT);
    pinMode(IO_RST_N, OUTPUT);
    pinMode(EPD_TO_PC4, OUTPUT);
    pinMode(IO_BUSY_N, INPUT);

    digitalWrite(SPI_MOSI, LOW);
    digitalWrite(SPI_CLOCK, LOW);

    if(redraw)
    {
        // pulse RST_N low for 110 microseconds
        digitalWrite(IO_RST_N, LOW);
        pm_wait_us(110);
        digitalWrite(IO_RST_N, HIGH);
        pm_wait_us(145);

        // start an initialisation sequence (black - all 0xFF)
        send_sequence(c, T_LUTV_init, T_LUT_KK_init, T_LUT_KW_init, T_DTM_init, 1);

        // Original firmware pauses here for about 2000 ms
        // in addition to display refresh BUSY_N signal (only on start-up)
        // Might be necessary in order to fully energise the black particles,
        // or it might be linked to BLE/hardware initialisation.
        pm_wait_ms(2000);

        // pulse RST_N low for 110 microseconds
        digitalWrite(IO_RST_N, LOW);
        pm_wait_us(110);
        digitalWrite(IO_RST_N, HIGH);
        pm_wait_us(145);

        // start an initialisation sequence (white - all 0x00)
        send_sequence(c, T_LUTV_init, T_LUT_KW_update, T_LUT_KK_update, T_DTM2_init, 1);

        // pulse RST_N low for 110 microseconds
        digitalWrite(IO_RST_N, LOW);
        pm_wait_us(110);
        digitalWrite(IO_RST_N, HIGH);
        pm_wait_us(145);
    }
}

void send_sequence(XiaomiMiaoMiaoCeBT* c, uint8_t *dataV, uint8_t *dataKK,
                                     uint8_t *dataKW, uint8_t *data,
                                     uint8_t is_init)
{
    // send Charge Pump ON command
    transmit(0, POWER_ON);

    // wait for the display to become ready to receive new
    // commands/data: when ready, the display sets IO_BUSY_N to 1
    while (digitalRead(IO_BUSY_N) == 0)
        pm_wait_ms(1);

    transmit(0, PANEL_SETTING);
    transmit(1, 0x0B);
    transmit(0, POWER_SETTING);
    transmit(1, 0x46);
    transmit(1, 0x46);
    transmit(0, POWER_OFF_SEQUENCE_SETTING);
    if (is_init)
    {
        transmit(1, 0x00);
    }
    else
    {
        transmit(1, 0x06);
    }
    
    transmit(0, PLL_CONTROL); // Frame Rate Control
    if (is_init)
    {
        transmit(1, 0x02);
    }
    else
    {
        transmit(1, 0x03);
    }

    // NOTE: Original firmware makes partial refresh on update, but not when initialising the screen.
    if ( !is_init )
    {
        transmit(0, PARTIAL_DISPLAY_REFRESH);
        transmit(1, 0x00);
        transmit(1, 0x87);
        transmit(1, 0x01);
    }

    // send the e-paper voltage settings (waves)
    transmit(0, LUT_FOR_VCOM);
    for (int i = 0; i < 15; i++)
        transmit(1, dataV[i]);

    if(is_init)
    {
        transmit(0, LUT_CMD_0x23);
        for (int i = 0; i < 15; i++)
            transmit(1, dataKK[i]);

        transmit(0, LUT_CMD_0x26);
        for (int i = 0; i < 15; i++)
            transmit(1, dataKW[i]);
    }
    else
    {
        transmit(0, LUT_CMD_0x23);
        for (int i = 0; i < 15; i++)
            transmit(1, dataV[i]);

        transmit(0, LUT_CMD_0x24);
        for (int i = 0; i < 15; i++)
            transmit(1, dataKK[i]);

        transmit(0, LUT_CMD_0x25);
        for (int i = 0; i < 15; i++)
            transmit(1, dataKW[i]);

        transmit(0, LUT_CMD_0x26);
        for (int i = 0; i < 15; i++)
            transmit(1, dataV[i]);
    }
 
    // send the actual data
    transmit(0, DATA_START_TRANSMISSION_1);
    for (int i = 0; i < 18; i++)
        transmit(1, data[i]);

    // Original firmware sends DATA_START_TRANSMISSION_2 command only
    // when performing full refresh
    if (is_init)
    {
        transmit(0, DATA_START_TRANSMISSION_2);
        for(int i = 0; i < 18; i++)
            transmit(1, data[i]);
    }
    
    transmit(0, DISPLAY_REFRESH);

    // wait for the display to become ready to receive new
    // commands/data: when ready, the display sets IO_BUSY_N to 1
    while (digitalRead(IO_BUSY_N) == 0)
        pm_wait_ms(1);

    // send Charge Pump OFF command
    transmit(0, POWER_OFF);
    transmit(1, 0x03);

    // wait for the display to become ready to receive new
    // commands/data: when ready, the display sets IO_BUSY_N to 1
    while (digitalRead(IO_BUSY_N) == 0)
        pm_wait_ms(1);
}

void transmit(uint8_t cd, uint8_t data_to_send)
{
    // enable SPI
    digitalWrite(SPI_ENABLE, LOW);
    pm_wait_us(delay_SPI_clock_pulse);

    // send the first bit, this indicates if the following is a command or data
    digitalWrite(SPI_CLOCK, LOW);
    if (cd != 0)
        digitalWrite(SPI_MOSI, HIGH);
    else
        digitalWrite(SPI_MOSI, LOW);
    pm_wait_us(delay_SPI_clock_pulse * 2 + 1);
    digitalWrite(SPI_CLOCK, HIGH);
    pm_wait_us(delay_SPI_clock_pulse);

    // send 8 bytes
    for (int i = 0; i < 8; i++)
    {
        // start the clock cycle
        digitalWrite(SPI_CLOCK, LOW);
        // set the MOSI according to the data
        if (data_to_send & 0x80)
            digitalWrite(SPI_MOSI, HIGH);
        else
            digitalWrite(SPI_MOSI, LOW);
        // prepare for the next bit
        data_to_send = (data_to_send << 1);
        pm_wait_us(delay_SPI_clock_pulse * 2 + 1);
        // the data is read at rising clock (halfway the time MOSI is set)
        digitalWrite(SPI_CLOCK, HIGH);
        pm_wait_us(delay_SPI_clock_pulse);
    }

    // finish by ending the clock cycle and disabling SPI
    digitalWrite(SPI_CLOCK, LOW);
    pm_wait_us(delay_SPI_end_cycle);
    digitalWrite(SPI_ENABLE, HIGH);
    pm_wait_us(delay_SPI_end_cycle);
}

void epd_write_display(XiaomiMiaoMiaoCeBT* c)
{
    // Send update waveforms
    send_sequence(c, T_LUTV_init, T_LUT_KK_update, T_LUT_KW_update, c->display_data, 0);
}

void epd_write_display_data(XiaomiMiaoMiaoCeBT* c, uint8_t *data)
{
    for (int i = 0; i < 18; i++)
    {
        c->display_data[i] = data[i];    
    }

    epd_write_display(c);
}

void epd_set_digit(XiaomiMiaoMiaoCeBT* c, uint8_t digit, uint8_t where)
{
    // check if the input is valid
    if ((digit >= 0) && (digit < 16) && (where >= TOP_LEFT) && (where <= BOTTOM_RIGHT))
    {
        // set which segments are to be used
        uint8_t *segments;
        switch(where)
        {
            case TOP_LEFT: segments = top_left; break;
            case TOP_MIDDLE: segments = top_middle; break;
            case TOP_RIGHT: segments = top_right; break;
            case BOTTOM_LEFT: segments = bottom_left; break;
            case BOTTOM_RIGHT: segments = bottom_right; break;
            default:
                break;
        }

        // set the segments, there are up to 11 segments in a digit
        int segment_byte;
        int segment_bit;
        for (int i = 0; i < 11; i++)
        {
            // get the segment needed to display the digit 'digit',
            // this is stored in the array 'digits'
            int segment = digits[digit][i] - 1;
            // segment = -1 indicates that there are no more segments to display
            if (segment >= 0)
            {
                segment_byte = segments[2 * segment];
                segment_bit = segments[1 + 2 * segment];
                epd_set_segment(c, segment_byte, segment_bit, 1);
            }
            else
                // there are no more segments to be displayed
                break;
        }
    }
}

void epd_set_or_clear_shape(XiaomiMiaoMiaoCeBT* c, uint8_t where, uint8_t set_or_clear)
{
    int num_of_segments = 0;
    uint8_t *segments;

    // set the number of segments and which segments have
    // to be displayed or removed
    switch (where)
    {
        case TOP_LEFT_1:
            num_of_segments = sizeof(top_left_1) / 2;
            segments = top_left_1;
            break;
        case BATTERY_LOW:
            num_of_segments = sizeof(battery_low) / 2;
            segments = battery_low;
            break;
        case DASHES:
            num_of_segments = sizeof(dashes) / 2;
            segments = dashes;
            break;
        case FACE:
            num_of_segments = sizeof(face) / 2;
            segments = face;
            break;
        case FACE_SMILE:
            num_of_segments = sizeof(face_smile) / 2;
            segments = face_smile;
            break;
        case FACE_FROWN:
            num_of_segments = sizeof(face_frown) / 2;
            segments = face_frown;
            break;
        case FACE_NEUTRAL:
            num_of_segments = sizeof(face_neutral) / 2;
            segments = face_neutral;
            break;
        case SUN:
            num_of_segments = sizeof(sun) / 2;
            segments = sun;
            break;
        case FIXED:
            num_of_segments = sizeof(fixed) / 2;
            segments = fixed;
            break;
        case FIXED_DEG_C:
            num_of_segments = sizeof(fixed_deg_C) / 2;
            segments = fixed_deg_C;
            break;
        case FIXED_DEG_F:
            num_of_segments = sizeof(fixed_deg_F) / 2;
            segments = fixed_deg_F;
            break;
        case MINUS:
            num_of_segments = sizeof(minus) / 2;
            segments = minus;
            break;
        case ATC:
            num_of_segments = sizeof(Atc) / 2;
            segments = Atc;
            break;
        default:
            return;
    }

    // set or remove the segments, depending on flag set_or_clear
    // set_or_clear = 1 --> set segments
    // set_or_clear = 0 --> clear segments
    for (uint8_t segment = 0; segment < num_of_segments; segment++)
    {
        uint8_t segment_byte = segments[2 * segment];
        uint8_t segment_bit = segments[1 + 2 * segment];
        epd_set_segment(c, segment_byte, segment_bit, set_or_clear);
    }
}

void epd_set_shape(XiaomiMiaoMiaoCeBT* c, uint8_t where)
{
    epd_set_or_clear_shape(c, where, 1);
}

void epd_unset_shape(XiaomiMiaoMiaoCeBT* c, uint8_t where)
{
    epd_set_or_clear_shape(c, where, 0);
}

void epd_set_segment(XiaomiMiaoMiaoCeBT* c, uint8_t segment_byte, uint8_t segment_bit,
                                   uint8_t value)
{
    if (value == 1)
        // set the bit
        c->display_data[segment_byte] |= (1 << segment_bit);
    else
        // clear the bit
        c->display_data[segment_byte] &= ~(1 << segment_bit);
}

void epd_start_new_screen(XiaomiMiaoMiaoCeBT* c)
{
    // prepare the data to be displayed, assume all segments
    // are initially off
    for (int i = 0; i < 18; i++)
        c->display_data[i] = 0x00;
}