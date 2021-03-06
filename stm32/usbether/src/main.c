//#include <stdio.h>
//#include <stdlib.h>
//#include <stdint.h>
#include <math.h>

#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"
#include "main.h"

#define LEDPORT GPIOA
#define LEDCLOCK  RCC_AHB1Periph_GPIOA

#include "usbd_cdc_r_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_rndis.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Private variables
volatile uint32_t time_var1, time_var2;
__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

// Private function prototypes
void init();
void calculation_test();

int main(void) {
	init();

	/*
	 * Disable STDOUT buffering. Otherwise nothing will be printed
	 * before a newline character or when the buffer is flushed.
	 */
//	setbuf(stdout, NULL);

	xTaskCreate( calculation_test, ( signed char * ) "CALC", configMINIMAL_STACK_SIZE, NULL, 2,
                            ( xTaskHandle * ) NULL);
	vTaskStartScheduler();
	for(;;) {
		GPIO_SetBits(LEDPORT, GPIO_Pin_13);
		vTaskDelay(1500);
		GPIO_ResetBits(LEDPORT, GPIO_Pin_13);
		vTaskDelay(500);
	}

	return 0;
}

void calculation_test() {
	float a = 1.001;
	int iteration = 0;

	for(;;) {
		GPIO_SetBits(LEDPORT, GPIO_Pin_13);
		vTaskDelay(500 / portTICK_RATE_MS);
		GPIO_ResetBits(LEDPORT, GPIO_Pin_13);
		vTaskDelay(500 / portTICK_RATE_MS);

//		time_var2 = 0;
//		for (int i = 0;i < 1000000;i++) {
//			a += 0.01 * sqrtf(a);
//		}

//		printf("Time:      %lu ms\n\r", time_var2);
//		printf("Iteration: %i\n\r", iteration);
//		printf("Value:     %.5f\n\n\r", a);

		iteration++;
	}
}

void init() {
	GPIO_InitTypeDef  GPIO_InitStructure;


	// ---------- GPIO -------- //
	// LEDPORT Periph clock enable
	RCC_AHB1PeriphClockCmd(LEDCLOCK, ENABLE);

	// Configure PD12, PD13, PD14 and PD15 in output pushpull mode
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(LEDPORT, &GPIO_InitStructure);

	// ------------- USB -------------- //
	USBD_Init(&USB_OTG_dev,
	            USB_OTG_FS_CORE_ID,
	            &USR_desc,
	            &USBD_CDC_R_cb,
	            &USR_cb);
}




void vApplicationTickHook(void) {
}
/*-----------------------------------------------------------*/
void vApplicationIdleHook( void )  {
}


void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed portCHAR *pcTaskName) {
  for (;;) {
  }
}
