/* USER CODE BEGIN Header */

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "socket.h"
#include "w5500.h"
#include "wizchip_conf.h"
#include "stdarg.h"
#include <string.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint8_t stat;
static uint8_t spi_tx_buf[8192];
static uint8_t spi_rx_buf[8192];
volatile bool spi_dma_completed = false;
volatile bool uartTxComplete = false;

#define START_PAGE 0
#define START_PAGE_READ 0
uint32_t current_page = START_PAGE;
uint32_t current_read_page = START_PAGE_READ;
#define BUFFER_SIZE 16384
uint8_t write_buffer[BUFFER_SIZE];
uint8_t read_buffer[BUFFER_SIZE];
#define PAGE_SIZE               2048
#define PAGES_PER_BLOCK         64
#define TOTAL_BLOCKS            4
#define TOTAL_PAGES             (TOTAL_BLOCKS * PAGES_PER_BLOCK)
#define WRITE_BUFFER_SIZE       PAGE_SIZE
#define READ_BUFFER_SIZE        PAGE_SIZE

volatile uint8_t button_pressed_flag = 0;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_rx;
DMA_HandleTypeDef hdma_spi2_tx;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

DMA_HandleTypeDef hdma_memtomem_dma2_stream0;
/* USER CODE BEGIN PV */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi){
    if (hspi == &hspi2) {
        spi_dma_completed = true;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
    if(GPIO_Pin == GPIO_PIN_13){
        button_pressed_flag = 1;
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
    if (huart->Instance == USART2){
        uartTxComplete = true;
    }
}

void UART_SendString(char* str){
	uartTxComplete = false;
	HAL_UART_Transmit_DMA(&huart2, (uint8_t*)str, strlen(str));
	while (!uartTxComplete);
}

void UART_SendDataHex(uint8_t* data, uint32_t length){
    char buffer[256];
    for(uint32_t i = 0; i < length; i++){
        sprintf(buffer, "%02X ", data[i]);
        UART_SendString(buffer);
        if((i+1) % 16 == 0)
            UART_SendString("\r\n");
    }
    UART_SendString("\r\n");
}

uint8_t NAND_ReadStatus() {
    uint8_t status = 0;
    NAND_SendCommand(0x70);
    status = NAND_ReadData();
    return status;
}

void UART_SendNumber(uint32_t number) {
    char num_str[12];
    snprintf(num_str, sizeof(num_str), "%lu\r\n", number);
    UART_SendString(num_str);
}

int NAND_WaitReady(void) {
    const uint32_t timeout = 1;
    uint32_t start = HAL_GetTick();
    while (HAL_GPIO_ReadPin(FLASH_RB_GPIO_Port, FLASH_RB_Pin) == GPIO_PIN_RESET) {
        if ((HAL_GetTick() - start) > timeout) {
            return -1;
        }
    }
    return 0;
}

void NAND_SendCommand(uint8_t command){
    FLASH_CLE_GPIO_Port->BSRR = FLASH_CLE_Pin;
    FLASH_ALE_GPIO_Port->BSRR = (uint32_t)FLASH_ALE_Pin << 16U;
    GPIOB->ODR = command;
    FLASH_WE_GPIO_Port->BSRR = (uint32_t)FLASH_WE_Pin << 16U;      // Сбросить WE (RESET - начало импульса WE)
    FLASH_WE_GPIO_Port->BSRR = FLASH_WE_Pin;                       // Установить WE (SET - конец импульса WE)
    FLASH_CLE_GPIO_Port->BSRR = (uint32_t)FLASH_CLE_Pin << 16U;    // Сбросить CLE (RESET)
}

void NAND_SendAddress(uint8_t address){
    FLASH_ALE_GPIO_Port->BSRR = FLASH_ALE_Pin;                      // Установить ALE (SET)
    FLASH_CLE_GPIO_Port->BSRR = (uint32_t)FLASH_CLE_Pin << 16U;     // Сбросить CLE (RESET)
    GPIOB->ODR = address;
    FLASH_WE_GPIO_Port->BSRR = (uint32_t)FLASH_WE_Pin << 16U;       // Сбросить WE (RESET - начало импульса WE)
    FLASH_WE_GPIO_Port->BSRR = FLASH_WE_Pin;                        // Установить WE (SET - конец импульса WE)
    FLASH_ALE_GPIO_Port->BSRR = (uint32_t)FLASH_ALE_Pin << 16U;     // Сбросить ALE (RESET)
}

void NAND_WriteData(uint8_t data){
    FLASH_CLE_GPIO_Port->BSRR = (uint32_t)FLASH_CLE_Pin << 16U;     // Сбросить CLE (RESET)
    FLASH_ALE_GPIO_Port->BSRR = (uint32_t)FLASH_ALE_Pin << 16U;     // Сбросить ALE (RESET)
    GPIOB->MODER &= ~0x0000FFFF;
    GPIOB->MODER |= 0x00005555;
    GPIOB->ODR = (GPIOB->ODR & ~0xFF) | (data & 0xFF);
    FLASH_WE_GPIO_Port->BSRR = (uint32_t)FLASH_WE_Pin << 16U;       // Сбросить WE (RESET)
    FLASH_WE_GPIO_Port->BSRR = FLASH_WE_Pin;                        // Установить WE (SET)
}

uint8_t NAND_ReadData(void) {
    uint8_t data;
    GPIOB->MODER &= ~0x0000FFFF;
    GPIOB->MODER |= 0x00004444;
    FLASH_RE_GPIO_Port->BSRR = (uint32_t)FLASH_RE_Pin << 16U;
    data = (uint8_t)(GPIOB->IDR & 0xFF);
    FLASH_RE_GPIO_Port->BSRR = FLASH_RE_Pin; // SET RE
    GPIOB->MODER &= ~0x0000FFFF;
    GPIOB->MODER |= 0x00005555;
    return data;
}

void NAND_EraseBlock(uint32_t block) {
    char debug_msg[50];
    NAND_SendCommand(0x60);
    uint32_t page_address = block * PAGES_PER_BLOCK;
    NAND_SendAddress((uint8_t)(0x00));
    NAND_SendAddress((uint8_t)(0x00));
    NAND_SendAddress((uint8_t)(page_address & 0xFF));
    NAND_SendAddress((uint8_t)((page_address >> 8) & 0xFF));
    NAND_SendAddress((uint8_t)((page_address >> 16) & 0xFF));
    NAND_SendCommand(0xD0);
    snprintf(debug_msg, "Block %lu erased successfully\r\n", block);
    //UART_SendString(debug_msg);
}

void NAND_WritePage(uint32_t page, uint32_t column, uint8_t* data) {
    NAND_SendCommand(0x80);
    NAND_SendAddress((uint8_t)(column & 0xFF));             // Колонка (низкий байт)
    NAND_SendAddress((uint8_t)((column >> 8) & 0xFF));      // Колонка (высокий байт)
    NAND_SendAddress((uint8_t)(page & 0xFF));               // Страница (байт 0)
    NAND_SendAddress((uint8_t)((page >> 8) & 0xFF));        // Страница (байт 1)
    NAND_SendAddress((uint8_t)((page >> 16) & 0xFF));       // Страница (байт 2)
    for (int i = 0; i < PAGE_SIZE; i++) {
        NAND_WriteData(data[i]);
    }
    HAL_GPIO_WritePin(FLASH_WE_GPIO_Port, FLASH_WE_Pin, GPIO_PIN_RESET);
    NAND_SendCommand(0x10);
    if (NAND_WaitReady() != 0) {
        UART_SendString("Write Operation Timeout\r\n");
    }
}

void NAND_ReadPage(uint32_t page, uint32_t column, uint8_t* buffer) {
    NAND_SendCommand(0x00);
    NAND_SendAddress((uint8_t)(column & 0xFF));             // Колонка (низкий байт)
    NAND_SendAddress((uint8_t)((column >> 8) & 0xFF));      // Колонка (высокий байт)
    NAND_SendAddress((uint8_t)(page >> 16 & 0xFF));         // Страница (байт 0)
    NAND_SendAddress((uint8_t)((page >> 8) & 0xFF));        // Страница (байт 1)
    NAND_SendAddress((uint8_t)((page >> 16) & 0xFF));       // Страница (байт 2)
    NAND_SendCommand(0x30);
    if (NAND_WaitReady() != 0){
        UART_SendString("Read Operation Timeout\r\n");
        return;
    }
}

void ReadAllPages(){
	uint32_t page_count = 0;
	uint32_t total_pages = TOTAL_BLOCKS * PAGES_PER_BLOCK;
	UART_SendString("Start read all pages...\r\n");
	UART_SendString("Total pages: ");
	UART_SendNumber(total_pages);
	UART_SendString("\r\n");
    for(uint32_t block = 0; block < TOTAL_BLOCKS; block++){
    	for(uint32_t page_in_block = 0; page_in_block < PAGES_PER_BLOCK; page_in_block++){
    		current_page = block * PAGES_PER_BLOCK + page_in_block;
    		for(uint32_t i = 0; i < READ_BUFFER_SIZE; i++) {
    			read_buffer[i] = (uint8_t)((current_page + i) & 0xFF);
    		}
    		NAND_ReadPage(current_page, 0, read_buffer);
    		page_count++;
            UART_SendString("Page ");
            UART_SendNumber(page_count);
            //UART_SendString("Read Data: \n");
            //UART_SendDataHex(read_buffer, READ_BUFFER_SIZE);
    	}
    }
}

wiz_NetInfo netInfo = {.mac = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef},
                       .ip = {192, 168, 1, 218},
                       .sn = {255, 255, 255, 0},
                       .gw = {192, 168, 1, 1},
                       .dns = {0, 0, 0, 0},
                       .dhcp = NETINFO_STATIC };

void wizchip_select(void){
	(W5500_CS_GPIO_Port->BSRR = (uint32_t)W5500_CS_Pin << 16U);
}

void wizchip_deselect(void){
	(W5500_CS_GPIO_Port->BSRR = W5500_CS_Pin);
}

void wizchip_reset(void){
	(W5500_RST_GPIO_Port->BSRR = (uint32_t)W5500_RST_Pin << 16U);
	(W5500_RST_GPIO_Port->BSRR = W5500_RST_Pin);
}

void wizchip_writebyte(uint8_t wb){
    uint8_t rb;
    spi_dma_completed = false;
    HAL_SPI_TransmitReceive_DMA(&hspi2, &wb, &rb, 1);
    while (!spi_dma_completed);
}

uint8_t wizchip_readbyte(void){
    uint8_t rb = 0;
    uint8_t wb = 0xFF;
    spi_dma_completed = false;
    HAL_SPI_TransmitReceive_DMA(&hspi2, &wb, &rb, 1);
    while (!spi_dma_completed);
    return rb;
}

void wizchip_readburst(uint8_t* pBuf, uint16_t len){
    spi_dma_completed = false;
    HAL_SPI_TransmitReceive_DMA(&hspi2, spi_tx_buf, pBuf, len);
    while (!spi_dma_completed);
}

void wizchip_writeburst(uint8_t* pBuf, uint16_t len){
    spi_dma_completed = false;
    HAL_SPI_TransmitReceive_DMA(&hspi2, pBuf, spi_rx_buf, len);
    while (!spi_dma_completed);
}

void tcp_server_init() {
    if(stat = socket(0, Sn_MR_TCP, 5000, 0) == 0)
    	UART_SendString("Socket init success\r\n");
    if(stat = getSn_SR(0)== SOCK_INIT)
    	UART_SendString("Socket open success\r\n");
    if(stat = listen(0) == SOCK_OK)
    	UART_SendString("Listen socket success\r\n");
    while(getSn_SR(0) == SOCK_LISTEN)
        {
    			HAL_Delay(0);
        }
    if(getSn_SR(0) == SOCK_ESTABLISHED){
    	UART_SendString("Input connection\r\n");
    }
}

void tcp_send(){
    uint32_t current_page = 0;
    uint32_t success_count = 0;
    static uint8_t initialized = 0;
    if(!initialized){
    	for(uint32_t i = 0; i < WRITE_BUFFER_SIZE; i++) {
    		write_buffer[i] = (uint8_t)(i & 0xFF);
    	}
    	initialized = 1;
    }
    if (getSn_SR(0) == SOCK_ESTABLISHED) {
    	while (BUFFER_SIZE > 0) {
    		if(button_pressed_flag){
    			button_pressed_flag = 0;
    			ReadAllPages();
    			return;
    		}
        	int32_t nbytes = send(0, write_buffer, BUFFER_SIZE);
        	if (nbytes > 0) {
        		//UART_SendString("Sending data and writing to NAND...\r\n");
        		if (current_page >= TOTAL_PAGES) {
        			UART_SendString("NAND Flash is full.\r\n");
        		    break;
        		}
                uint32_t block = current_page / PAGES_PER_BLOCK;
                uint32_t page_in_block = current_page % PAGES_PER_BLOCK;
                if (page_in_block == 0) {
                    UART_SendString("Erasing Block ");
                    UART_SendNumber(block);
                    //UART_SendString("...\r\n");
                    NAND_EraseBlock(block);
                }
                for(uint32_t i = 0; i < WRITE_BUFFER_SIZE; i++) {
                	write_buffer[i] = (uint8_t)((current_page + i) & 0xFF);
                }
                NAND_WritePage(current_page, 0, write_buffer);
            	//UART_SendNumber(block);
                //UART_SendString("\r\n");
                //UART_SendString("Written Data: \n");
                //UART_SendDataHex(write_buffer, WRITE_BUFFER_SIZE);
                for(uint32_t i = 0; i < READ_BUFFER_SIZE; i++) {
                	read_buffer[i] = (uint8_t)((current_page + i) & 0xFF);
                }
                NAND_ReadPage(current_page, 0, read_buffer);
                //UART_SendString("Read Data: \n");
                //UART_SendDataHex(read_buffer, READ_BUFFER_SIZE);
                if(memcmp(write_buffer, read_buffer, WRITE_BUFFER_SIZE) != 0) {
                	UART_SendString("Data verification FAILED at page ");
                    UART_SendNumber(current_page);
                    UART_SendString("\r\n");
                } else {
                	//UART_SendString("Data verification PASSED\r\n");
                	//UART_SendNumber(block);
                    success_count++;
                }
                current_page++;
                if(current_page >= TOTAL_PAGES) {
                	UART_SendString("All pages written and verified. Restarting.\r\n");
                	current_page = 0;
                	success_count = 0;
                }
                //UART_SendString("Success Count: ");
                UART_SendNumber(success_count);
                //UART_SendString("\r\n");
        	}
        	else {
        		close(0);
        		UART_SendString("Socket closed.\r\n\n");
                break;
        	}
    	}
    }
    else {
        UART_SendString("Socket not established.\r\n");
    }
}

void w5500_init() {
  wizchip_reset();
  reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
  reg_wizchip_spi_cbfunc(wizchip_readbyte, wizchip_writebyte);
  reg_wizchip_spiburst_cbfunc(wizchip_readburst, wizchip_writeburst);
  uint8_t memsize[2][8] = {{16,0,0,0,0,0,0,0},{16,0,0,0,0,0,0,0}};
  wizchip_init(memsize, memsize);
  wizchip_setnetinfo(&netInfo);
  ctlnetwork(CN_SET_NETINFO, (void*) &netInfo);
  HAL_Delay(10);
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	w5500_init();
	tcp_server_init();
	tcp_send();
	while(!button_pressed_flag){
		HAL_Delay(1);
	}
	button_pressed_flag = 0;
	ReadAllPages();
	HAL_Delay(100);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  * Configure DMA for memory to memory transfers
  *   hdma_memtomem_dma2_stream0
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* Configure DMA request hdma_memtomem_dma2_stream0 on DMA2_Stream0 */
  hdma_memtomem_dma2_stream0.Instance = DMA2_Stream0;
  hdma_memtomem_dma2_stream0.Init.Channel = DMA_CHANNEL_0;
  hdma_memtomem_dma2_stream0.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma2_stream0.Init.PeriphInc = DMA_PINC_ENABLE;
  hdma_memtomem_dma2_stream0.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma2_stream0.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_memtomem_dma2_stream0.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_memtomem_dma2_stream0.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma2_stream0.Init.Priority = DMA_PRIORITY_VERY_HIGH;
  hdma_memtomem_dma2_stream0.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
  hdma_memtomem_dma2_stream0.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  hdma_memtomem_dma2_stream0.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma_memtomem_dma2_stream0.Init.PeriphBurst = DMA_PBURST_SINGLE;
  if (HAL_DMA_Init(&hdma_memtomem_dma2_stream0) != HAL_OK)
  {
    Error_Handler( );
  }

  /* DMA interrupt init */
  /* DMA1_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
  /* DMA1_Stream4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, FLASH_CLE_Pin|FLASH_ALE_Pin|FLASH_RE_Pin|FLASH_WE_Pin
                          |FLASH_CE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, D0_Pin|D1_Pin|D2_Pin|D3_Pin
                          |D4_Pin|D5_Pin|D6_Pin|D7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, W5500_CS_Pin|W5500_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : FLASH_CLE_Pin FLASH_ALE_Pin FLASH_RE_Pin FLASH_WE_Pin
                           FLASH_CE_Pin */
  GPIO_InitStruct.Pin = FLASH_CLE_Pin|FLASH_ALE_Pin|FLASH_RE_Pin|FLASH_WE_Pin
                          |FLASH_CE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : FLASH_RB_Pin */
  GPIO_InitStruct.Pin = FLASH_RB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(FLASH_RB_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : D0_Pin D1_Pin D2_Pin D3_Pin
                           D4_Pin D5_Pin D6_Pin D7_Pin */
  GPIO_InitStruct.Pin = D0_Pin|D1_Pin|D2_Pin|D3_Pin
                          |D4_Pin|D5_Pin|D6_Pin|D7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : W5500_CS_Pin W5500_RST_Pin */
  GPIO_InitStruct.Pin = W5500_CS_Pin|W5500_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
