/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "fatfs.h"
#include "i2c.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mfrc522.h"
#include "fonts.h"
#include "ssd1306_oled.h"
#include "ds1307.h"
#include "csv_utils.h"

#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* Definición de estados */
typedef enum {
    STATE_IDLE,
	STATE_WAIT_FOR_SECOND_READ
} State;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
FATFS fs; /* File system object structure (FATFS) - para montar la sd*/
FIL fil; /* File object structure (FIL) - Para crear archivos de texto*/
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
State current_state = STATE_IDLE;
uint8_t UID[8];
uint8_t TagType;
char buf_tx[50];
//uint8_t existingUID[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}; // Sustituye esto por tu ID existente
char time_str[20];

uint32_t record_id = 1;

ds1307_dev_t my_rtc;
char tiempo_inicial[20];



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void DisplayTime(const char* time_str) {
    SSD1306_GotoXY(5, 1);
    SSD1306_Puts(time_str, &Font_7x10, WHITE);
    SSD1306_UpdateScreen();
}

void DisplayIdle() {
    ds1307_update(&my_rtc);
    sprintf(time_str, "%02d:%02d:%02d", my_rtc.hours, my_rtc.minutes, my_rtc.seconds);
    DisplayTime(time_str); // Muestra el tiempo en la pantalla OLED
    SSD1306_GotoXY(0, 25);
    SSD1306_Puts("ESCANEAR ", &Font_11x18, WHITE);
    SSD1306_UpdateScreen();
}

void DisplayUser(const uint8_t* UID, const char* tiempo_inicial) {
    SSD1306_Clear();
    SSD1306_GotoXY(5, 1);
    SSD1306_Puts("Tiempo Inicial:", &Font_7x10, WHITE);
    SSD1306_GotoXY(18, 15);
    SSD1306_Puts(tiempo_inicial, &Font_11x18, WHITE);
    SSD1306_GotoXY(25, 36);
    SSD1306_Puts("BUEN VUELO!", &Font_7x10, WHITE);
    SSD1306_GotoXY(5, 50);
    SSD1306_Puts("Usuario:", &Font_7x10, WHITE);
    sprintf(buf_tx, "%02X%02X%02X%02X", UID[0], UID[1], UID[2], UID[3]);
    SSD1306_GotoXY(65, 50);
    SSD1306_Puts(buf_tx, &Font_7x10, WHITE);
    SSD1306_UpdateScreen();
    HAL_Delay(3000);
    SSD1306_Clear();
}

void Display2Case(const char* tiempo_inicial) {
	 ds1307_update(&my_rtc);
	 sprintf(time_str, "%02d:%02d:%02d", my_rtc.hours, my_rtc.minutes, my_rtc.seconds);
	 DisplayTime(time_str);
	 SSD1306_GotoXY(5, 20);
	 SSD1306_Puts("Inicial:", &Font_7x10, WHITE);
	 SSD1306_GotoXY(65, 20);
	 SSD1306_Puts(tiempo_inicial, &Font_7x10, WHITE);
	 SSD1306_GotoXY(5, 35);
	 SSD1306_Puts("Final:", &Font_7x10, WHITE);
	 SSD1306_GotoXY(5, 50);
	 SSD1306_Puts("Usuario:", &Font_7x10, WHITE);
	 sprintf(buf_tx, "%02X%02X%02X%02X", UID[0], UID[1], UID[2], UID[3]);
	 SSD1306_GotoXY(65, 50);
	 SSD1306_Puts(buf_tx, &Font_7x10, WHITE);
	 SSD1306_UpdateScreen();
}

void Display2Case2(const char* tiempo_inicial, const char* tiempo_final) {
	 ds1307_update(&my_rtc);
	 sprintf(time_str, "%02d:%02d:%02d", my_rtc.hours, my_rtc.minutes, my_rtc.seconds);
	 DisplayTime(time_str);
	 SSD1306_GotoXY(5, 20);
	 SSD1306_Puts("Inicial:", &Font_7x10, WHITE);
	 SSD1306_GotoXY(65, 20);
	 SSD1306_Puts(tiempo_inicial, &Font_7x10, WHITE);
	 SSD1306_GotoXY(5, 35);
	 SSD1306_Puts("Final:", &Font_7x10, WHITE);
	 SSD1306_GotoXY(65, 35);
	 SSD1306_Puts(tiempo_final, &Font_7x10, WHITE);
	 SSD1306_UpdateScreen();
}

void DisplayMessage(const char* message) {
    SSD1306_Clear();
    SSD1306_GotoXY(5, 1);
    SSD1306_Puts("Tiempo de Vuelo:", &Font_7x10, WHITE);
    SSD1306_GotoXY(18, 15); // Posición del mensaje en la pantalla
    SSD1306_Puts(message, &Font_11x18, WHITE);
    SSD1306_UpdateScreen();
}

int CompareUID(const uint8_t* UID1, const uint8_t* UID2) {
    for (int i = 0; i < 8; i++) {
        if (UID1[i] != UID2[i]) {
            return 0; // Los IDs no coinciden
        }
    }
    return 1; // Los IDs coinciden
}

void CalculateTimeDifference(TimeRecord* record) {
    // Supon que record.tiempo_inicial y record.tiempo_final están en el formato "HH:MM:SS"
    // Puedes usar una función para extraer las horas, minutos y segundos de ambas cadenas
    int initial_hours, initial_minutes, initial_seconds;
    int final_hours, final_minutes, final_seconds;

    sscanf(record->tiempo_inicial, "%d:%d:%d", &initial_hours, &initial_minutes, &initial_seconds);
    sscanf(record->tiempo_final, "%d:%d:%d", &final_hours, &final_minutes, &final_seconds);

    // Calcula la diferencia
    int time_difference_hours = final_hours - initial_hours;
    int time_difference_minutes = final_minutes - initial_minutes;
    int time_difference_seconds = final_seconds - initial_seconds;

    // Asegúrate de que los minutos y segundos no sean negativos
    if (time_difference_seconds < 0) {
        time_difference_seconds += 60;
        time_difference_minutes -= 1;
    }
    if (time_difference_minutes < 0) {
        time_difference_minutes += 60;
        time_difference_hours -= 1;
    }

    // Formatea la diferencia en una cadena y guárdala en record.tiempo_total
    sprintf(record->tiempo_total, "%02d:%02d:%02d", time_difference_hours, time_difference_minutes, time_difference_seconds);
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
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_SPI2_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  MFRC522_Init();
  SSD1306_Init();
  ds1307_init();


  ds1307_config(20, 44, 23, SUNDAY, 5, JAN, 2025, +3, 00);

  // Montar el sistema de archivos
  f_mount(&fs, "", 0);
  TimeRecord record;
  record.id = record_id;
  record.id_aeronave = 1;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  switch (current_state) {
	      case STATE_IDLE:
	          DisplayIdle();
	          if (MFRC522_IsCard(&TagType)) {
	              if (MFRC522_ReadCardSerial((uint8_t*)&UID)) {
	            	  HAL_Delay(500);
	                  record.id_usuario = (UID[0] << 24) | (UID[1] << 16) | (UID[2] << 8) | UID[3];
	                  strcpy(record.tiempo_inicial, time_str);
	                  DisplayUser(UID, record.tiempo_inicial);
	                  current_state = STATE_WAIT_FOR_SECOND_READ;
	              }
	              MFRC522_Halt();
	          }
	          break;

	      case STATE_WAIT_FOR_SECOND_READ:
	    	  Display2Case(record.tiempo_inicial);
	    	  if (MFRC522_IsCard(&TagType)) {
	    	      if (MFRC522_ReadCardSerial((uint8_t*)&UID)) {
	    	    	  HAL_Delay(500);
	    	          strcpy(record.tiempo_final, time_str);
	    	          CalculateTimeDifference(&record);
	    	          Display2Case2(record.tiempo_inicial, record.tiempo_final);
	    	          HAL_Delay(3000);
	    	          DisplayMessage(record.tiempo_total);
	    	          HAL_Delay(5000);
	    	          SSD1306_Clear();
	    	           if (f_open(&fil, "data.csv", FA_OPEN_ALWAYS | FA_WRITE | FA_READ) == FR_OK) {
	    	               writeCSVRecord(&fil, &record);
	    	               f_close(&fil);
	    	            }
	    	          current_state = STATE_IDLE;
	    	       }
	    	       MFRC522_Halt();
	    	  }
	    	  break;
	  }

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
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

  /** Initializes the CPU, AHB and APB buses clocks
  */
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
