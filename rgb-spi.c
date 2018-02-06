/**
  ******************************************************************************
  * @file    main.c
  * @author  sdiemert
  * @version V1.0
  * @date    Jan 24th 2018
  * @brief   Sample code for controlling APA-102C SPI Colored LEDs (https://cpldcpu.files.wordpress.com/2014/08/apa-102c-super-led-specifications-2014-en.pdf)
  ******************************************************************************
*/


#include "stm32f0xx.h"

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

// Pin mappings for SPI outputs.
#define SPI_MISO_PIN GPIO_Pin_6
#define SPI_MOSI_PIN GPIO_Pin_7
#define SPI_SCK_PIN GPIO_Pin_5
#define SPI_CS_PIN GPIO_Pin_8
#define SPI_MISO_PORT GPIOA
#define SPI_MOSI_PORT GPIOA
#define SPI_SCK_PORT GPIOA
#define SPI_CS_PORT GPIOC

void setupSPI(void)
{
	// enable relevant clocks
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	// first configure the SPI peripheral
	SPI_InitTypeDef S;
	S.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	S.SPI_Direction = SPI_Direction_1Line_Tx; //we are only sending data.
	S.SPI_Mode = SPI_Mode_Master;
	S.SPI_DataSize = SPI_DataSize_16b;
	S.SPI_NSS = SPI_NSS_Soft;
	S.SPI_FirstBit = SPI_FirstBit_MSB;
	S.SPI_CPOL = SPI_CPOL_High;
	S.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_Init(SPI1, &S);

	// then set up the GPIO pins used by the SPI
	// for their alternative function.
	// we only need the MOSI and SCK pins (PA7 and PA5 resp).
	GPIO_InitTypeDef G;
	GPIO_StructInit(&G);
	G.GPIO_Pin = SPI_MOSI_PIN | SPI_SCK_PIN | SPI_MISO_PIN;
	G.GPIO_Mode = GPIO_Mode_AF;
	G.GPIO_OType =  GPIO_OType_PP;
	G.GPIO_Speed = GPIO_Speed_Level_3;
	G.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(SPI_MOSI_PORT, &G);

	GPIO_PinAFConfig(SPI_SCK_PORT, GPIO_PinSource5, GPIO_AF_0);
	GPIO_PinAFConfig(SPI_MOSI_PORT, GPIO_PinSource7, GPIO_AF_0);
	GPIO_PinAFConfig(SPI_MISO_PORT, GPIO_PinSource6, GPIO_AF_0);

	// Setup a normal GPIO as a chip select pin.
	GPIO_StructInit(&G);
	G.GPIO_Pin = SPI_CS_PIN;
	G.GPIO_Mode = GPIO_Mode_OUT;
	G.GPIO_OType =  GPIO_OType_PP;
	G.GPIO_Speed = GPIO_Speed_Level_1;
	G.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(SPI_CS_PORT, &G);

	GPIO_SetBits(SPI_CS_PORT, SPI_CS_PIN);

	SPI_Cmd(SPI1, ENABLE);
}

void sendSPIData(SPI_TypeDef * SPI, uint16_t d)
{
	SPI_I2S_SendData16(SPI1, d);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));
}

uint8_t make_brightness(uint8_t brightness){

	// Need to return 8 bits:
	// - 111 (3 1's)
	// - XXXXX (5 bits up to 32

	uint8_t START_SEQ = 0xE0; // 1110 0000

	uint8_t GLOBAL_SEQ = brightness;

	return (START_SEQ | GLOBAL_SEQ);
}

/**
 * SPI - the spi peripheral (e.g., SPI1).
 * brightness - a value between 0 and 31 to control brightness
 * R - Red control (0 - 0xFF)
 * B - Blue control (0 - 0xFF)
 * G - Green control (0 - 0xFF)
 */
void setLEDColor(SPI_TypeDef * SPI, uint8_t brightness, uint16_t R, uint16_t B, uint16_t G){

	SPI_Cmd(SPI1, ENABLE);

	GPIO_ResetBits(SPI_CS_PORT, SPI_CS_PIN);

	// send the data

	// send 32 zeros at start of packet.
	sendSPIData(SPI1, 0x0000);
	sendSPIData(SPI1, 0x0000);

	// send LED frame
	sendSPIData(SPI1, (make_brightness(brightness) << 8) | B);
	sendSPIData(SPI1, (G << 8) | R);
	sendSPIData(SPI1, 0x0000);

	sendSPIData(SPI1, (make_brightness(brightness) << 8) | B);
	sendSPIData(SPI1, (G << 8) | R);
	sendSPIData(SPI1, 0x0000);

	// wait for the data to finish being sent
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));

	// turn off the chip select
	GPIO_SetBits(SPI_CS_PORT, SPI_CS_PIN);

	// need to send "termination" sequence
	sendSPIData(SPI1, 0x0000);
	sendSPIData(SPI1, 0x0000);

	SPI_Cmd(SPI1, DISABLE);

}


void fadeBrightness(uint8_t R, uint8_t B, uint8_t G, uint32_t STEP, uint32_t cycles){
	for(uint32_t c = 0; c < cycles; c++){
		for(uint8_t i = 0; i < 32; i++){
			setLEDColor(SPI1, i, R, B, G);
			delay_ms(STEP);
		}
		for(uint8_t i = 31; i >1; i--){
			setLEDColor(SPI1, i, R, B, G);
			delay_ms(STEP);
		}
	}
}


int main(void)
{

	// init the system, includes clocks.
	SystemInit();
	SystemCoreClockUpdate();

	// Set up a systick interrupt every millisecond
	// SystemCoreClock = 48x10^6 Hz for STM32F0xx
	SysTick_Config(SystemCoreClock/1000);

	// setup the SPI peripheral
	setupSPI();

	setLEDColor(SPI1, 0, 0x00, 0x00, 0xFF);

	while(1){

		// start red, fade up blue
		for(uint8_t r = 0; r < 255; r++){
			setLEDColor(SPI1, 31, 255-r,r,0x00);
			delay_ms(10);
		}

		delay_ms(10);

		// fade back into red.
		for(uint8_t r = 0; r < 255; r++){
			setLEDColor(SPI1, 31, r,255-r,0x00);
			delay_ms(10);
		}
	}

	for(;;);

}
