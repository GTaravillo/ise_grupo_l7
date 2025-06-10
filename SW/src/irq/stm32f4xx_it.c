/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "stm32f4xx_hal.h"
#include "../rtc/rtc.h"
#include "../led/led.h"
#include "../posicion/PositionManager.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

// IRQ pines para Interrupciones (INT) de los expansores
void EXTI15_10_IRQHandler(void) 
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);	// PF12 - Exp 2  
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);	// PF14 - Exp 3  
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);	// PF15 - Exp 4 
}

void EXTI9_5_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);  // PC9 - Exp 1
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_9)
	{
		osThreadFlagsSet(e_positionManagerThreadId, HALL_DETECTED_1);
	} 	
	else if (GPIO_Pin == GPIO_PIN_12)
	{
		osThreadFlagsSet(e_positionManagerThreadId, HALL_DETECTED_2);
	}	
	else if (GPIO_Pin == GPIO_PIN_14)
	{
		osThreadFlagsSet(e_positionManagerThreadId, HALL_DETECTED_3);
	}	
	else if (GPIO_Pin == GPIO_PIN_15)
	{
		osThreadFlagsSet(e_positionManagerThreadId, HALL_DETECTED_4);
	}
}
