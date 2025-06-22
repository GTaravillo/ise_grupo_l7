#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"     
#include "adc.h"
#include "stdio.h"
#define RESOLUTION_12B 4096U
#define VREF 3.3f

/**
  * @brief config the use of analog inputs ADC123_IN10 and ADC123_IN13 and enable ADC1 clock
  * @param None
  * @retval None
  */
  
 osThreadId_t tid_ThADC;
 
 void ThADC (void *argument);
  
  osMessageQueueId_t   queueADC;
   
  
void ADC1_pins_F429ZI_config(){
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	/*PC0     ------> ADC1_IN10
    PC3     ------> ADC1_IN13
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  }

int Init_ADC (void) {
 
   //Inicializaci�n potenciometros
   
	ADC1_pins_F429ZI_config();
   
   //Crear Queue
   
   queueADC = osMessageQueueNew(QUEUE_ADC_SIZE, sizeof(MSG_POT), NULL);
   
  tid_ThADC = osThreadNew(ThADC, NULL, NULL);
  if (tid_ThADC == NULL) {
    return(-1);
  }
 
  return(0);
}
/**
  * @brief Initialize the ADC to work with single conversions. 12 bits resolution, software start, 1 conversion
  * @param ADC handle
	* @param ADC instance
  * @retval HAL_StatusTypeDef HAL_ADC_Init
  */

int ADC_Init_Single_Conversion(ADC_HandleTypeDef *hadc, ADC_TypeDef  *ADC_Instance)
{
   /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc->Instance = ADC_Instance;
  hadc->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc->Init.Resolution = ADC_RESOLUTION_12B;
  hadc->Init.ScanConvMode = DISABLE;
  hadc->Init.ContinuousConvMode = DISABLE;
  hadc->Init.DiscontinuousConvMode = DISABLE;
  hadc->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc->Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc->Init.NbrOfConversion = 1;
  hadc->Init.DMAContinuousRequests = DISABLE;
  hadc->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(hadc) != HAL_OK)
  {
    return -1;
  }
 return 0;
}
 
/**
  * @brief Configure a specific channels ang gets the voltage in float type. This funtion calls to  HAL_ADC_PollForConversion that needs HAL_GetTick()
  * @param ADC_HandleTypeDef
	* @param channel number
	* @retval voltage in float (resolution 12 bits and VRFE 3.3
  */

uint32_t ADC_getVoltage(ADC_HandleTypeDef *hadc, uint32_t Channel)
{
ADC_ChannelConfTypeDef sConfig = {0};
HAL_StatusTypeDef status;

uint32_t raw = 0;
static float voltage = 0;
 /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = Channel;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
  {
    return -1;
  }

HAL_ADC_Start(hadc);

do {
  status = HAL_ADC_PollForConversion(hadc, 0);
}  //This funtions uses the HAL_GetTick(), then it only can be executed wehn the OS is running
while(status != HAL_OK);

  raw = HAL_ADC_GetValue(hadc);

  //voltage = raw*VREF/RESOLUTION_12B; 

  return raw;

}
	// Example of using this code from a Thread 
void ThADC (void *argument) {
  ADC_HandleTypeDef adchandle; //handler definition
	//ADC1_pins_F429ZI_config(); //specific PINS configuration
	float value_consumo;
	float value_ruido;
	MSG_POT adc_queue_msg;
	
	
	
	ADC_Init_Single_Conversion(&adchandle , ADC1); //ADC1 configuration
  while (1) {
    //osThreadFlagsWait(0x01,osFlagsWaitAny,osWaitForever);
		
	  value_ruido=ADC_getVoltage(&adchandle , 10 ); //get values from channel 10->ADC123_IN10
		value_consumo=ADC_getVoltage(&adchandle , 13 );
		
		value_ruido = (((3.3*value_ruido)/4096)-2) ;
		value_consumo = ((3.3*value_consumo)/4096)/10;
    
		adc_queue_msg.ruido = (uint32_t)(value_ruido);
		adc_queue_msg.consumo =(uint32_t)(value_consumo);
    
    printf("Valor de ruido medido: %f, valor de consumo leido : %f mA \n",value_ruido,value_consumo);
    
    osDelay(1000);
		
		//osMessageQueuePut(queueADC,&adc_queue_msg,NULL,100);
		
	  osThreadYield();      
	
  }
}

