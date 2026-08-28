#include "stm32f10x.h"                  // Device header
#include "Serial.h"

uint8_t Image_Buffer1[1024];
uint8_t Image_Buffer2[1024];

volatile uint8_t current_buffer = 0;
volatile uint8_t ready_buffer = 0;
volatile uint8_t serial_cmd = 0;
static volatile uint8_t Serial_TX_Busy = 0;

void Serial_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &USART_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART2, ENABLE);
}

void Serial_DMA_Init(void) {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_InitTypeDef DMA_InitStructure;

    DMA_DeInit(DMA1_Channel7);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)0;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel7, &DMA_InitStructure);

    DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);

    DMA_DeInit(DMA1_Channel6);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Image_Buffer1;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 1024;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);

    DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);

    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
}

void Serial_EnableDMA_RX(void) {
    DMA_Cmd(DMA1_Channel6, DISABLE);
    DMA_SetCurrDataCounter(DMA1_Channel6, 1024);
    current_buffer = 0;
    ready_buffer = 0;
    DMA1_Channel6->CMAR = (uint32_t)Image_Buffer1;
    DMA_Cmd(DMA1_Channel6, ENABLE);
}

void Serial_DisableDMA_RX(void) {
    DMA_Cmd(DMA1_Channel6, DISABLE);
}

void Serial_SendByte(uint8_t Byte) {
    USART_SendData(USART2, Byte);
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}

void Serial_SendString(char *str) {
    while (*str != '\0') {
        Serial_SendByte((uint8_t)*str);
        str++;
    }
}

void Serial_SendArray_DMA(uint8_t *Data, uint16_t Size) {
    while (Serial_TX_Busy);
    Serial_TX_Busy = 1;

    DMA_Cmd(DMA1_Channel7, DISABLE);
    DMA_SetCurrDataCounter(DMA1_Channel7, Size);
    DMA1_Channel7->CMAR = (uint32_t)Data;
    DMA_Cmd(DMA1_Channel7, ENABLE);
}

void USART2_IRQHandler(void) {
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET) {
        serial_cmd = USART_ReceiveData(USART2);
    }
}

void DMA1_Channel7_IRQHandler(void) {
    if (DMA_GetITStatus(DMA1_IT_TC7)) {
        DMA_ClearITPendingBit(DMA1_IT_TC7);
        Serial_TX_Busy = 0;
    }
}

void DMA1_Channel6_IRQHandler(void) {
    if (DMA_GetITStatus(DMA1_IT_TC6)) {
        DMA_ClearITPendingBit(DMA1_IT_TC6);

        ready_buffer = current_buffer + 1;
        current_buffer = 1 - current_buffer;

        DMA_Cmd(DMA1_Channel6, DISABLE);
        DMA_SetCurrDataCounter(DMA1_Channel6, 1024);
        if (current_buffer == 0) {
            DMA1_Channel6->CMAR = (uint32_t)Image_Buffer1;
        } else {
            DMA1_Channel6->CMAR = (uint32_t)Image_Buffer2;
        }
        DMA_Cmd(DMA1_Channel6, ENABLE);
    }
}