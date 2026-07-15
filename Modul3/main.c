/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* USER CODE BEGIN PV */
//-------- VARIABLE DECLARATION ---------
uint32_t t_awal;
uint32_t t_akhir;
uint32_t t_selisih;
volatile uint32_t sys_ticks= 0;
volatile uint32_t waktu_tekan_terakhir = 0;
uint32_t durasi_isr = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */


//void EXTI0_IRQHandler(void) {
//	if (EXTI->PR & (1 << 0)) {
//		 EXTI->PR |= (1 << 0); // Hapus flag interupsi
//		 GPIOC->ODR ^= (1 << 13); // Eksekusi aksi: Toggle LED PC13
//	}
//}

//void SysTick_Handler(void) {
//	sys_ticks++;
//}
// There is the same function in another file: stm32f4xx_it.c

// MITIGASI BOUNCING (EKSPERIMEN 7)
//void EXTI0_IRQHandler(void) {
//	 if (EXTI->PR & (1 << 0)) {
//		 EXTI->PR |= (1 << 0); // Hapus flag
//		 // Hanya proses penekanan jika selisih waktu > 50 ms dari penekanan terakhir
//		 if ((sys_ticks- waktu_tekan_terakhir) > 50) {
//			 GPIOC->ODR ^= (1 << 13);
//			 waktu_tekan_terakhir = sys_ticks; // Perbarui catatan waktu
//		 }
//	 }
//}

// EKSPERIMEN 8: Profiling Komparatif Beban CPU (PollingvsInterupsi)
void EXTI0_IRQHandler(void) {
//	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	 uint32_t waktu_mulai = DWT->CYCCNT; // Catat waktu saat CPU memasuki ISR
	 if (EXTI->PR & (1 << 0)) {
		 EXTI->PR |= (1 << 0);
		 GPIOC->ODR ^= (1 << 13);
	 }
	 durasi_isr = DWT->CYCCNT - waktu_mulai;
}

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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // EKSPERIMEN 1
  /* Mengaktifkan kapabilitas Trace pada ARM Core */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  /* Mengatur ulang nilai counter ke nol */
  DWT->CYCCNT = 0;
  /* Menyalakan pencacah siklus CYCCNT */
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  RCC->AHB1ENR |= (1 << 0);
  // Konfigurasi PA0 sebagai Input (Bit 0 dan 1 di MODER diset ke 00)
  GPIOA->MODER &= ~(3 << (0 * 2));
  // 1. Mengaktifkan clock AHB1 untuk GPIOC (Bit ke-2)
  RCC->AHB1ENR |= (1 << 2);
  // 2. Mengatur Mode PC13 menjadi General Purpose Output
  // Bersihkan bit 26 dan 27 terlebih dahulu (masking)
  GPIOC->MODER &= ~(3 << (13 * 2));
  // Atur bit ke-26 menjadi logika 1 (01 = Output Mode)
  GPIOC->MODER |= (1 << (13 * 2));

  //-------- EKSPERIMEN 5---------
  // 1. Aktifkan Clock untuk blok System Configuration (APB2 bit 14)
  RCC->APB2ENR |= (1 << 14);

  // 2. Hubungkan jalur EXTI0 ke Port A melalui register EXTICR1
  SYSCFG->EXTICR[0] &= ~(0xF << 0);

  // 3. Batalkan status Masking (aktifkan) jalur interrupt 0
  EXTI->IMR |= (1 << 0);

  // 4. Konfigurasikan interupsi terjadi saat Rising Edge (tombol ditekan)
  EXTI->RTSR |= (1 << 0);

  // 5. Daftarkan EXTI0 (IRQ Number 6) ke NVIC milik prosesor ARM
  NVIC_EnableIRQ(EXTI0_IRQn);

  while (1)
  {

//	  t_awal = DWT->CYCCNT; // Catat waktu mulai
//	  // Blok kode yang akan diuji kinerjanya
//	  HAL_Delay(1); // Uji jeda 1 milidetik standar HAL
//	  t_akhir = DWT->CYCCNT; // Catat waktu selesai
//	  t_selisih = t_akhir- t_awal;

	  //-------- EKSPERIMEN 2---------
//
//	  GPIOC->ODR ^= (1 << 13); // Toggle status pin PC13
//	  HAL_Delay(500);

	  //-------- EKSPERIMEN 3---------
//	  Delay_ms_Baremetal(1000); / Menggantikan HAL_Delay(500);

	  //-------- EKSPERIMEN 4---------
//	   Aktifkan clock AHB1 untuk GPIOA (Bit ke-0)

//
//	  // Baca status bit ke-0 dari register Input Data (IDR)
//	  if ((GPIOA->IDR & (1 << 0)) !=0){  // Tombol ditekan (Logika 1)
//		  GPIOC->ODR &= ~(1 << 13); // Nyalakan LED PC13 (Active Low)
//	  } else { // Tombol dilepas (Logika 0)
//		  GPIOC->ODR |= (1 << 13); // Padamkan LED PC13
//	  }



	  //-------- EKSPERIMEN 6--------- //FAILED?
//	  SysTick->LOAD = (84000- 1);
//	  SysTick->VAL = 0;
//	  SysTick->CTRL = (1 << 0) | (1 << 1) | (1 << 2);
//	  if (sys_ticks >= 500) {
//		  sys_ticks = 0; // Reset pencacah
//		  GPIOC->ODR ^= (1 << 13); // Toggle LED setiap 500ms
//	  }

	  //-------- EKSPERIMEN 7---------
//	  uncomment the EKSPERIMEN 5

	  //-------- EKSPERIMEN 8---------

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
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
