/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f0xx.h"

// PA01 is attached to TIM2 CH 2 (PA 07 on breakout)
#define LED_RED_PORT GPIOA
#define LED_RED_PIN GPIO_Pin_1

// PB10 is attached to TIM2 CH 3 (PA 03 on breakout)
#define LED_GREEN_PORT GPIOB
#define LED_GREEN_PIN GPIO_Pin_10


// PB4 is attached to TIM3 CH 1 (PA 10 on breakout)
#define LED_BLUE_PORT GPIOB
#define LED_BLUE_PIN GPIO_Pin_4


typedef enum {
	RED, BLUE, GREEN
} LED;


// use this to track milliseconds elapsed.
volatile uint32_t milli = 0;

/**
 * Called by system every SysTick (1ms) as defined
 * in main.
 */
void SysTick_Handler(void){
	milli++;
}

void delay_ms(uint32_t m){
	volatile uint32_t start = milli;
	while(milli - start < m) asm("nop");
}

void setupLEDs(void){

	GPIO_InitTypeDef G;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);

	GPIO_StructInit(&G);
	G.GPIO_Pin = LED_RED_PIN;
	G.GPIO_Mode = GPIO_Mode_AF;
	G.GPIO_PuPd = GPIO_PuPd_NOPULL;
	G.GPIO_OType = GPIO_OType_PP;
	G.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_Init(LED_RED_PORT, &G);
	GPIO_PinAFConfig(LED_RED_PORT, GPIO_PinSource1, GPIO_AF_2);

	GPIO_StructInit(&G);
	G.GPIO_Pin = LED_GREEN_PIN;
	G.GPIO_Mode = GPIO_Mode_AF;
	G.GPIO_PuPd = GPIO_PuPd_NOPULL;
	G.GPIO_OType = GPIO_OType_PP;
	G.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_Init(LED_GREEN_PORT, &G);
	GPIO_PinAFConfig(LED_GREEN_PORT, GPIO_PinSource10, GPIO_AF_2);

	GPIO_StructInit(&G);
	G.GPIO_Pin = LED_BLUE_PIN;
	G.GPIO_Mode = GPIO_Mode_AF;
	G.GPIO_PuPd = GPIO_PuPd_NOPULL;
	G.GPIO_OType = GPIO_OType_PP;
	G.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_Init(LED_BLUE_PORT, &G);
	GPIO_PinAFConfig(LED_BLUE_PORT, GPIO_PinSource4, GPIO_AF_1);

}

/**
 * Determines the pulse width value from the duty cycle
 * given a frequency and clock prescaler.
 *
 * duy is a value between 0 and 100.
 */
uint32_t getPulseFromDuty(uint32_t duty, uint32_t freq, uint32_t prescaler){

	return duty*(48000000 / (prescaler * freq))/100;

}

void setupPWM(void){

	uint32_t freq = 1000;
	uint32_t pulse = getPulseFromDuty(10, 1000, 1000); // pulse width = (pulse/period)
	uint32_t prescaler = 1000;
	uint32_t period = (48000000)/(prescaler*freq);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseInitTypeDef T;
	TIM_OCInitTypeDef O;

	T.TIM_ClockDivision = TIM_CKD_DIV1;
	T.TIM_CounterMode = TIM_CounterMode_Up;
	T.TIM_Prescaler = prescaler;
	T.TIM_Period = period;
	T.TIM_RepetitionCounter = 0x000;


	// Using TIM2 and TIM3
	TIM_TimeBaseInit(TIM2, &T);
	TIM_TimeBaseInit(TIM3, &T);

	TIM_OCStructInit(&O);
	O.TIM_OCMode = TIM_OCMode_PWM1;
	O.TIM_Pulse = pulse;
	O.TIM_OutputState = TIM_OutputState_Enable;
	O.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
	TIM_OC2Init(TIM2, &O);

	TIM_OCStructInit(&O);
	O.TIM_OCMode = TIM_OCMode_PWM1;
	O.TIM_Pulse = pulse;
	O.TIM_OutputState = TIM_OutputState_Enable;
	O.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
	TIM_OC3Init(TIM2, &O);

	TIM_OCStructInit(&O);
	O.TIM_OCMode = TIM_OCMode_PWM1;
	O.TIM_Pulse = pulse;
	O.TIM_OutputState = TIM_OutputState_Enable;
	O.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
	TIM_OC1Init(TIM3, &O);

	TIM_Cmd(TIM2, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
}

void adjustPulse(uint16_t pulse, LED L){

	// CCR2 is used as the register for setting the pulse.

	switch(L){

		case RED:
			TIM2->CCR2 = (uint32_t) getPulseFromDuty(pulse, 1000, 1000);
			break;

		case GREEN:
			TIM2->CCR3 = (uint32_t) getPulseFromDuty(pulse, 1000, 1000);
			break;

		case BLUE:
			TIM3->CCR1 = (uint32_t) getPulseFromDuty(pulse, 1000, 1000);
			break;

	default:
		break;

	}

}

int main(void)
{
	SystemInit();
	SystemCoreClockUpdate();

	// Set up a systick interrupt every millisecond
	// SystemCoreClock = 48x10^6 Hz for STM32F0xx
	SysTick_Config(SystemCoreClock/1000);

	setupLEDs();
	setupPWM();

	adjustPulse(0, RED);
	adjustPulse(0, GREEN);
	adjustPulse(0, BLUE);

	uint32_t step = 1;


	while(1){

		for(uint16_t p=1; p < 100; p++){
			delay_ms(step);
			adjustPulse(p, BLUE);
		}

		for(uint16_t p=100; p > 0; p--){
			delay_ms(step);
			adjustPulse(p, BLUE);
		}

		for(uint16_t p=1; p < 100; p++){
			delay_ms(step);
			adjustPulse(p, RED);
		}

		for(uint16_t p=100; p > 0; p--){
			delay_ms(step);
			adjustPulse(p, RED);
		}

		for(uint16_t p=1; p < 100; p++){
			delay_ms(step);
			adjustPulse(p, GREEN);
		}

		for(uint16_t p=100; p > 0; p--){
			delay_ms(step);
			adjustPulse(p, GREEN);
		}

		step = (step + 1)%10 + 1;

	}
}
