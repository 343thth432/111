***
#### W5500 TCPS SPI+DMA STM32F401RE-NUCLEO NAND K9F1G08U0D.  
TCP сервер на stm32f401re nucleo c сетевым модулем W5500 по SPI DMA бесконечно передает данные на скорости 5.5Мбит, и записывает их на NANDFlash память (без FSMC)(3.4Мбит). По нажатию кнопки отправка и запись прерывается и производится чтение всей NAND памяти.
***
### Настойка пинов и тактирование. 
1. Тактирование:  
SYCLK(MHz) = 84  
HCLK(MHz) &ensp;= 84  
1. Отладка USART+DMA:  
USART2_TX      &ensp;PA2  
USART2_RX      &ensp;PA3  
3. Кнопка перывания для чтения данных с Nand:  
GPIO_EXIT13    &ensp;PC13  
3. Сетевой модуль W5500 ТСР/IP (Ethernet):  
SPI2_MISO      &ensp;&ensp;PC2  
SPI2_MOSI      &ensp;&ensp;PC3  
SPI2_SCK       &ensp;&ensp;&ensp;PB10  
W5500_RST      &ensp;PC7  
W5500_CS       &ensp;&ensp;PC6  
3V3            &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;CN7-16(3V3)  
GND            &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;CN7-19(GND)  
4. (K9F1G08U0D) NandFlash Board (A):  
D0-D7          &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;PB0-PB7  
FLASH_CE       &ensp;&ensp;PA10  
FLASH_WE       &ensp;&ensp;PA9  
FLASH_RE       &ensp;&ensp;PA8  
FLASH_CLE      &ensp;PA5  
FLASH_ALE      &ensp;PA6  
FLASH_RB       &ensp;&ensp;PA7  
WP             &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;CN6-4(3V3)  
GND            &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;CN7-20(GND)
***
### Настройка Модулей.  
1. Настройка DMA:  
SPI2_RX    &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;DMA1 Stream 3  &ensp;&ensp;Peripheral To Memory  &ensp;&ensp;Very High  
SPI2_TX    &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;DMA1 Stream 4  &ensp;&ensp;Memory To Peripheral  &ensp;&ensp;Very High  
USART2_RX  &ensp;&ensp;&ensp;DMA1 Stream 5  &ensp;&ensp;Peripheral To Memory  &ensp;&ensp;Very High  
USART2_TX  &ensp;&ensp;&ensp;DMA1 Stream 6  &ensp;&ensp;Memory To Peripheral  &ensp;&ensp;Very High  
MEMTOMEM   &ensp;&ensp;&ensp;DMA2 Stream 0  &ensp;&ensp;Memory To Memory      &ensp;&ensp;&ensp;Very High  
2. Настройка GPIO:  
GPIO_EXIT13  &ensp;&ensp;External Interrupt Mode with Faling edge trigger detection  &ensp;Pull-up  
SPI:&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;Maximum output speed  &ensp;Very High  
3. Настройка NVIC:  
SPI2              &ensp;global interrupt  &ensp;0  
USART2            &ensp;global interrupt  &ensp;0  
EXTI line[15:10]  &ensp;global interrupt  &ensp;2  
4. Настройка RCC:  
HSE  &ensp;Crystal/Ceramic Resonator  
LSE  &ensp;Disable  
5. Настройка SYS:  
Debug  &ensp;Serial Wire  
7. Настройка TIM2:  
Clock Source  &ensp;Internal Clock  
7. Настройка SPI2:  
Mode                  &ensp;Full-Duplex Master  
Hardware NSS Signal   &ensp;Disable  
   - Parameter Settings:  
     - Frame Format      &ensp;Motorola  
     - Data Size         &ensp;8 Bits  
     - First Bit         &ensp;MSB First  
     - Prescaler         &ensp;2  
     - Clock (CPOL)      &ensp;Low  
     - Clock (CPHA)      &ensp;1 Edge  
     - CRC Calculatoin   &ensp;Disabled  
     - NSS Signal Type   &ensp;Software  
8. Настройка USART2:  
Mode                        &ensp;Asynchronous  
HardwareFlowControl(RS232)  &ensp;Disable  
   - Parameter Settings:  
     - Baud Rate               &ensp;115200  
     - Word Length             &ensp;8 Bits  
     - Parity                  &ensp;None  
     - Stop Bits               &ensp;1  
     - Data Direction          &ensp;Receive and Transmit  
     - Over Sampling           &ensp;16 Samples
***
Используя связку stm32 + NandFlash + W5500 для модуля W5500 может потребоваться доп. питание.  
***
