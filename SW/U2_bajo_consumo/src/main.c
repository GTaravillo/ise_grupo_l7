#include "main.h"
/* ARM */
/* std */
#include <stdio.h>
/* Interfaces */
#include "config/Paths.h"
#include "com_placas/ComunicacionPlacas.h"  //PATH_COM_PLACAS
#include "irq/stm32f4xx_it.h" //PATH_IRQ
#include "distancia/VL6180X/thDistancia.h" //
#include "nfc/rc522.h"
#include "adc/adc.h"

#include "bajo_consumo/BajoConsumo.h"

#ifdef RTE_CMSIS_RTOS2_RTX5
/**
  * Override default HAL_GetTick function
  */
uint32_t HAL_GetTick (void) {
  static uint32_t ticks = 0U;
         uint32_t i;

  if (osKernelGetState () == osKernelRunning) {
    return ((uint32_t)osKernelGetTickCount ());
  }

  /* If Kernel is not running wait approximately 1 ms then increment 
     and return auxiliary tick counter value */
  for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  }
  return ++ticks;
}

/**
  * Override default HAL_InitTick function
  */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) {
  
  UNUSED(TickPriority);

  return HAL_OK;
}

#endif

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/*	System Core Clock
	HSE_VALUE = 8MHz
	PLLM              =  8    [0,63]      ->  8MHz/8    = 1MHz		
	PLLN              =  336  [50,432]    ->  1MHz*336  = 336MHz
	PLLP              =  RCC_PLLP_DIV2    ->  336MHz/2  = 168MHz  = SystemCoreClock
	AHBCLKDivider     =  RCC_SYSCLK_DIV1  ->  168MHz
    APB1CLKDivider  =  RCC_HCLK_DIV4    ->  168MHz/4  = 42MHz   = APB1 peripheral clocks (max 45MHz)
                                            42MHz*2   = 84MHz   = APB1 Timer clocks
    APB2CLKDivider  =  RCC_HCLK_DIV2    ->  168MHz/2  = 84MHz   = APB2 peripheral clocks (max 90MHz)
                                            84MHz*2   = 168MHz  = APB2 Timer clocks
*/
#define PLLM_DIV8     8
#define PLLN_MULT336  336
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, Flash preread and Buffer caches
       - Systick timer is configured by default as source of time base, but user 
             can eventually implement his proper time base source (a general purpose 
             timer for example or other time source), keeping in mind that Time base 
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
             handled in milliseconds basis.
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();

  /* Add your application code here
     */
	
  

#ifdef RTE_CMSIS_RTOS2
  /* Initialize CMSIS-RTOS2 */
  osKernelInitialize ();
	

  /* Create thread functions that start executing*/
	ComunicacionPlacasInitialize();
  Init_Thread_NFC();
  ThDistancia();
  //ThSimNfc();
  Init_ADC();
	//BajoConsumoInitialize();
  
  /* Start thread execution */
  osKernelStart();
#endif

  /* Infinite loop */
  while (1)
  {
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
  RCC_OscInitStruct.HSEState 			 = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState 	 = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM 			 = PLLM_DIV8;
  RCC_OscInitStruct.PLL.PLLN 			 = PLLN_MULT336;
  RCC_OscInitStruct.PLL.PLLP 			 = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ 			 = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    printf("[main::%s] HAL_RCC_OscConfig ERROR!\n", __func__);
    
    Error_Handler();
  }
	
	if(HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    /* Initialization Error */
    printf("[main::%s] HAL_PWREx_EnableOverDrive ERROR!\n", __func__);
    
    Error_Handler();
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
    printf("[main::%s] HAL_RCC_ClockConfig ERROR!\n", __func__);
    
    Error_Handler();
  }

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
