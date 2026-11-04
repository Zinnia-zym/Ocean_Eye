/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "ssd1306.h"
#include "soft_i2c.h"
#include "ssd1306_fonts.h"
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
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
#define LED_RED_PORT      GPIOB
#define LED_RED_PIN       GPIO_PIN_3

#define LED_GREEN_PORT    GPIOB
#define LED_GREEN_PIN     GPIO_PIN_4

#define LED_BLUE_PORT     GPIOB
#define LED_BLUE_PIN      GPIO_PIN_5

#define US_TRIG_PORT      GPIOC
#define US_TRIG_PIN       GPIO_PIN_7

uint8_t RxData;
uint8_t us_rx[8];
volatile uint16_t distance_mm = 0;
volatile uint8_t distance_ready = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
void LED_Set(uint8_t red, uint8_t green, uint8_t blue);
void Ultrasonic_Trigger(void);
void PC_SendText(char *text);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LED_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOB,
                      GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5,
                      GPIO_PIN_RESET);
}

void LED_Set(uint8_t red, uint8_t green, uint8_t blue)
{
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN,
                      red ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN,
                      green ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LED_BLUE_PORT, LED_BLUE_PIN,
                      blue ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Ultrasonic_Trigger(void)
{
    /* 对应传感器参考手册 */
    HAL_GPIO_WritePin(US_TRIG_PORT, US_TRIG_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(US_TRIG_PORT, US_TRIG_PIN, GPIO_PIN_SET);
}

void PC_SendText(char *text)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)text, strlen(text), 100);
}
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
  LED_GPIO_Init();

  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  HAL_UART_Receive_IT(&huart3, &us_rx[0], 1);

  /* 上电跑灯：红 -> 绿 -> 蓝 */
  LED_Set(1, 0, 0);
  HAL_Delay(300);

  LED_Set(0, 1, 0);
  HAL_Delay(300);

  LED_Set(0, 0, 1);
  HAL_Delay(300);

  LED_Set(0, 0, 0);
  PC_SendText("STM32 ready\r\n");

  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart1, &RxData, 1);
  ssd1306_Init();
  ssd1306_Fill(Black);
  ssd1306_UpdateScreen();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // 1. 触发超声波测距
    Ultrasonic_Trigger();
    HAL_Delay(50);

    // 2. 处理距离数据
    if (distance_ready)
    {
        distance_ready = 0;

        if (distance_mm == 65533)
        {
            // 异常情况：无回波 / 超量程
            PC_SendText("useless\r\n");
            ssd1306_SetCursor(0, 0);
            ssd1306_WriteString((char*)"useless        ", Font_11x18, White);
            ssd1306_UpdateScreen();
            LED_Set(1, 0, 0);   // 红灯
        }
        else
        {
            // 正常情况：有真实距离
            char msg[32];
            sprintf(msg, "Dist: %d mm\r\n", distance_mm);
            PC_SendText(msg);

            char oled_buf[24];
            sprintf(oled_buf, "Dist: %d mm    ", distance_mm);
            ssd1306_SetCursor(0, 0);
            ssd1306_WriteString(oled_buf, Font_11x18, White);
            ssd1306_UpdateScreen();

            // 距离控制红绿灯
            if (distance_mm < 60 ) {
            LED_Set(1, 0, 0);
            } else if (distance_mm <= 350) {
            LED_Set(0, 1, 0);  // 绿灯
            } else {
            LED_Set(1, 0, 0);  // 红灯
}
        }
    }
    else
    {
        // 没有任何数据时
        PC_SendText("No echo\r\n");
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString((char*)"No echo waiting", Font_11x18, White);
        ssd1306_UpdateScreen();
    }

    HAL_Delay(500);
    /* USER CODE END 3 */
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(US_TRIG_GPIO_Port, US_TRIG_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_RED_Pin|LED_GREEN_Pin|LED_BLUE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, OLED_SDA_Pin|OLED_SCL_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : US_TRIG_Pin */
  GPIO_InitStruct.Pin = US_TRIG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(US_TRIG_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_RED_Pin LED_GREEN_Pin LED_BLUE_Pin */
  GPIO_InitStruct.Pin = LED_RED_Pin|LED_GREEN_Pin|LED_BLUE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : OLED_SDA_Pin OLED_SCL_Pin */
  GPIO_InitStruct.Pin = OLED_SDA_Pin|OLED_SCL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        static uint8_t rx_buf[8];
        static uint8_t rx_cnt = 0;

        rx_buf[rx_cnt++] = us_rx[0];

        // 判断帧头
        if (rx_cnt == 1) {
            if (rx_buf[0] != 0xFF) {
                rx_cnt = 0;  // 非法帧头，清空
            }
        }

        // 距离帧（4字节：FF Data_H Data_L SUM）
        if (rx_buf[0] == 0xFF && rx_cnt >= 4) {
            uint8_t sum = (uint8_t)(rx_buf[0] + rx_buf[1] + rx_buf[2]);
            if (sum == rx_buf[3]) {
                distance_mm = ((uint16_t)rx_buf[1] << 8) | rx_buf[2];
                distance_ready = 1;
            }
            rx_cnt = 0;
        }

        // 防止溢出
        if (rx_cnt >= 8) rx_cnt = 0;

        HAL_UART_Receive_IT(&huart3, &us_rx[0], 1);
    }

    if (huart->Instance == USART1)
    {
        char debug[32];
        sprintf(debug, "RX: %c\r\n", RxData);
        PC_SendText(debug);

        if (RxData == '1') // 识别到垃圾
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET); // 绿灯亮
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);   // 红灯灭
        }
        else if (RxData == '0') // 没识别到垃圾
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);   // 绿灯灭
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET); // 红灯亮
        }

        HAL_UART_Receive_IT(&huart1, &RxData, 1);
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while (1)
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_Delay(500);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */