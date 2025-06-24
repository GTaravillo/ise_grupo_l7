#include "BajoConsumo.h"

#include "stm32f4xx_hal.h"

#include <stdio.h>

#include "../config/Paths.h"
#include PATH_ADC
#include PATH_COM_PLACAS
#include PATH_DISTANCIA
#include PATH_NFC

osThreadId_t e_bajoConsumoThreadId;

void BajoConsumoInitialize(void);
static void Run(void *argument);
static void GpioInitialize(void);
static void EntrarModoBajoConsumo(void);
static void SalirModoBajoConsumo(void);

static void SystemClock_Config(void);

void BajoConsumoInitialize(void)
{
  e_bajoConsumoThreadId = osThreadNew(Run, NULL, NULL);
  // e_writeSemaphoreId = osSemaphoreNew(1, 1, NULL);

  if (e_bajoConsumoThreadId == NULL) 
  {
   // printf"[bajo_consumo::%s] ERROR! osThreadNew [%d]\n", __func__, (e_bajoConsumoThreadId == NULL));
  }
}

static void Run(void *argument)
{
    HAL_Init();
    SystemClock_Config();
    GpioInitialize();

    while (1)
    {
     // printf"[bajo_consumo::%s] Espero flag bajo consumo\n", __func__);
      uint32_t flags = osThreadFlagsWait(FLAGS_BAJO_CONSUMO, osFlagsWaitAny, osWaitForever);
      
      if (flags == ENTRADA_BAJO_CONSUMO)
      {
        EntrarModoBajoConsumo();
      }
      else if (flags == SALIDA_BAJO_CONSUMO)
      {
        SalirModoBajoConsumo();
      }
    }
}

static void GpioInitialize(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // PA0 as EXTI input (wake-up trigger)
    GPIO_InitStruct.Pin  = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Enable EXTI0 interrupt
    HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

static void EntrarModoBajoConsumo(void)
{
 // printf"[bajo_consumo::%s] ENTRO EN BAJO CONSUMO\n", __func__);
  HAL_SuspendTick();  // Stop SysTick for max power saving

  // Enter STOP Mode with low power regulator ON, wait for interrupt (WFI)
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}

static void SalirModoBajoConsumo(void)
{
  HAL_ResumeTick();   // Restart SysTick

  // MUST reconfigure clock after waking from STOP mode
  SystemClock_Config();
 // printf"[bajo_consumo::%s] SALGO DE BAJO CONSUMO\n", __func__);

}

void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_0) {
        GPIO_PinState state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
        if (state == GPIO_PIN_RESET) {
          osThreadFlagsSet(e_bajoConsumoThreadId, ENTRADA_BAJO_CONSUMO);
        } else {
          osThreadFlagsSet(e_bajoConsumoThreadId, SALIDA_BAJO_CONSUMO);
        }
    }
}

static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState 	     = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM       = 8;
  RCC_OscInitStruct.PLL.PLLN       = 336;
  RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ       = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
   // printf"[main::%s] HAL_RCC_OscConfig ERROR!\n", __func__);
    

  }
	
	if(HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    /* Initialization Error */
   // printf"[main::%s] HAL_PWREx_EnableOverDrive ERROR!\n", __func__);
    

  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType      = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
   // printf"[main::%s] HAL_RCC_ClockConfig ERROR!\n", __func__);
    

  }
}