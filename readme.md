# OLED_video

本项目由STM32F103C8T6实现，通过将视频抽帧、二值化后得到的值以数组形式存储。通过串口传输并使用W25Q64存储数据，以实现大量数据存储。通过4引脚的0.96寸OLED屏显示视频。
单片机引脚配置默认为
PB8：OLED屏的SCL
PB9:OLED屏的SDA
PA2:串口的RXD
PA3:串口的TXD
PA4:W25Q64的CS
PA5:W25Q64的CLK
PA6:W25Q64的DO
PA7:W25Q64的DI
