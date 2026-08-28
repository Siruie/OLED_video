#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Image.h"
#include "W25Q64.h"
#include "Serial.h"

#define WRITE_MODE       0
#define READ_MODE        1
#define SECTOR_SIZE      4096
#define IMAGE_SIZE       1024
#define IMAGES_PER_SECTOR (SECTOR_SIZE / IMAGE_SIZE)
#define MAX_SECTORS      2048
#define MAX_IMAGE_COUNT  (MAX_SECTORS * IMAGES_PER_SECTOR)
#define IDLE_TIMEOUT_MS  5000

volatile uint8_t mode = READ_MODE;
uint16_t image_count = 0;
uint16_t current_sector = 0;
uint8_t  offset_in_sector = 0;
uint32_t idle_ms = 0;

extern volatile uint8_t ready_buffer;
extern volatile uint8_t serial_cmd;
extern uint8_t Image_Buffer1[IMAGE_SIZE];
extern uint8_t Image_Buffer2[IMAGE_SIZE];

void Init(void) {
    OLED_Init();
    W25Q64_Init();
    Serial_Init();
    Serial_DMA_Init();
}

int main(void) {
    Init();

    Serial_SendString("Read mode. Send '0' to write Flash.\r\n");

    while (1) {
        if (mode == WRITE_MODE) {

            if (ready_buffer != 0) {
                idle_ms = 0;

                uint8_t *data;
                if (ready_buffer == 1) {
                    data = Image_Buffer1;
                } else {
                    data = Image_Buffer2;
                }

                uint32_t addr = current_sector * SECTOR_SIZE + offset_in_sector * IMAGE_SIZE;

                for (uint16_t i = 0; i < IMAGE_SIZE; i += 256) {
                    W25Q64_PageProgram(addr + i, data + i, 256);
                }

                offset_in_sector++;
                if (offset_in_sector >= IMAGES_PER_SECTOR) {
                    offset_in_sector = 0;
                    current_sector++;
                }
                image_count++;

                Serial_SendByte(0xAB);

                ready_buffer = 0;

                if (offset_in_sector == 0 && image_count < MAX_IMAGE_COUNT) {
                    W25Q64_SetErase(current_sector * SECTOR_SIZE);
                }

                if (image_count >= MAX_IMAGE_COUNT) {
                    mode = READ_MODE;
                    Serial_DisableDMA_RX();
                    Serial_SendString("\r\nAll images saved, switch to Read mode.\r\n");
                }
            } else {
                Delay_ms(1);
                idle_ms++;
                if (idle_ms >= IDLE_TIMEOUT_MS) {
                    mode = READ_MODE;
                    Serial_DisableDMA_RX();
                    Serial_SendString("Idle timeout, switch to Read mode.\r\n");
                }
            }
        } else {
            uint16_t total = (image_count > 0) ? image_count : MAX_IMAGE_COUNT / 4;
            uint16_t last_sector = 0xFFFF;
            for (uint16_t i = 0; i < total; i++) {
                if (serial_cmd == '0') {
                    serial_cmd = 0;
                    mode = WRITE_MODE;
                    idle_ms = 0;
                    image_count = 0;
                    current_sector = 0;
                    offset_in_sector = 0;
                    W25Q64_SetErase(0);
                    Serial_EnableDMA_RX();
                    Serial_SendString("Switch to Write mode.\r\n");
                    break;
                }

                uint16_t sector = i / IMAGES_PER_SECTOR;
                uint8_t  offset = i % IMAGES_PER_SECTOR;

                if (sector != last_sector) {
                    last_sector = sector;
                    Image_Load(sector * SECTOR_SIZE);
                }

                Show_Image(offset);
                Delay_ms(90);
            }
        }
    }
}