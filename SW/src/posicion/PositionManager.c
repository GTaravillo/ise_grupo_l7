/*
Velocidad?

*/

#include "PositionManager.h"
/* ARM */
#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
/* std */
#include <stdio.h>
#include <stdbool.h>
/* Interfaces */
#include "../config/Paths.h"
#include PATH_LED

#define POWER_ON_RESET  150     // Tiempo (ms) que debe mantenerse en reset al inicializar
#define SLAVE_1_ADDR    0x40
#define SLAVE_2_ADDR    0x48

/* Driver I2C */
extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;

osThreadId_t e_positionManagerThreadId;

/* Private */
static  void  Run(void *argument);

void  I2C_SignalEvent(uint32_t event);

/**************************************/

void PositionManagerInitialize(void)
{
  e_positionManagerThreadId = osThreadNew(Run, NULL, NULL);

  if ((e_positionManagerThreadId == NULL))
  {
    printf("[position::%s] ERROR! osThreadNew [%d]\n", __func__, (e_positionManagerThreadId == NULL));
  }
}

static void Run(void *argument)
{
	int32_t status;
  printf("[posicion::Run] Initializing I2C\n");
  I2Cdrv->Initialize(I2C_SignalEvent);
  I2Cdrv->PowerControl (ARM_POWER_FULL);
  I2Cdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
  I2Cdrv->Control(ARM_I2C_BUS_CLEAR, NULL);
  osDelay(1000);

  ledMessage_t ledMsg = 
  {
    .mode = LED_ON,
    .ledsOn.leds = LD1
  };
  osStatus_t osStatus = osMessageQueuePut(e_ledInputMessageId, &ledMsg, 1, 0);
  
  uint8_t buff[2];

  uint8_t writeBuffer[2] = {0xFF, 0xFF};
  status = I2Cdrv->MasterTransmit(SLAVE_1_ADDR, writeBuffer, 2, false);
	printf("MasterTransmit [%d]\n", status);
  osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
  osDelay(1000);

  while (1)
  {
		buff[0] = 0x00;
		buff[1] = 0x00;
		
    status = I2Cdrv->MasterReceive(SLAVE_1_ADDR, buff, 2, false);
		printf("MasterReceive [%d]\n", status);
    // Wait for the transfer to complete
    osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
  
    uint16_t gpioState = (buff[0] << 8) | buff[1];
		printf("Estado PC8 [%d]\n", HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8));
    printf("LECTURA [%d]\n", gpioState);

    osDelay(1000);
  }

}

void I2C_SignalEvent(uint32_t event) 
{
	printf("I2C_SignalEvent [%#x]\n", event);
  if (event & ARM_I2C_EVENT_TRANSFER_DONE) 
  {
    /* Transfer or receive is finished */
		osThreadFlagsSet(e_positionManagerThreadId, ARM_I2C_EVENT_TRANSFER_DONE);
  }
}
