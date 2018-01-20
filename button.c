/**
  ******************************************************************************
  * @file    button.c
  * @author  sdiemert
  * @version V1.0
  * @date    2018 01 19
  * @brief   Default main function.
  ******************************************************************************
*/

#include "stm32f0xx.h"

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
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	GPIO_InitTypeDef G;
	G.GPIO_Pin = GPIO_Pin_13;
	G.GPIO_Mode = GPIO_Mode_IN;
	G.GPIO_PuPd = GPIO_PuPd_UP;
	G.GPIO_OType = GPIO_OType_PP;
	G.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_Init(GPIOC, &G);
}

void ledOn(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_5);
}

void ledOff(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_5);
}


int main(void)
{
	SystemInit();
	SystemCoreClockUpdate();

	setupLED();
	setupButton();

	while(1){

		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13)){
			ledOn();
		}else{
			ledOff();
		}

	}
}

