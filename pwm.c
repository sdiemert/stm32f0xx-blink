/**
  ******************************************************************************
  * @file    main.c
  * @author  sdiemert
  * @version V1.0
  * @date    Jan 22nd 2018
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
	// enable the clock for GPIO Port B
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	// Setup the GPIO pin we want.
	GPIO_InitTypeDef G;
	G.GPIO_Pin = GPIO_Pin_4;

	// -----------------------------
	// IMPORTANT: set the GPIO to be alternative function.
	G.GPIO_Mode = GPIO_Mode_AF;
	// -----------------------------

	G.GPIO_PuPd = GPIO_PuPd_NOPULL;
	G.GPIO_OType = GPIO_OType_PP;
	G.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_Init(GPIOB, &G);
}

void ledOn(void)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET);
}

void ledOff(void)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET);
}

void initTimer(void)
{

	// enable the timer
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	// Useful reference: https://visualgdb.com/tutorials/arm/stm32/timers/
	//
	// The equation for total timer frequency:
	//  timer_freq  = 1/(prescalar * (1/clk_freq) * (timer_period_steps-1))
	//   			= clk_freq/(prescalar * (timer_period_steps-1))
	//
	// For example:
	//
	// f = 1000 = (48x10^6/((1000) * S)) (solve for S)
	// S = 48x10^6 / (1000 * 1000)
	// S = 48

	uint32_t freq = 1000; // in KHz
	uint32_t pulse = 36; // pulse width = pulse/period
	uint32_t prescalar = 1000;
	uint32_t period = (48000000)/(prescalar * freq);

	TIM_TimeBaseInitTypeDef T;
	T.TIM_ClockDivision = TIM_CKD_DIV1; // no divider
	T.TIM_CounterMode = TIM_CounterMode_Up; // counts from 0 up to the target value.
	T.TIM_Prescaler = prescalar;
	T.TIM_Period = period - 1; // subtract one b/c we start count from 0.
	T.TIM_RepetitionCounter = 0x000;
	TIM_TimeBaseInit(TIM3, &T);

	// we need to use this timer in *output mode*
	// so that we can export a wave-form directly from the
	// timer without having to handle it indirectly via
	// an interrupt or polling.
	TIM_OCInitTypeDef O;
	TIM_OCStructInit(&O);

	O.TIM_OCMode = TIM_OCMode_PWM1;
	O.TIM_Pulse = pulse;
	O.TIM_OutputState = TIM_OutputState_Enable;

	// IF TIM_OCPolarity_Low is used THEN the pulse will be low voltage part of wave
	// IF TIM_OCPolarity_High is used THEN the pulse will be high voltage part of wave
	O.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OC1Init(TIM3, &O);

    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);

    // configure the alternative function output for PB4.
    // see data sheet for Nucleo for the mapping of AF's to pins.
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_1);

	TIM_Cmd(TIM3, ENABLE);
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

	ledOn(); // not strictly required, but nice to know we got to this point.

	for(;;);
}

