/**
 * @file XPT2046.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stddef.h>

#include "esp_system.h"
#include "driver/gpio.h"

#include "tp_spi.h"
#include "xpt2046.h"

/*********************
 *      DEFINES
 *********************/
#define CMD_X_READ  0b10010000
#define CMD_Y_READ  0b11010000

/**********************
 *      TYPEDEFS
 **********************/
#define XPT2046_XY_SWAP	1

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void xpt2046_corr(int16_t * x, int16_t * y);
static void xpt2046_avg(int16_t * x, int16_t * y);

/**********************
 *  STATIC VARIABLES
 **********************/
// XPT2046_AVG esta definido en xpt2046.h y vale 4

int16_t avg_buf_x[XPT2046_AVG] = {0};
int16_t avg_buf_y[XPT2046_AVG] = {0};
uint8_t avg_last = 0;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the XPT2046
 */
void xpt2046_init(void)
{
    gpio_set_direction(XPT2046_IRQ, GPIO_MODE_INPUT);
    gpio_set_direction(TP_SPI_CS, GPIO_MODE_OUTPUT);
    gpio_set_level(TP_SPI_CS, 1);
}

/**
 * Get the current position and state of the touchpad
 * @param data store the read data here
 * @return false: because no ore data to be read
 */
bool xpt2046_read(lv_indev_data_t * data)
{
    static int16_t last_x = 0;
    static int16_t last_y = 0;
    bool valid = true;
    uint8_t buf = 0;

    int16_t x = 0;
    int16_t y = 0;

    uint8_t irq = gpio_get_level(XPT2046_IRQ);

    if(irq == 0) {
        gpio_set_level(TP_SPI_CS, 0);
        tp_spi_xchg(CMD_X_READ);         /*Start x read*/

        buf = tp_spi_xchg(0);           /*Read x MSB*/
        x = buf << 8;
        buf = tp_spi_xchg(CMD_Y_READ);  /*Until x LSB converted y command can be sent*/
        x += buf;

        buf =  tp_spi_xchg(0);   /*Read y MSB*/
        y = buf << 8;

        buf =  tp_spi_xchg(0);   /*Read y LSB*/
        y += buf;
        gpio_set_level(TP_SPI_CS, 1);

        /*Normalize Data*/
        x = x >> 3;
        y = y >> 3;

        xpt2046_corr(&x, &y);
        xpt2046_avg(&x, &y);
        last_x = x;
        last_y = y;

#if 0
		printf("IRQ after: pos x: %i, pos y: %i\r\n", x, y);
#endif

    } else {
        x = last_x;
        y = last_y;
        avg_last = 0;
        valid = false;
    }

    data->point.x = x;
    data->point.y = y;

    data->state = valid == false ? LV_INDEV_STATE_REL : LV_INDEV_STATE_PR;

    return valid;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void xpt2046_corr(int16_t * x, int16_t * y)
{
#if XPT2046_XY_SWAP != 0
    int16_t swap_tmp = 0;
    swap_tmp = *x;
    *x = *y;
    *y = swap_tmp;
#endif

    if((*x) > XPT2046_X_MIN)(*x) -= XPT2046_X_MIN;
    else(*x) = 0;

    if((*y) > XPT2046_Y_MIN)(*y) -= XPT2046_Y_MIN;
    else(*y) = 0;

    (*x) = (uint32_t)((uint32_t)(*x) * LV_HOR_RES) /
           (XPT2046_X_MAX - XPT2046_X_MIN);

    (*y) = (uint32_t)((uint32_t)(*y) * LV_VER_RES) /
           (XPT2046_Y_MAX - XPT2046_Y_MIN);

#if XPT2046_X_INV != 0
    (*x) =  LV_HOR_RES - (*x);
#endif

#if XPT2046_Y_INV != 0
    (*y) =  LV_VER_RES - (*y);
#endif
}

static void xpt2046_avg(int16_t * x, int16_t * y)
{
    /*Shift out the oldest data*/
    uint8_t i = 0;

    for(i = XPT2046_AVG - 1; i > 0 ; i--) {
        avg_buf_x[i] = avg_buf_x[i - 1];
        avg_buf_y[i] = avg_buf_y[i - 1];
    }

    /*Insert the new point*/
    avg_buf_x[0] = *x;
    avg_buf_y[0] = *y;
    if(avg_last < XPT2046_AVG) avg_last++;

    /*Sum the x and y coordinates*/
    int32_t x_sum = 0;
    int32_t y_sum = 0;
    for(i = 0; i < avg_last ; i++) {
        x_sum += avg_buf_x[i];
        y_sum += avg_buf_y[i];
    }

    /*Normalize the sums*/
    (*x) = (int32_t)x_sum / avg_last;
    (*y) = (int32_t)y_sum / avg_last;
}

