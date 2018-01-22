/**
  ******************************************************************************
  * @file    main.c
  * @author  sdiemert
  * @version V1.0
  * @date    2018-01-20
  * @brief   Default main function.
  ******************************************************************************
*/

#include "stm32f0xx.h"

uint8_t led_state = 0;
uint8_t button_pressed = 0;

/**
 * This function is a clone of the one from the
 * button.c example.
 *
 * Setup the LED on the Nucleo as output on PA5.
 */
void setupLED(void)
{

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitTypeDef G;

	G.GPIO_Pin = GPIO_Pin_5;
	G.GPIO_Mode = GPIO_Mode_OUT;
	G.GPIO_PuPd = GPIO_PuPd_NOPULL;
	G.GPIO_OType = GPIO_OType_PP;
	G.GPIO_Speed = GPIO_Speed_Level_1;

	GPIO_Init(GPIOA, &G);

}


void setupButton(void)
{

	// setup the GPIO for the button.
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	GPIO_InitTypeDef G;
	G.GPIO_Pin = GPIO_Pin_9;
	G.GPIO_Pin = GPIO_Mode_IN;
	G.GPIO_PuPd = GPIO_PuPd_UP;
	G.GPIO_OType = GPIO_OType_PP;
	G.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_Init(GPIOC, &G);

	// setup the interrupt for the button

	// we are using an external event (off chip)
	// so we must init an external interrupt
	// and then also attach it to a routine
	// using the NVIC.


	// See section 9.1.2 of the STM32F0 reference manual
	// for a description of this register.
	//
	// Basically, binds the GPIO pin we want (PC9) to the
	// interrupt we want.
	//
	// We want to set PC9 to be associated with EXTI9 so we
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource9);

	EXTI_InitTypeDef I;
	I.EXTI_Line = EXTI_Line9;
	I.EXTI_LineCmd = ENABLE;
	I.EXTI_Mode = EXTI_Mode_Interrupt;
	I.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_Init(&I);

	// Init the NVIC with this interrupt.
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

void HardFault_Handler(void){

	asm("nop");

}

void ledOn(void)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET);
	led_state = 1;
}

void ledOff(void)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET);
	led_state = 0;
}


void EXTI4_15_IRQHandler(void)
{

	// EXTI lines 4 through 15 may have caused this interrupt
	// handler to fire.
	//
	// Check that EXTI 9 is unmasked (it is a 1) and that the
	// pending bit for 9 is a 1, then it must have been triggered
	// by a 9.
	if((EXTI->IMR & EXTI_IMR_MR9) && (EXTI->PR & EXTI_PR_PR9))
	{

		// check that hte button is depressed still.
		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9))
		{
			button_pressed = 1;
		}

		// clear the pending flag.
		EXTI_ClearFlag(EXTI_Line9);
	}
}


int main(void)
{

	SystemInit();
	SystemCoreClockUpdate();

	setupLED();
	setupButton();

	ledOn();


	for(;;)
	{

		// opted for this approach to limit the number of
		// times we are issuing writes to the GPIO registers
		// for on/off.

		// Check if the LED is on/off and if we have pressed the button
		// recently.

		if(led_state == 1 && button_pressed == 1)
		{
			ledOff();
			button_pressed = 0;
		}

		else if(led_state == 0 && button_pressed == 1)
		{
			ledOn();
			button_pressed = 0;
		}
	}
}

