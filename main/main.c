/* SPI Master example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"

#include "driver/gpio.h"

#include "esp_freertos_hooks.h"

// lvgl
#include "lvgl.h"
// #include "lv_apps/demo/demo.h"

// drv
#include "disp_spi.h"
#include "ili9341.h"
#include "tp_spi.h"
#include "xpt2046.h"

static void IRAM_ATTR lv_tick_task(void);
static lv_res_t btn_click_action(lv_obj_t *btn);

#define BTN_NEXT	GPIO_NUM_27
#define BTN_PREV	GPIO_NUM_26
#define BTN_OK		GPIO_NUM_22

#define IS_PRESSED(x)	(x != 0)

bool my_read_function(lv_indev_data_t *data)
{
	int prev_btn = gpio_get_level(BTN_PREV);
	int next_btn = gpio_get_level(BTN_NEXT);
	int ok_btn = gpio_get_level(BTN_OK);

	if (prev_btn || next_btn || ok_btn) {
		data->state = LV_INDEV_STATE_PR;

		if (IS_PRESSED(prev_btn) && !IS_PRESSED(next_btn) && !IS_PRESSED(ok_btn)) {
			printf("PREV\r\n");
			data->key = LV_GROUP_KEY_PREV;
		} else if (!IS_PRESSED(prev_btn) && IS_PRESSED(next_btn) && !IS_PRESSED(ok_btn)) {
			printf("NEXT\r\n");
			data->key = LV_GROUP_KEY_NEXT;
		} else if (IS_PRESSED(ok_btn) && !IS_PRESSED(prev_btn) && !IS_PRESSED(next_btn)) {
			printf("OK\r\n");
			data->key = LV_GROUP_KEY_ENTER;
		} else {
			data->state = LV_INDEV_STATE_REL;
		}
	} else {
		data->state = LV_INDEV_STATE_REL;
	}

	return false;
}

void app_main()
{
	gpio_pad_select_gpio(BTN_PREV);	
	gpio_set_direction(BTN_PREV, GPIO_MODE_DEF_INPUT);
	gpio_set_pull_mode(BTN_PREV, GPIO_PULLDOWN_ONLY);
	
	gpio_pad_select_gpio(BTN_NEXT);	
	gpio_set_direction(BTN_NEXT, GPIO_MODE_DEF_INPUT);
	gpio_set_pull_mode(BTN_NEXT, GPIO_PULLDOWN_ONLY);

	gpio_pad_select_gpio(BTN_OK);	
	gpio_set_direction(BTN_OK, GPIO_MODE_DEF_INPUT);
	gpio_set_pull_mode(BTN_OK, GPIO_PULLDOWN_ONLY);
	
	lv_init();

	printf("Running lvgl version %d.%d\r\n", LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR);

	disp_spi_init();
	ili9341_init();

#if 0
	tp_spi_init();
    xpt2046_init();
#endif

	lv_disp_drv_t disp;
	lv_disp_drv_init(&disp);
	disp.disp_flush = ili9341_flush;
    disp.disp_fill = ili9341_fill;
	lv_disp_drv_register(&disp);

#if 0
	lv_indev_drv_t indev;
    lv_indev_drv_init(&indev);
    indev.read = xpt2046_read;
    indev.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev);
#endif

	/*Crea una etiqueta de título*/
	lv_obj_t * label = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_text(label, "Ejemplo LVGL");
	lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 5);

	/*Crea un botón normal*/
	lv_obj_t * btn1 = lv_btn_create(lv_scr_act(), NULL);
	lv_cont_set_fit(btn1, true, true); /*Habilita el redimensionamiento horizontal y vertical*/
	lv_obj_align(btn1, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_set_free_num(btn1, 1);   /*Establece un número unico para el botón*/
	lv_btn_set_action(btn1, LV_BTN_ACTION_CLICK, btn_click_action);

	/*Agrega una etiqueta al botón*/
	label = lv_label_create(btn1, NULL);
	lv_label_set_text(label, "Presioname");

	/*Crea un botón normal*/
	lv_obj_t * btn2 = lv_btn_create(lv_scr_act(), NULL);
	/*Habilita el redimensionamiento horizontal y vertical*/
	lv_cont_set_fit(btn2, true, true);
	lv_obj_align(btn2, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
	/*Establece un número unico para el botón*/
	lv_obj_set_free_num(btn2, 2);
	lv_btn_set_action(btn2, LV_BTN_ACTION_CLICK, btn_click_action);

	/*Agrega una etiqueta al botón*/
	label = lv_label_create(btn2, NULL);
	lv_label_set_text(label, "Presioname2");

	/*Crea un botón normal*/
	lv_obj_t * btn3 = lv_btn_create(lv_scr_act(), NULL);
	/*Habilita el redimensionamiento horizontal y vertical*/
	lv_cont_set_fit(btn2, true, true);
	lv_obj_align(btn3, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
	lv_obj_set_free_num(btn3, 3);
	lv_btn_set_action(btn3, LV_BTN_ACTION_CLICK, btn_click_action);

	/*Agrega una etiqueta al botón*/
	label = lv_label_create(btn3, NULL);
	lv_label_set_text(label, "OK");

	/* create an object group  */
	lv_group_t *group = lv_group_create();
	lv_group_add_obj(group, btn1);
	lv_group_add_obj(group, btn2);
	lv_group_add_obj(group, btn3);

	/* set a group to a given indev */
	lv_indev_drv_t right_btn;
	lv_indev_drv_init(&right_btn);
	// LV_INDEV_TYPE_KEYPAD so we can navigate in the group
	right_btn.type = LV_INDEV_TYPE_KEYPAD;
	right_btn.read = my_read_function;
	lv_indev_t *kb_indev = lv_indev_drv_register(&right_btn);	

	/* When you register an input device driver the library adds
	 * some extra information to it to describe the state of the
	 * input device. When a user action happens and an action
	 * function is triggered always there is an input device which
	 * triggered that action. You can get this input device with */
	lv_indev_set_group(kb_indev, group);

	printf("Setting the last tick hook.\n");
	esp_register_freertos_tick_hook(lv_tick_task);

	while(1) {
		vTaskDelay(1);
		lv_task_handler();
	}
}

static lv_res_t btn_click_action(lv_obj_t *btn)
{
    uint8_t id = lv_obj_get_free_num(btn);

    printf("Button %d is released\n", id);

    /* Se soltó el botón.
     * Haz algo aqui */

	/* Retorna OK si el botón no se borro */
    return LV_RES_OK;
}

static void IRAM_ATTR lv_tick_task(void)
{
	lv_tick_inc(portTICK_RATE_MS);
}
