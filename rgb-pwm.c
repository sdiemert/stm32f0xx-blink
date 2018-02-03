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
			
#define LED1_PORT GPIOA
#define LED1_PIN GPIO_Pin_1

#define LED2_PORT GPIOA
#define LED2_PIN GPIO_Pin_5

#define LED3_PORT GPIOA
#define LED3_PIN GPIO_Pin_5


void setupLEDs(void){

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	GPIO_InitTypeDef G;
	G.GPIO_Pin = LED1_PIN;
	G.GPIO_Mode = GPIO_Mode_AF;
	G.GPIO_PuPd = GPIO_PuPd_NOPULL;
	G.GPIO_OType = GPIO_OType_PP;
	G.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_Init(GPIOA, &G);
	GPIO_PinAFConfig(LED1_PORT, GPIO_PinSource1, GPIO_AF_2);
}

void setupPWM(void){

	uint32_t freq = 1000;
	uint32_t pulse = 1; // pulse width (pulse/period)
	uint32_t prescaler = 1000;
	uint32_t period = (48000000)/(prescaler*freq);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseInitTypeDef T;
	TIM_OCInitTypeDef O;

	T.TIM_ClockDivision = TIM_CKD_DIV1;
	T.TIM_CounterMode = TIM_CounterMode_Up;
	T.TIM_Prescaler = prescaler;
	T.TIM_Period = period;
	T.TIM_RepetitionCounter = 0x000;

	TIM_OCStructInit(&O);
	O.TIM_OCMode = TIM_OCMode_PWM1;
	O.TIM_Pulse = pulse;
	O.TIM_OutputState = TIM_OutputState_Enable;
	O.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);

	TIM_TimeBaseInit(TIM2, &T);
	TIM_OC2Init(TIM2, &O);

	TIM_Cmd(TIM2, ENABLE);
}

int main(void)
{
	SystemInit();
	SystemCoreClockUpdate();

	setupLEDs();
	setupPWM();

	while(1);
}
\