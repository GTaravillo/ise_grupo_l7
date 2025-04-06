#include "stm32f4xx_hal.h"
#include "adc.h"
#include <stdio.h>

#define ADC_RESOLUTION        12        /* Number of A/D converter bits       */
#define VREF 									3.3f


static ADC_HandleTypeDef hadc1;
static volatile uint8_t  AD_done;       /* AD conversion done flag            */


/**
  \fn          int32_t ADC_Initialize (void)
  \brief       Initialize Analog-to-Digital Converter
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t ADC_Initialize (void) {
  ADC_ChannelConfTypeDef sConfig;
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /* ADC1 GPIO Configuration: PC0 -> ADC1_IN10 */
  GPIO_InitStruct.Pin 	= GPIO_PIN_0;
  GPIO_InitStruct.Mode 	= GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull 	= GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	/* ADC1 GPIO Configuration: PA3 -> ADC1_IN3 */
  GPIO_InitStruct.Pin 	= GPIO_PIN_3;
  GPIO_InitStruct.Mode 	= GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull 	= GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Peripheral clock enable */
  __HAL_RCC_ADC1_CLK_ENABLE();

  /* Configure the global features of the ADC
    (Clock, Resolution, Data Alignment and number of conversion) */
  hadc1.Instance 										= ADC1;
  hadc1.Init.ClockPrescaler 				= ADC_CLOCKPRESCALER_PCLK_DIV2;
  hadc1.Init.Resolution 						= ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode 					= DISABLE;
  hadc1.Init.ContinuousConvMode 		= DISABLE;
  hadc1.Init.DiscontinuousConvMode 	= DISABLE;
  hadc1.Init.NbrOfDiscConversion 		= 1;
  hadc1.Init.ExternalTrigConvEdge 	= ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DataAlign 							= ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion 				= 1;
  hadc1.Init.DMAContinuousRequests 	= DISABLE;
  hadc1.Init.EOCSelection 					= ADC_EOC_SINGLE_CONV;
  HAL_ADC_Init(&hadc1);

  /* Configure the selected ADC channel */
  sConfig.Channel 			= ADC_CHANNEL_3;
  sConfig.Rank 					= 1;
  sConfig.SamplingTime 	= ADC_SAMPLETIME_3CYCLES;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

  AD_done = 0;

  return 0;
}

/**
  \fn          int32_t ADC_Uninitialize (void)
  \brief       De-initialize Analog-to-Digital Converter
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t ADC_Uninitialize (void) {

  /* Peripheral clock disable */
  __HAL_RCC_ADC1_CLK_DISABLE();
	
  /* ADC1 GPIO Configuration: PC0 -> ADC1_IN10 */
  HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0);
	
	/* ADC1 GPIO Configuration: PA3 -> ADC1_IN3 */
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3);

  return 0;
}

/**
  \fn          int32_t ADC_StartConversion (void)
  \brief       Start conversion
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t ADC_StartConversion (void) {
  __HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_EOC);
  HAL_ADC_Start(&hadc1);

  AD_done = 0;

  return 0;
}

/**
  \fn          int32_t ADC_ConversionDone (void)
  \brief       Check if conversion finished
  \returns
   - \b  0: conversion finished
   - \b -1: conversion in progress
*/
int32_t ADC_ConversionDone (void) {
  HAL_StatusTypeDef status;

  status = HAL_ADC_PollForConversion(&hadc1, 0);
  if (status == HAL_OK) {
    AD_done = 1;
    return 0;
  } else {
    AD_done = 0;
    return -1;
  }
}

/**
  \fn          int32_t ADC_GetValue (void)
  \brief       Get converted value
  \returns
   - <b> >=0</b>: converted value
   - \b -1: conversion in progress or failed
*/
int32_t ADC_GetValue (void) {
  HAL_StatusTypeDef status;
  int32_t val;

  if (AD_done == 0) {
    status = HAL_ADC_PollForConversion(&hadc1, 0);
    if (status != HAL_OK) return -1;
  } else {
    AD_done = 0;
  }

  val = (int32_t)HAL_ADC_GetValue(&hadc1);
	printf("[adc::%s]ADC val [%d]\n", __func__, val);

  return val;
}

/**
  \fn          uint32_t ADC_GetResolution (void)
  \brief       Get resolution of Analog-to-Digital Converter
  \returns     Resolution (in bits)
*/
uint32_t ADC_GetResolution (void) {
  return ADC_RESOLUTION;
}
