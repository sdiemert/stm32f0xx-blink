/**
  ******************************************************************************
  * @file    main.c
  * @author  sdiemert
  * @version V1.0
  * @date    Jan 20th 2018
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f0xx.h"


void setupLED()
{

	// enable the clock for GPIO Port A
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	// Setup the GPIO pin we want.
	GPIO_InitTypeDef G;
	G.GPIO_Pin = GPIO_Pin_5;
	G.GPIO_Mode = GPIO_Mode_OUT;
	G.GPIO_PuPd = GPIO_PuPd_NOPULL;
	G.GPIO_OType = GPIO_OType_PP;
	G.GPIO_Speed = GPIO_Speed_Level_2;
	GPIO_Init(GPIOA, &G);
}

void setupOutPin(void)
{
	// enable the clock for GPIO Port A
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	// Setup the GPIO pin we want.
	GPIO_InitTypeDef G;
	G.GPIO_Pin = GPIO_Pin_7;
	G.GPIO_Mode = GPIO_Mode_OUT;
	G.GPIO_PuPd = GPIO_PuPd_NOPULL;
	G.GPIO_OType = GPIO_OType_PP;
	G.GPIO_Speed = GPIO_Speed_Level_2;
	GPIO_Init(GPIOA, &G);
}

void ledOn(void)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET);
	GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_SET);
}

void ledOff(void)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET);
	GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_RESET);
}

void initTimer(void)
{

	// enable the timer
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	// Useful reference: https://visualgdb.com/tutorials/arm/stm32/timers/
	//
	// The equation for total timer frequency:
	//  timer_freq  = 1/(prescalar * (1/clk_freq) * timer_period_steps)
	//   			= clk_freq/(prescalar * timer_period_steps)
	//
	// For example:
	// Clock frequency is 48MHz -> clock period is 0.00000002083333... seconds
	// Timer Period Steps: 65535
	// Prescalar: 1000
	//
	// Each step takes: 1000 * 0.00000002083333 = 0.000020833 seconds = 0.020833 ms
	//
	// The timer overruns at: 65535 steps which is 65535*0.000020833 = 1.36 seconds
	//
	// 1/1.36 seconds = 0.73244 Hz

	TIM_TimeBaseInitTypeDef T;
	T.TIM_ClockDivision = TIM_CKD_DIV1; // no divider
	T.TIM_CounterMode = TIM_CounterMode_Up; // counts from 0 up to the target value.
	T.TIM_Period = 0xFFFF;
	T.TIM_Prescaler = 1000;
	T.TIM_RepetitionCounter = 0x000;
	TIM_TimeBaseInit(TIM2, &T);

	// Enable a software interrupt for the TIM2.
	NVIC_InitTypeDef I;
	I.NVIC_IRQChannel = TIM2_IRQn;
	I.NVIC_IRQChannelPriority = 0;
	I.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&I);

	// Clear any pending interrupt bits before
	// we start the interrupts, in case something was
	// set previously.
	TIM_ClearITPendingBit (TIM2, TIM_IT_Update);

	// Enable interrupts for TIM2 on "update" (i.e., when the
	// timer completes a full cycle).
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	// Alternatively, could use CCx to compare values on channels
	// and have different timing events bound to the same interrupt.
	TIM_Cmd(TIM2, ENABLE);
}

uint8_t state = 0;

void TIM2_IRQHandler(void){

	// IMPORTANT:
	// ----------------------------
	// Clear the interrupt pending
	// bit so we don't fire another one.
	// ----------------------------
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

	// Do the interrupt routine, in this case toggle the LED/GPIO.
	if(!state){
		ledOn();
		state = 1;
	}else{
		ledOff();
		state = 0;
	}

}

int main(void)
{
	// init system and clocks.
	SystemInit();
	SystemCoreClockUpdate();

	// setup routines
	setupLED();
	setupOutPin();
	initTimer();

	ledOn();

	for(;;);
}

