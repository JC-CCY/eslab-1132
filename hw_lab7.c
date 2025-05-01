/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "math_helper.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
DFSDM_Channel_HandleTypeDef hdfsdm1_channel1;

I2C_HandleTypeDef hi2c2;

QSPI_HandleTypeDef hqspi;

UART_HandleTypeDef huart1;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */
int __io_putchar(int ch){
	HAL_UART_Transmit(&huart1,(uint8_t *)&ch,1,0xFFFF);
	return ch;
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DFSDM1_Init(void);
static void MX_I2C2_Init(void);
static void MX_QUADSPI_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
/* USER CODE BEGIN PFP */

/* FIR filter parameters */
#define BLOCK_SIZE  32
#define NUM_TAPS    29
#define length_variable    30

#define SAMPLE_RATE   1000   // 取樣頻率 (Hz)
#define SIGNAL_LENGTH 2001   // 總共有1000筆資料 (1秒)
#define FILTER_TAP_NUM 29    // FIR濾波器tap數 (濾波器長度)

// 預先準備好buffer
float32_t mixedSignal[SIGNAL_LENGTH];
float32_t filteredSignal[SIGNAL_LENGTH];

// FIR濾波器用
float32_t firStateF32[FILTER_TAP_NUM + SIGNAL_LENGTH - 1];
arm_fir_instance_f32 S;

// FIR濾波器係數 (直接從MATLAB產生，這裡是29 tap低通)
const float32_t firCoeffs32[FILTER_TAP_NUM] = {
    -0.0015, -0.0027, -0.0040, -0.0052, -0.0060,
    -0.0059, -0.0045, -0.0013,  0.0042,  0.0118,
     0.0212,  0.0318,  0.0427,  0.0527,  0.0607,
     0.0659,  0.0680,  0.0668,  0.0626,  0.0560,
     0.0480,  0.0396,  0.0318,  0.0252,  0.0199,
     0.0159,  0.0129,  0.0105,  0.0085
};

const float32_t mixed_signal[2001] = {
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f, -0.0123f,  1.9511f, -0.1090f,  1.8090f, -0.2929f,  1.5878f, -0.5460f,  1.3090f, -0.8436f,  1.0000f, -1.1564f,  0.6910f, -1.4540f,  0.4122f, -1.7071f,  0.1910f, -1.8910f,  0.0489f, -1.9877f,
     0.0000f, -1.9877f,  0.0489f, -1.8910f,  0.1910f, -1.7071f,  0.4122f, -1.4540f,  0.6910f, -1.1564f,  1.0000f, -0.8436f,  1.3090f, -0.5460f,  1.5878f, -0.2929f,  1.8090f, -0.1090f,  1.9511f, -0.0123f,
     2.0000f
};
arm_fir_instance_f32 S;

/* Buffers */
float32_t inputBuffer[BLOCK_SIZE];
float32_t outputBuffer[BLOCK_SIZE];
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
  MX_DFSDM1_Init();
  MX_I2C2_Init();
  MX_QUADSPI_Init();
  MX_USART1_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  /* USER CODE BEGIN 2 */
  BSP_ACCELERO_Init();
  printf("Starting FIR filter test...\r\n");

  // 初始化FIR濾波器
   arm_fir_init_f32(&S, FILTER_TAP_NUM, (float32_t *)firCoeffs32, firStateF32, SIGNAL_LENGTH);

   float32_t input[SIGNAL_LENGTH];
   float32_t output[SIGNAL_LENGTH];

   // 開始逐筆做FIR濾波
   for (int i = 0; i < SIGNAL_LENGTH; i++){
      input[i] = mixed_signal[i];
   }

   arm_fir_f32(&S, &input, &output, SIGNAL_LENGTH);

   // 印出來看
   for (int i = 0; i < SIGNAL_LENGTH; i++){
	   printf("Raw: %.4f\tFiltered: %.4f\r\n", mixed_signal[i], output[i]);
   }

    /* Real data processing */
    while (1)
    {
      int16_t accData[3];
      float32_t accX[length_variable], filteredX[length_variable];
      float32_t accY[length_variable], filteredY[length_variable];
      float32_t accZ[length_variable], filteredZ[length_variable];

      for(int i=0;i<length_variable;i++){
    	  BSP_ACCELERO_AccGetXYZ(accData);
    	  accX[i] = (float32_t)accData[0];
    	  accY[i] = (float32_t)accData[1];
    	  accZ[i] = (float32_t)accData[2];
    	  HAL_Delay(100);
      }
//      BSP_ACCELERO_AccGetXYZ(accData);
//      accX = (float32_t)accData[0];
//      accY = (float32_t)accData[1];
//      accZ = (float32_t)accData[2];

      arm_fir_f32(&S, &accX, &filteredX, length_variable);
      arm_fir_f32(&S, &accY, &filteredY, length_variable);
      arm_fir_f32(&S, &accZ, &filteredZ, length_variable);

      for(int i=0;i<length_variable;i++){
    	  printf("Raw X: %.2f\tFiltered X: %.2f\t", accX[i], filteredX[i]);
    	  printf("Raw Y: %.2f\tFiltered Y: %.2f\t", accY[i], filteredY[i]);
    	  printf("Raw Z: %.2f\tFiltered Z: %.2f\r\n", accZ[i], filteredZ[i]);
      }

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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief DFSDM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DFSDM1_Init(void)
{

  /* USER CODE BEGIN DFSDM1_Init 0 */

  /* USER CODE END DFSDM1_Init 0 */

  /* USER CODE BEGIN DFSDM1_Init 1 */

  /* USER CODE END DFSDM1_Init 1 */
  hdfsdm1_channel1.Instance = DFSDM1_Channel1;
  hdfsdm1_channel1.Init.OutputClock.Activation = ENABLE;
  hdfsdm1_channel1.Init.OutputClock.Selection = DFSDM_CHANNEL_OUTPUT_CLOCK_SYSTEM;
  hdfsdm1_channel1.Init.OutputClock.Divider = 2;
  hdfsdm1_channel1.Init.Input.Multiplexer = DFSDM_CHANNEL_EXTERNAL_INPUTS;
  hdfsdm1_channel1.Init.Input.DataPacking = DFSDM_CHANNEL_STANDARD_MODE;
  hdfsdm1_channel1.Init.Input.Pins = DFSDM_CHANNEL_FOLLOWING_CHANNEL_PINS;
  hdfsdm1_channel1.Init.SerialInterface.Type = DFSDM_CHANNEL_SPI_RISING;
  hdfsdm1_channel1.Init.SerialInterface.SpiClock = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
  hdfsdm1_channel1.Init.Awd.FilterOrder = DFSDM_CHANNEL_FASTSINC_ORDER;
  hdfsdm1_channel1.Init.Awd.Oversampling = 1;
  hdfsdm1_channel1.Init.Offset = 0;
  hdfsdm1_channel1.Init.RightBitShift = 0x00;
  if (HAL_DFSDM_ChannelInit(&hdfsdm1_channel1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DFSDM1_Init 2 */

  /* USER CODE END DFSDM1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00000E14;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief QUADSPI Initialization Function
  * @param None
  * @retval None
  */
static void MX_QUADSPI_Init(void)
{

  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /* USER CODE END QUADSPI_Init 1 */
  /* QUADSPI parameter configuration*/
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 2;
  hqspi.Init.FifoThreshold = 4;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
  hqspi.Init.FlashSize = 23;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */

  /* USER CODE END QUADSPI_Init 2 */

}


/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */


/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.battery_charging_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, M24SR64_Y_RF_DISABLE_Pin|M24SR64_Y_GPO_Pin|ISM43362_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, ARD_D10_Pin|SPBTLE_RF_RST_Pin|ARD_D9_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ARD_D8_Pin|ISM43362_BOOT0_Pin|ISM43362_WAKEUP_Pin|LED2_Pin
                          |SPSGRF_915_SDN_Pin|ARD_D5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, USB_OTG_FS_PWR_EN_Pin|PMOD_RESET_Pin|STSAFE_A100_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPBTLE_RF_SPI3_CSN_GPIO_Port, SPBTLE_RF_SPI3_CSN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, VL53L0X_XSHUT_Pin|LED3_WIFI__LED4_BLE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPSGRF_915_SPI3_CSN_GPIO_Port, SPSGRF_915_SPI3_CSN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ISM43362_SPI3_CSN_GPIO_Port, ISM43362_SPI3_CSN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : M24SR64_Y_RF_DISABLE_Pin M24SR64_Y_GPO_Pin ISM43362_RST_Pin ISM43362_SPI3_CSN_Pin */
  GPIO_InitStruct.Pin = M24SR64_Y_RF_DISABLE_Pin|M24SR64_Y_GPO_Pin|ISM43362_RST_Pin|ISM43362_SPI3_CSN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_OTG_FS_OVRCR_EXTI3_Pin SPSGRF_915_GPIO3_EXTI5_Pin SPBTLE_RF_IRQ_EXTI6_Pin ISM43362_DRDY_EXTI1_Pin */
  GPIO_InitStruct.Pin = USB_OTG_FS_OVRCR_EXTI3_Pin|SPSGRF_915_GPIO3_EXTI5_Pin|SPBTLE_RF_IRQ_EXTI6_Pin|ISM43362_DRDY_EXTI1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : BUTTON_EXTI13_Pin */
  GPIO_InitStruct.Pin = BUTTON_EXTI13_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUTTON_EXTI13_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARD_A5_Pin ARD_A4_Pin ARD_A3_Pin ARD_A2_Pin
                           ARD_A1_Pin ARD_A0_Pin */
  GPIO_InitStruct.Pin = ARD_A5_Pin|ARD_A4_Pin|ARD_A3_Pin|ARD_A2_Pin
                          |ARD_A1_Pin|ARD_A0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : ARD_D1_Pin ARD_D0_Pin */
  GPIO_InitStruct.Pin = ARD_D1_Pin|ARD_D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : ARD_D10_Pin SPBTLE_RF_RST_Pin ARD_D9_Pin */
  GPIO_InitStruct.Pin = ARD_D10_Pin|SPBTLE_RF_RST_Pin|ARD_D9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : ARD_D4_Pin */
  GPIO_InitStruct.Pin = ARD_D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
  HAL_GPIO_Init(ARD_D4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ARD_D7_Pin */
  GPIO_InitStruct.Pin = ARD_D7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ARD_D7_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARD_D13_Pin ARD_D12_Pin ARD_D11_Pin */
  GPIO_InitStruct.Pin = ARD_D13_Pin|ARD_D12_Pin|ARD_D11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : ARD_D3_Pin */
  GPIO_InitStruct.Pin = ARD_D3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ARD_D3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ARD_D6_Pin */
  GPIO_InitStruct.Pin = ARD_D6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ARD_D6_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARD_D8_Pin ISM43362_BOOT0_Pin ISM43362_WAKEUP_Pin LED2_Pin
                           SPSGRF_915_SDN_Pin ARD_D5_Pin SPSGRF_915_SPI3_CSN_Pin */
  GPIO_InitStruct.Pin = ARD_D8_Pin|ISM43362_BOOT0_Pin|ISM43362_WAKEUP_Pin|LED2_Pin
                          |SPSGRF_915_SDN_Pin|ARD_D5_Pin|SPSGRF_915_SPI3_CSN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : LPS22HB_INT_DRDY_EXTI0_Pin LSM6DSL_INT1_EXTI11_Pin ARD_D2_Pin HTS221_DRDY_EXTI15_Pin
                           PMOD_IRQ_EXTI12_Pin */
  GPIO_InitStruct.Pin = LPS22HB_INT_DRDY_EXTI0_Pin|LSM6DSL_INT1_EXTI11_Pin|ARD_D2_Pin|HTS221_DRDY_EXTI15_Pin
                          |PMOD_IRQ_EXTI12_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_OTG_FS_PWR_EN_Pin SPBTLE_RF_SPI3_CSN_Pin PMOD_RESET_Pin STSAFE_A100_RESET_Pin */
  GPIO_InitStruct.Pin = USB_OTG_FS_PWR_EN_Pin|SPBTLE_RF_SPI3_CSN_Pin|PMOD_RESET_Pin|STSAFE_A100_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : VL53L0X_XSHUT_Pin LED3_WIFI__LED4_BLE_Pin */
  GPIO_InitStruct.Pin = VL53L0X_XSHUT_Pin|LED3_WIFI__LED4_BLE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : VL53L0X_GPIO1_EXTI7_Pin LSM3MDL_DRDY_EXTI8_Pin */
  GPIO_InitStruct.Pin = VL53L0X_GPIO1_EXTI7_Pin|LSM3MDL_DRDY_EXTI8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PMOD_SPI2_SCK_Pin */
  GPIO_InitStruct.Pin = PMOD_SPI2_SCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(PMOD_SPI2_SCK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PMOD_UART2_CTS_Pin PMOD_UART2_RTS_Pin PMOD_UART2_TX_Pin PMOD_UART2_RX_Pin */
  GPIO_InitStruct.Pin = PMOD_UART2_CTS_Pin|PMOD_UART2_RTS_Pin|PMOD_UART2_TX_Pin|PMOD_UART2_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : ARD_D15_Pin ARD_D14_Pin */
  GPIO_InitStruct.Pin = ARD_D15_Pin|ARD_D14_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
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
