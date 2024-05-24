/* USER CODE BEGIN Header */
/**
    ******************************************************************************
    * @file                     : main.c
    * @brief                    : Main program body
    ******************************************************************************
    * @attention
    *
    * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
    * All rights reserved.</center></h2>
    *
    * This software component is licensed by ST under Ultimate Liberty license
    * SLA0044, the "License"; You may not use this file except in compliance with
    * the License. You may obtain a copy of the License at:
    *                                                         www.st.com/SLA0044
    *
    ******************************************************************************
    */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "FRAM.h"
#include "FPGA_UART.h"
#include "FPGA_Data_saving.h"
#include "GS_Telemetry.h"
#include "uC_Data_Saving.h"
#include "COBS.h"
#include "Space_Packet_Protocol.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c4;
DMA_HandleTypeDef hdma_i2c4_rx;
DMA_HandleTypeDef hdma_i2c4_tx;

SD_HandleTypeDef hsd1;
DMA_HandleTypeDef hdma_sdmmc1_rx;
DMA_HandleTypeDef hdma_sdmmc1_tx;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
DMA_HandleTypeDef hdma_uart4_rx;
DMA_HandleTypeDef hdma_uart4_tx;
DMA_HandleTypeDef hdma_uart5_rx;
DMA_HandleTypeDef hdma_uart5_tx;

DMA_HandleTypeDef hdma_memtomem_dma2_stream1;
SRAM_HandleTypeDef hsram1;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */

FATFS FatFs;
FIL logFileFPGATx;
FIL logFileFPGARx;
FIL stateFile;

#define UNIT_ID_CU 0x1A
#define UNIT_ID_EMUCONTROL 0x1B
#define UNIT_ID_EMUSCIENCE 0x1C
#define UNIT_ID_SMILE 0x1D
uint8_t unitID = 0;
uint8_t ffuID = 0;

uint8_t SPP_message_received = 0;

uint16_t ADCBuffer[11];		// Buffer for ADC values
uint16_t ADCValues[11];		// Current ADC values
float temperature = 0;
float uc3v = 0;
float fpga3v = 0;
float fpga1p5v = 0;
float vbat = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_SDMMC1_SD_Init(void);
static void MX_UART5_Init(void);
static void MX_I2C4_Init(void);
static void MX_FMC_Init(void);
static void MX_UART4_Init(void);
void StartDefaultTask(void const * argument);

static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
    * @brief    The application entry point.
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
    MX_ADC1_Init();
    MX_SDMMC1_SD_Init();
    MX_UART5_Init();
    MX_UART4_Init();
    MX_I2C4_Init();
    MX_FMC_Init();

    /* Initialize interrupts */
    MX_NVIC_Init();
    /* USER CODE BEGIN 2 */

    /* USER CODE END 2 */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    /* USER CODE END RTOS_QUEUES */

    /* Create the thread(s) */
    /* definition and creation of defaultTask */
    osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 2048);
    defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    /* USER CODE END RTOS_THREADS */

    /* Start scheduler */
    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
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
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 216;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Activate the Over-Drive mode
    */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                                            |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART4|RCC_PERIPHCLK_UART5
                                                            |RCC_PERIPHCLK_I2C4|RCC_PERIPHCLK_SDMMC1
                                                            |RCC_PERIPHCLK_CLK48;
    PeriphClkInitStruct.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
    PeriphClkInitStruct.Uart5ClockSelection = RCC_UART5CLKSOURCE_PCLK1;
    PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PCLK1;
    PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
    PeriphClkInitStruct.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_CLK48;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
    * @brief NVIC Configuration.
    * @retval None
    */
static void MX_NVIC_Init(void)
{
    /* SDMMC1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SDMMC1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
    /* DMA1_Stream0_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
    /* ADC_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(ADC_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(ADC_IRQn);
    /* DMA1_Stream7_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);
    /* UART5_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(UART5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(UART5_IRQn);
    /* DMA2_Stream3_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
    /* DMA2_Stream6_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
    /* DMA1_Stream1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
    /* DMA1_Stream5_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
    /* DMA2_Stream0_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
    /* EXTI9_5_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    /* DMA2_Stream1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
}

/**
    * @brief ADC1 Initialization Function
    * @param None
    * @retval None
    */
static void MX_ADC1_Init(void)
{

    /* USER CODE BEGIN ADC1_Init 0 */

    /* USER CODE END ADC1_Init 0 */

    ADC_ChannelConfTypeDef sConfig = {0};

    /* USER CODE BEGIN ADC1_Init 1 */

    /* USER CODE END ADC1_Init 1 */
    /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
    */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = ENABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 5;
    hadc1.Init.DMAContinuousRequests = ENABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }
    /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
    */
    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
    */
    sConfig.Channel = ADC_CHANNEL_6;
    sConfig.Rank = ADC_REGULAR_RANK_2;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
    */
    sConfig.Channel = ADC_CHANNEL_7;
    sConfig.Rank = ADC_REGULAR_RANK_3;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
    */
    sConfig.Channel = ADC_CHANNEL_14;
    sConfig.Rank = ADC_REGULAR_RANK_4;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
    */
    sConfig.Channel = ADC_CHANNEL_15;
    sConfig.Rank = ADC_REGULAR_RANK_5;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN ADC1_Init 2 */

    /* USER CODE END ADC1_Init 2 */

}

/**
    * @brief I2C4 Initialization Function
    * @param None
    * @retval None
    */
static void MX_I2C4_Init(void)
{

    /* USER CODE BEGIN I2C4_Init 0 */

    /* USER CODE END I2C4_Init 0 */

    /* USER CODE BEGIN I2C4_Init 1 */

    /* USER CODE END I2C4_Init 1 */
    hi2c4.Instance = I2C4;
    hi2c4.Init.Timing = 0x20404768;
    hi2c4.Init.OwnAddress1 = 0;
    hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c4.Init.OwnAddress2 = 0;
    hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_ENABLE;
    if (HAL_I2C_Init(&hi2c4) != HAL_OK)
    {
        Error_Handler();
    }
    /** Configure Analogue filter
    */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }
    /** Configure Digital filter
    */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN I2C4_Init 2 */

    /* USER CODE END I2C4_Init 2 */

}

/**
    * @brief SDMMC1 Initialization Function
    * @param None
    * @retval None
    */
static void MX_SDMMC1_SD_Init(void)
{

    /* USER CODE BEGIN SDMMC1_Init 0 */

    /* USER CODE END SDMMC1_Init 0 */

    /* USER CODE BEGIN SDMMC1_Init 1 */

    /* USER CODE END SDMMC1_Init 1 */
    hsd1.Instance = SDMMC1;
    hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    hsd1.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
    hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B;
    hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd1.Init.ClockDiv = 0;
    /* USER CODE BEGIN SDMMC1_Init 2 */

    /* USER CODE END SDMMC1_Init 2 */

}

/**
    * @brief UART4 Initialization Function
    * @param None
    * @retval None
    */
static void MX_UART4_Init(void)
{

    /* USER CODE BEGIN UART4_Init 0 */

    /* USER CODE END UART4_Init 0 */

    /* USER CODE BEGIN UART4_Init 1 */

    /* USER CODE END UART4_Init 1 */
    huart4.Instance = UART4;
    huart4.Init.BaudRate = 115200;
    huart4.Init.WordLength = UART_WORDLENGTH_8B;
    huart4.Init.StopBits = UART_STOPBITS_1;
    huart4.Init.Parity = UART_PARITY_NONE;
    huart4.Init.Mode = UART_MODE_TX_RX;
    huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling = UART_OVERSAMPLING_16;
    huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart4) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN UART4_Init 2 */

    /* USER CODE END UART4_Init 2 */

}

/**
    * @brief UART5 Initialization Function
    * @param None
    * @retval None
    */
static void MX_UART5_Init(void)
{

    /* USER CODE BEGIN UART5_Init 0 */

    /* USER CODE END UART5_Init 0 */

    /* USER CODE BEGIN UART5_Init 1 */

    /* USER CODE END UART5_Init 1 */
    huart5.Instance = UART5;
    huart5.Init.BaudRate = 115200;
    huart5.Init.WordLength = UART_WORDLENGTH_8B;
    huart5.Init.StopBits = UART_STOPBITS_1;
    huart5.Init.Parity = UART_PARITY_NONE;
    huart5.Init.Mode = UART_MODE_TX_RX;
    huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart5.Init.OverSampling = UART_OVERSAMPLING_16;
    huart5.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart5.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart5) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN UART5_Init 2 */

    /* USER CODE END UART5_Init 2 */

}

/**
    * Enable DMA controller clock
    * Configure DMA for memory to memory transfers
    *     hdma_memtomem_dma2_stream1
    */
static void MX_DMA_Init(void)
{
    /* DMA controller clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* Configure DMA request hdma_memtomem_dma2_stream1 on DMA2_Stream1 */
    hdma_memtomem_dma2_stream1.Instance = DMA2_Stream1;
    hdma_memtomem_dma2_stream1.Init.Channel = DMA_CHANNEL_0;
    hdma_memtomem_dma2_stream1.Init.Direction = DMA_MEMORY_TO_MEMORY;
    hdma_memtomem_dma2_stream1.Init.PeriphInc = DMA_PINC_ENABLE;
    hdma_memtomem_dma2_stream1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_memtomem_dma2_stream1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_memtomem_dma2_stream1.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_memtomem_dma2_stream1.Init.Mode = DMA_NORMAL;
    hdma_memtomem_dma2_stream1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_memtomem_dma2_stream1.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_memtomem_dma2_stream1.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_memtomem_dma2_stream1.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_memtomem_dma2_stream1.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&hdma_memtomem_dma2_stream1) != HAL_OK)
    {
        Error_Handler( );
    }

    /* DMA interrupt init */
    /* DMA1_Stream2_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
    /* DMA1_Stream4_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

}
/* FMC initialization function */
static void MX_FMC_Init(void)
{
    FMC_NORSRAM_TimingTypeDef Timing;

    /** Perform the SRAM1 memory initialization sequence
    */
    hsram1.Instance = FMC_NORSRAM_DEVICE;
    hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
    /* hsram1.Init */
    hsram1.Init.NSBank = FMC_NORSRAM_BANK1;
    hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_ENABLE;
    hsram1.Init.MemoryType = FMC_MEMORY_TYPE_PSRAM;
    hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_8;
    hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_ENABLE;
    hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
    hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_DISABLE;
    hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
    hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
    hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram1.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
    hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
    hsram1.Init.WriteFifo = FMC_WRITE_FIFO_DISABLE;
    hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;
    /* Timing */
    Timing.AddressSetupTime = 15;
    Timing.AddressHoldTime = 15;
    Timing.DataSetupTime = 255;
    Timing.BusTurnAroundDuration = 15;
    Timing.CLKDivision = 6;
    Timing.DataLatency = 2;
    Timing.AccessMode = FMC_ACCESS_MODE_A;
    /* ExtTiming */

    if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
    {
        Error_Handler( );
    }

}

/**
    * @brief GPIO Initialization Function
    * @param None
    * @retval None
    */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(UC_CONSOLE_EN_GPIO_Port, UC_CONSOLE_EN_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(SD_SW_A_GPIO_Port, SD_SW_A_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, LED4_Pin|LED3_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin : UC_CONSOLE_EN_Pin */
    GPIO_InitStruct.Pin = UC_CONSOLE_EN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(UC_CONSOLE_EN_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : SD_SW_B_Pin */
    GPIO_InitStruct.Pin = SD_SW_B_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(SD_SW_B_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : SD_SW_A_Pin */
    GPIO_InitStruct.Pin = SD_SW_A_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SD_SW_A_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : FPGA_BUF_INT_Pin */
    GPIO_InitStruct.Pin = FPGA_BUF_INT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(FPGA_BUF_INT_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : LED4_Pin LED3_Pin */
    GPIO_InitStruct.Pin = LED4_Pin|LED3_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_SRAM_DMA_XferCpltCallback(DMA_HandleTypeDef *hdma) {
	FPGADMATransferCplt();
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &huart5) {
		FPGA_RX_CpltCallback();
	} else if (huart == &SPP_DEBUG_UART) {
        *(OBCRxBuffer + SPP_recv_count) = SPP_recv_char;
        if (SPP_recv_char == 0x00) {
            SPP_message_received = 1;
            SPP_recv_count = 0;
        } else {
            SPP_recv_count++;
        }
        HAL_UART_Receive_DMA(&SPP_DEBUG_UART, &SPP_recv_char, 1);
	}

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    memcpy(ADCValues, ADCBuffer, 22);

    temperature = ((float) ADCValues[0]) * 0.000732421875 + 20;
    vbat = ((float) ADCValues[1]) * 0.000732421875 * 2;
    fpga3v = ((float) ADCValues[2]) * 0.000732421875 * 1.33;
    uc3v = ((float) ADCValues[3]) * 0.000732421875 * 1.33;
    fpga1p5v = ((float) ADCValues[4]) * 0.000732421875;

    ADCNewData = 1;
}

// Enables print() to terminal over SWD
int _write(int file, char *ptr, int len)
{
    int i=0;
    for(i=0 ; i<len ; i++)
        ITM_SendChar((*ptr++));
    return len;
}



/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
    * @brief    Function implementing the defaultTask thread.
    * @param    argument: Not used 
    * @retval None
    */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
    /* init code for FATFS */
    MX_FATFS_Init();

    /* USER CODE BEGIN 5 */

    // Initialize SD card
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
    //BSP_SD_Init();
    //f_mount(&FatFs, (TCHAR const*) SDPath, 0);

    // Start listening on UART5 (FPGA)
    HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 4);
    HAL_UART_Receive_DMA(&SPP_DEBUG_UART, &SPP_recv_char, 1);

    { // Update boot count in FRAM
	    uint16_t boot_cnt = 0;
	    readFRAM(FRAM_BOOT_CNT, (uint8_t*) &boot_cnt, 2);
	    boot_cnt = boot_cnt + 1;
	    writeFRAM(FRAM_BOOT_CNT, (uint8_t*) &boot_cnt, 2);
	    printf("Boot count is now %u\n", boot_cnt);
    }

    // Read out uC GS identifier from FRAM
    readFRAM(FRAM_GS_ID_UC, &uC_GS_ID, 1);

    { // Boot up FPGA transmissions. Send FFU ID, Unit ID and GS ID stored in FRAM to FPGA.
	    uint8_t gsid_fpga;

	    readFRAM(FRAM_FFU_ID, &ffuID, 1);
	    readFRAM(FRAM_UNIT_ID, &unitID, 1);
	    readFRAM(FRAM_GS_ID_FPGA, &gsid_fpga, 1);

	    uint8_t ffuID_TX[5] = {0xB5, 0x43, 0x46, ffuID, 0x0A};
	    uint8_t unitID_TX[5] = {0xB5, 0x43, 0x55, unitID, 0x0A};
	    uint8_t gsID_TX[5] = {0xB5, 0x43, 0x47, gsid_fpga, 0x0A};

	    FPGA_Transmit_Binary(ffuID_TX, 5);
	    FPGA_Transmit_Binary(unitID_TX, 5);
	    FPGA_Transmit_Binary(gsID_TX, 5);
    }

    // Appoint DMA to FMC
    hsram1.hdma = &hdma_memtomem_dma2_stream1;

    // Start ADC DMA transfers
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*) ADCBuffer, 5);
    ADCPacket[0] = 'A';
    ADCPacket[19] = '\n';

    //uint8_t SD_Card_switch = 1;
    //SD_Card_switch = HAL_GPIO_ReadPin(SD_SW_B_GPIO_Port, SD_SW_B_Pin);

    FRESULT result = 0;

    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
/*
    if (SD_Card_switch == 0) {
	    result = openFPGADataFile();

	    if (result == FR_OK) {
		    FPGAFileOpen = 1;
	    }
	    else {
		    FPGAFileOpen = 0;
		    HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET);
	    }

	    result = openUCDataFile();

	    if (result == FR_OK) {
		    uCFileOpen = 1;
	    }
	    else {
		    uCFileOpen = 0;
		    HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET);
	    }
    }
    else
	    HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET);
*/
    uint32_t current_ticks = 0;
    uint32_t ucFileTicks = 0;
    uint32_t SPP_test_ticks = 0;
    uint8_t oldFlightState = 0;
/*
	f_open(&stateFile, "/FLIGHT_STATES.log", FA_OPEN_APPEND | FA_WRITE);
	char bootstr[256];
	sprintf(bootstr, "Boot completed, entering main loop at t = %ld ms\n", current_ticks);
	f_write(&stateFile, bootstr, strlen(bootstr), 0);
	f_close(&stateFile);
*/
    /* Infinite loop */
    for(;;) {
	    current_ticks = xTaskGetTickCount();

        if (SPP_message_received) {
            SPP_handle_incoming_TC();
            SPP_message_received = 0;
        }
        
        if (current_ticks - SPP_test_ticks > 5000) {
            HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
            //SPP_send_HK_test_packet();
            SPP_test_ticks = current_ticks;
        }

	    if (FPGAFlightState < 7)
		    HandleFPGAStream();
/*
	    // uC Binary file
	    if (FPGAFlightState > 2 && FPGAFlightState < 7) {
		    if (uCFileOpen) {
			    if (current_ticks - ucFileTicks > 1000) {	// Sync uC file every sec
				    f_sync(&uCDataFile);
				    ucFileTicks = current_ticks;
			    }

			    if (ADCNewData) {
				    ADCNewData = 0;
				    memcpy(&ADCPacket[1], &current_ticks, 4);
				    memcpy(&ADCPacket[5], ADCValues, 2);
				    memcpy(&ADCPacket[9], &ADCValues[1], 8);
				    f_write(&uCDataFile, ADCPacket, 20, 0);
			    }
		    }
	    }
*/
	    // This pin controls UART line switching inside the FPGA
	    if (console_enabled)
		    HAL_GPIO_WritePin(UC_CONSOLE_EN_GPIO_Port, UC_CONSOLE_EN_Pin, GPIO_PIN_SET);
	    else
		    HAL_GPIO_WritePin(UC_CONSOLE_EN_GPIO_Port, UC_CONSOLE_EN_Pin, GPIO_PIN_RESET);

	    // Console mode - UART bypasses FPGA
	    if (ConsoleCommandReady) {
		    HandleConsole(); // Declared in FPGA_UART.c
	    }

	    if (FPGAFlightState != oldFlightState) {
		    	//char str[256];
			    oldFlightState = FPGAFlightState;
/*
			f_open(&stateFile, "/FLIGHT_STATES.log", FA_OPEN_APPEND | FA_WRITE);
			sprintf(str, "Time: %ld, State: %d\n", current_ticks, FPGAFlightState);
			f_write(&stateFile, str, strlen(str), 0);
			f_close(&stateFile);

	    }

	    if (FPGAFlightState == STATE_LANDED) {
		    if (FPGAFileOpen) {
			    f_close(&FPGADataFile);
			    FPGAFileOpen = 0;
		    }
		    if (uCFileOpen) {
			    f_close(&uCDataFile);
			    uCFileOpen = 0;
		    }
*/
	    }
    }
    /* USER CODE END 5 */
}

/**
    * @brief    Period elapsed callback in non blocking mode
    * @note     This function is called    when TIM1 interrupt took place, inside
    * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
    * a global variable "uwTick" used as application time base.
    * @param    htim : TIM handle
    * @retval None
    */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /* USER CODE BEGIN Callback 0 */

    /* USER CODE END Callback 0 */
    if (htim->Instance == TIM1) {
        HAL_IncTick();
    }
    /* USER CODE BEGIN Callback 1 */

    /* USER CODE END Callback 1 */
}

/**
    * @brief    This function is executed in case of error occurrence.
    * @retval None
    */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */

    /* USER CODE END Error_Handler_Debug */
}

#ifdef    USE_FULL_ASSERT
/**
    * @brief    Reports the name of the source file and the source line number
    *                 where the assert_param error has occurred.
    * @param    file: pointer to the source file name
    * @param    line: assert_param error line source number
    * @retval None
    */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
         tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
