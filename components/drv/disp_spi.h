/**
 * @file disp_spi.h
 *
 */

#ifndef DISP_SPI_H
#define DISP_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

#define DISP_SPI_MOSI 13
#define DISP_SPI_CLK  14
#define DISP_SPI_CS   15


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void disp_spi_init(void);
void disp_spi_send_data(uint8_t * data, uint16_t length);
void disp_spi_send_colors(uint8_t * data, uint16_t length);

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DISP_SPI_H*/
