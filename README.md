***
#### W5500 TCPS SPI+DMA STM32F401RE-NUCLEO NAND K9F1G08U0D.  
TCP сервер на stm32f401re nucleo c сетевым модулем W5500 по SPI DMA бесконечно передает данные на скорости 5.5Мбит, и записывает их на NANDFlash память (без FSMC)(3.4Мбит). По нажатию кнопки отправка и запись прерывается и производится чтение всей NAND памяти.
***
##### Настойка пинов и тактирование.  
SYCLK(MHz) = 84  APB Prescaler = 1  APB1\APB2 Prescaler = 2  HCLK(MHz) = 84  
1. Отладка USART+DMA:  
 - |USART2_TX   |   PA2|  
 - |USART2_RX   |   PA3|  
2. Кнопка перывания для чтения данных с Nand:  
 - GPIO_EXIT13    PC13  
3. Сетевой модуль W5500 ТСР/IP (Ethernet):  
 - SPI2_MISO      PC2  
 - SPI2_MOSI      PC3  
 - SPI2_SCK       PB10  
 - W5500_RST      PC7  
 - W5500_CS       PC6  
 - 5V             CN7-16(3V3)  
 - GND            CN7-19(GND)  
4. (K9F1G08U0D) NandFlash Board (A):  
 - D0-D7          PB0-PB7  
 - FLASH_CE       PA10  
 - FLASH_WE       PA9  
 - FLASH_RE       PA8  
 - FLASH_CLE      PA5  
 - FLASH_ALE      PA6  
 - FLASH_RB       PA7  
 - WP             CN6-4(3V3)  
 - GND            CN7-20(GND)
***
##### Настройка Модулей.  
1. Настройка DMA:  
 - SPI2_RX    DMA1 Stream 3  Peripheral To Memory  Very High  
 - SPI2_TX    DMA1 Stream 4  Memory To Peripheral  Very High  
 - USART2_RX  DMA1 Stream 5  Peripheral To Memory  Very High  
 - USART2_TX  DMA1 Stream 6  Memory To Peripheral  Very High  
 - MEMTOMEM   DMA2 Stream 0  Memory To Memory      Very High  
2. Настройка GPIO:  
 - GPIO_EXIT13  External Interrupt Mode with Faling edge trigger detection  Pull-up  
   - SPI:  
     - (PC2 PC3 PB10) Maximum output speed  Very High  
3. Настройка NVIC:  
 - SPI2              global interrupt  0  
 - USART2            global interrupt  0  
 - EXTI line[15:10]  global interrupt  2  
4. Настройка RCC:  
 - HSE  Crystal/Ceramic Resonator  
 - LSE  Disable  
5. Настройка SYS:  
 - Debug  Serial Wire  
6. Настройка TIM2:  
 - Clock Source  Internal Clock  
7. Настройка SPI2:  
 - Mode                  Full-Duplex Master  
 - Hardware NSS Signal   Disable  
   - Parameter Settings:  
     - Frame Format      Motorola  
     - Data Size         8 Bits  
     - First Bit         MSB First  
     - Prescaler         2  
     - Clock (CPOL)      Low  
     - Clock (CPHA)      1 Edge  
     - CRC Calculatoin   Disabled  
     - NSS Signal Type   Software  
8. Настройка USART2:  
 - Mode                        Asynchronous  
 - HardwareFlowControl(RS232)  Disable  
   - Parameter Settings:  
     - Baud Rate               115200  
     - Word Length             8 Bits  
     - Parity                  None  
     - Stop Bits               1  
     - Data Direction          Receive and Transmit  
     - Over Sampling           16 Samples
***
Используя связку stm32 + NandFlash + W5500 для модуля W5500 может потребоваться доп. питание.  
***
