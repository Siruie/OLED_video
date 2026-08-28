#ifndef __SERIAL_H
#define __SERIAL_H
#include "stm32f10x.h"                  // Device header


extern uint8_t Image_Buffer1[1024];
extern uint8_t Image_Buffer2[1024];

extern volatile uint8_t current_buffer;
extern volatile uint8_t ready_buffer;
extern volatile uint8_t serial_cmd;

void Serial_Init(void);
void Serial_DMA_Init(void);
void Serial_EnableDMA_RX(void);
void Serial_DisableDMA_RX(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendString(char *str);
void Serial_SendArray_DMA(uint8_t *Data, uint16_t Size);

#endif