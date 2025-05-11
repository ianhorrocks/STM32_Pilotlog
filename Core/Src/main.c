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
#include <stdlib.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    STATE_IDLE,
	STATE_WAIT_FOR_SECOND_READ
} State;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
FATFS fs; /* File system object structure (FATFS) - para montar la sd*/
FIL fil; /* File object structure (FIL) - Para crear archivos de texto*/
ds1307_dev_t my_rtc;
uint8_t UID[8], TagType;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void DisplayTimeOnly(void);
void DrawIdleStatic(void);
void DisplayScan(const char* timestamp, const char* usuario);
void UpdateCSVDateTimeString(char* out);
void generateUniqueId(ds1307_dev_t* rtc, char* out);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void DisplayTimeOnly(void) {
    ds1307_update(&my_rtc);
    char date_str[20], time_str[20], dt[30];
    sprintf(date_str, "%02d/%02d/%02d",
            my_rtc.date, my_rtc.month, my_rtc.year % 100);
    sprintf(time_str, "%02d:%02d:%02d",
            my_rtc.hours, my_rtc.minutes, my_rtc.seconds);
    sprintf(dt, "%s %s", date_str, time_str);

    SSD1306_GotoXY(5, 1);
    SSD1306_Puts("                    ", &Font_7x10, BLACK);
    SSD1306_GotoXY(5, 1);
    SSD1306_Puts((char*)dt, &Font_7x10, WHITE);  // cast para evitar warning
    SSD1306_UpdateScreen();
}

void DrawIdleStatic(void) {
    SSD1306_Clear();
    DisplayTimeOnly();
    SSD1306_GotoXY(0, 25);
    SSD1306_Puts("ESCANEAR", &Font_11x18, WHITE);
    SSD1306_UpdateScreen();
}

// --- Helper estático: pinta Usuario en medio y Fecha/Hora abajo ---
static void DrawUserAndDateBelow(const char* timestamp, const char* usuario) {
    int sfw = Font_7x10.FontWidth;
    int sfh = Font_7x10.FontHeight;
    int lfw = Font_11x18.FontWidth;
    int lfh = Font_11x18.FontHeight;

    // 1) Usuario centrado en Y ≈ 28
    char usr_line[32];
    snprintf(usr_line, sizeof(usr_line), "Usuario:%s", usuario);
    int wus = strlen(usr_line) * sfw;
    int xus = (SSD1306_WIDTH - wus) / 2;
    SSD1306_GotoXY(xus, (SSD1306_HEIGHT - sfh) / 2);
    SSD1306_Puts(usr_line, &Font_7x10, WHITE);

    // 2) Fecha+Hora formateada y centrada en Y = 64 - lfh - 2
    char dt_disp[16];
    snprintf(dt_disp, sizeof(dt_disp), "%c%c/%c%c/%c%c %c%c:%c%c",
             timestamp[8], timestamp[9],
             timestamp[5], timestamp[6],
             timestamp[2], timestamp[3],
             timestamp[11], timestamp[12],
             timestamp[14], timestamp[15]);
    int wdt = strlen(dt_disp) * lfw;
    int xdt = (SSD1306_WIDTH - wdt) / 2;
    int ydt = SSD1306_HEIGHT - lfh - 2;
    SSD1306_GotoXY(xdt, ydt);
    SSD1306_Puts(dt_disp, &Font_11x18, WHITE);
}

// --- Función principal con restauración ---
void DisplayScan(const char* timestamp, const char* usuario) {
    // 1) Limpiar toda la pantalla
    SSD1306_Clear();

    // 2) Pintar la parte fija (usuario + fecha abajo)
    DrawUserAndDateBelow(timestamp, usuario);
    SSD1306_UpdateScreen();

    // 3) Parámetros de fuente pequeña
    int sfw = Font_7x10.FontWidth;
    int sfh = Font_7x10.FontHeight;

    // 4) Animación arriba: "Lectura OK" sobre negro
    const char* m1 = "Lectura OK";
    int x1 = (SSD1306_WIDTH - strlen(m1) * sfw) / 2;
    SSD1306_DrawFilledRectangle(0, 0, SSD1306_WIDTH, sfh, BLACK);
    SSD1306_GotoXY(x1, 0);
    SSD1306_Puts((char*)m1, &Font_7x10, WHITE);
    SSD1306_UpdateScreen();
    HAL_Delay(1200);

    // 5) Animación arriba: "Guardando..." sobre blanco
    const char* m2 = "Guardando...";
    int x2 = (SSD1306_WIDTH - strlen(m2) * sfw) / 2;
    SSD1306_DrawFilledRectangle(0, 0, SSD1306_WIDTH, sfh, WHITE);
    SSD1306_GotoXY(x2, 0);
    SSD1306_Puts((char*)m2, &Font_7x10, BLACK);
    SSD1306_UpdateScreen();
    HAL_Delay(1200);

    // 6) Restaurar la franja de arriba (fondo negro) y volver a pintar la parte fija
    SSD1306_DrawFilledRectangle(0, 0, SSD1306_WIDTH, sfh, BLACK);
    DrawUserAndDateBelow(timestamp, usuario);
    SSD1306_UpdateScreen();
}


void UpdateCSVDateTimeString(char* out) {
    ds1307_update(&my_rtc);
    sprintf(out, "20%02d-%02d-%02d %02d:%02d:%02d",
            my_rtc.year % 100,
            my_rtc.month,
            my_rtc.date,
            my_rtc.hours,
            my_rtc.minutes,
            my_rtc.seconds);
}

void generateUniqueId(ds1307_dev_t* rtc, char* out) {
    ds1307_update(rtc);
    uint16_t r = rand() % 1000;
    sprintf(out, "20%02d%02d%02d%02d%02d%02d%03d",
        rtc->year % 100,
        rtc->month,
        rtc->date,
        rtc->hours,
        rtc->minutes,
        rtc->seconds,
        r
    );
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


  //ds1307_config(30, 44, 12, Mar, 06, Mayo, 2025, +3, 00);

  // Montar el sistema de archivos
  f_mount(&fs, "", 0);
  if (f_open(&fil, "data.csv", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK) {
      f_close(&fil);
  }

  bool idle_drawn = false;
  char usuario[9];
  TimeRecord record;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
      {
          if (!idle_drawn) {
              DrawIdleStatic();
              idle_drawn = true;
          } else {
              DisplayTimeOnly();
          }

          if (MFRC522_IsCard(&TagType) && MFRC522_ReadCardSerial(UID)) {
              HAL_Delay(200);

              generateUniqueId(&my_rtc, record.id);
              strcpy(record.id_embebed, "AA4K0GH8"); // ID EMBEBED DE FABRICA
              sprintf(record.id_tag, "%02X%02X%02X%02X",
                      UID[0], UID[1], UID[2], UID[3]);
              UpdateCSVDateTimeString(record.timestamp);

              sprintf(usuario, "%02X%02X%02X%02X",
                      UID[0], UID[1], UID[2], UID[3]);
              DisplayScan(record.timestamp, usuario);

              if (f_open(&fil, "data.csv", FA_OPEN_ALWAYS | FA_WRITE) == FR_OK) {
                  writeCSVRecord(&fil, &record);
                  f_close(&fil);
              }

              idle_drawn = false;
              SSD1306_Clear();
              HAL_Delay(200);
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
