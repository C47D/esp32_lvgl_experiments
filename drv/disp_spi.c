/**
 * @file disp_spi.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <string.h>

#include "disp_spi.h"

#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "freertos/task.h"

#include "../lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void IRAM_ATTR spi_ready(spi_transaction_t *trans);

/**********************
 *  STATIC VARIABLES
 **********************/
static spi_device_handle_t spi;

static volatile bool spi_trans_in_progress;
static volatile bool spi_color_sent;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void disp_spi_init(void)
{
    esp_err_t ret = ESP_FAIL;

    spi_bus_config_t buscfg = {
            .miso_io_num = -1,
            .mosi_io_num = DISP_SPI_MOSI,
            .sclk_io_num = DISP_SPI_CLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = LV_VDB_SIZE * 2,
    };

    spi_device_interface_config_t lcd_cfg = {
            .clock_speed_hz = 40000000,
            .mode = 0,
            .spics_io_num = DISP_SPI_CS,
            .queue_size = 1,
            .pre_cb = NULL,
            /* Callback al terminar la transaccion por SPI */
            .post_cb = spi_ready,
    };

    ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    assert(ESP_OK == ret);

    // Attach the LCD to the SPI bus
    ret = spi_bus_add_device(HSPI_HOST, &lcd_cfg, &spi);
    assert(ESP_OK == ret);
}

void disp_spi_send_data(uint8_t * data, uint16_t length)
{
    if (0 == length) {
        return;
    }

    while (spi_trans_in_progress) {

    }

    spi_transaction_t t = {0};
    memset(&t, 0, sizeof(t));
    
    t.length = length * 8;
    t.tx_buffer = data;
    
    spi_trans_in_progress = true;
    //Mark the "lv_flush_ready" NOT needs to be called in "spi_ready"
    spi_color_sent = false;
    
    spi_device_queue_trans(spi, &t, portMAX_DELAY);
}

void disp_spi_send_colors(uint8_t * data, uint16_t length)
{
    if (0 == length) {
        return;
    }

    while (spi_trans_in_progress) {

    }

    spi_transaction_t t = {0};
    memset(&t, 0, sizeof(t));
    
    t.length = length * 8;
    t.tx_buffer = data;
    
    spi_trans_in_progress = true;
    
    //Mark the "lv_flush_ready" needs to be called in "spi_ready"
    spi_color_sent = true;
    spi_device_queue_trans(spi, &t, portMAX_DELAY);
}


/**********************
 *   STATIC FUNCTIONS
 **********************/
static void IRAM_ATTR spi_ready (spi_transaction_t *trans)
{
    spi_trans_in_progress = false;

    if(spi_color_sent) {
        lv_flush_ready();
    }
}
