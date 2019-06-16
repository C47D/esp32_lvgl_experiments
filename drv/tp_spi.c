/**
 * @file tp_spi.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <string.h>

#include "tp_spi.h"

#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/
static spi_device_handle_t spi;

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void tp_spi_init(void)
{
	esp_err_t ret = ESP_FAIL;

	spi_bus_config_t buscfg = {
		.miso_io_num = TP_SPI_MISO,
		.mosi_io_num = TP_SPI_MOSI,
		.sclk_io_num = TP_SPI_CLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1
	};

	spi_device_interface_config_t touch_cfg = {
		.clock_speed_hz = 10000000,
		.mode = 0,
		.spics_io_num = -1,
		.queue_size = 1,
		.pre_cb = NULL,
		.post_cb = NULL,
	};

	//Initialize the SPI bus
	ret = spi_bus_initialize(VSPI_HOST, &buscfg, 2);
	assert(ESP_OK == ret);

	//Attach the LCD to the SPI bus
	ret = spi_bus_add_device(VSPI_HOST, &touch_cfg, &spi);
	assert(ESP_OK == ret);
}

uint8_t tp_spi_xchg(uint8_t data_send)
{
	spi_transaction_t *rt = NULL;
	esp_err_t ret = ESP_FAIL;
    uint8_t data_rec = 0;

	spi_transaction_t t = {0};
    memset(&t, 0, sizeof(t));
	
	t.length = 8; /* in bits */
	t.tx_buffer = &data_send;
	t.rx_buffer = &data_rec;

	spi_device_queue_trans(spi, &t, portMAX_DELAY);
	spi_device_get_trans_result(spi, &rt, portMAX_DELAY);

	return data_rec;
}


/**********************
 *   STATIC FUNCTIONS
 **********************/
