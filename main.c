/**
  ******************************************************************************
  * @file    main.c
  * @author  sdiemert
  * @version V1.0
  * @date    17-Jan-2018
  * @brief   Short blink sample for the STM32F0xx
  ******************************************************************************
*/

#include "stm32f0xx.h"
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_gpio.h>

// use this to track milliseconds elapsed.
volatile uint32_t milli = 0;

void delay_ms(uint32_t m){
	volatile uint32_t start = milli;
	while(milli - start < m) asm("nop");
}

/**
 * Called by system every SysTick (1ms) as defined
 * in main.
 */
void SysTick_Handler(void){
	milli++;
}

int main(void)
{
	// init the system, includes clocks.
	SystemInit();
	SystemCoreClockUpdate();

	// Set up a systick interrupt every millisecond
	// SystemCoreClock = 48x10^6 Hz for STM32F0xx
	SysTick_Config(SystemCoreClock/1000);


	// enable the clock for GPIO Port A
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	// Setup the GPIO pin we want.
	GPIO_InitTypeDef G;
	G.GPIO_Pin = GPIO_Pin_5;
	G.GPIO_Mode = GPIO_Mode_OUT;
	G.GPIO_PuPd = GPIO_PuPd_NOPULL;
	G.GPIO_OType = GPIO_OType_PP;
	G.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_Init(GPIOA, &G);

	// the loop that actually
	while(1){
		GPIO_SetBits(GPIOA, GPIO_Pin_5);
		delay_ms(500);
		GPIO_ResetBits(GPIOA, GPIO_Pin_5);
		delay_ms(500);
	}
}
