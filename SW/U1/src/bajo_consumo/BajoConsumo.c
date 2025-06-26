#include "BajoConsumo.h"

#include "stm32f4xx_hal.h"

#include <stdio.h>

osThreadId_t e_bajoConsumoThreadId;

void BajoConsumoInitialize(void);

static void Run(void *argument);
void GpioInit(void);
static void StopOtheBoard(void);
static void WakeUpOtherBoard(void);

void BajoConsumoInitialize(void)
{
  e_bajoConsumoThreadId = osThreadNew(Run, NULL, NULL);
	
	const int threadError = e_bajoConsumoThreadId == NULL;
	
	if (threadError)
	{
		printf("[bajo_consumo::%s] Thread error!\n", __func__);
	}
		
	
}

void Run(void *argument)
{
    GpioInit();
	  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);


    while (1)
    {
      printf("[bajo_consumo::%s] Espero flag bajo consumo\n", __func__);
      uint32_t flags = osThreadFlagsWait(FLAGS_BAJO_CONSUMO, osFlagsWaitAny, osWaitForever);
			printf("[bajo_consumo::%s] El flag que llega: [0x%02X]\n", __func__, flags);
      if (flags == ENTRADA_BAJO_CONSUMO)
      {
        StopOtheBoard();
      }
      else if (flags == SALIDA_BAJO_CONSUMO)
      {
        WakeUpOtherBoard();
      }
    }
}

void GpioInit(void)
{
    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // PD15 as push-pull output
    GPIO_InitStruct.Pin   = GPIO_PIN_15;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

static void WakeUpOtherBoard(void)
{
  printf("[bajo_consumo::%s] Mando interrupcion DESPERTAR\n", __func__);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
}

static void StopOtheBoard(void)
{
  printf("[bajo_consumo::%s] Mando interrupcion DORMIR\n", __func__);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
}