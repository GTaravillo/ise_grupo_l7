
#include "Memoria.h"
/* ARM */
#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
/* std */
#include <stdio.h>
#include <stdbool.h>
/* Interfaces */
#include "../config/Paths.h"

#define SLAVE_ADDR    0x50
#define PAGE_NUM			512
#define PAGE_SIZE			64


/* Driver I2C */
extern ARM_DRIVER_I2C Driver_I2C2;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C2;

osThreadId_t        e_memoriaThreadId;
osMessageQueueId_t  e_memoriaMessageId;  // Cola recepción datos

/* Private */
static void Run(void *argument);

static void SoftwareReset(void);

static void  I2C_SignalEvent(uint32_t event);

/**************************************/

void MemoriaInitialize(void)
{
  e_memoriaThreadId = osThreadNew(Run, NULL, NULL);

  if ((e_memoriaThreadId == NULL))
  {
    printf("[position::%s] ERROR! osThreadNew [%d]\n", __func__, (e_memoriaThreadId == NULL));
  }
}

static void Run(void *argument)
{
  int32_t status;
  printf("[memoria::Run] Initializing I2C\n");
  status = I2Cdrv->Initialize(I2C_SignalEvent);
  osDelay(1000);
  status = I2Cdrv->PowerControl (ARM_POWER_FULL);
  osDelay(1000);
  status = I2Cdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
  osDelay(1000);
  status = I2Cdrv->Control(ARM_I2C_BUS_CLEAR, NULL);
  osDelay(1000);
  
  //SoftwareReset();

  int pagina = 1;   // De 0 a 511 (1 1111 1111)
  int byte   = 0;   // De 0 a 63
  uint16_t direccion = (pagina * 64) + byte;  // Direccion del byte en concreto
  uint8_t direccion_dato[3] = 
  {
    (direccion & 0xFF00) >> 8,
    (direccion & 0x00FF), 
    0xA5
  };
  printf("[memoria::%s] direccion dato [0x%02X 0x%02X]\n", __func__, direccion_dato[0], direccion_dato[1]);

  /* Escribo en las direcciones 0x0000 a 0x0020 (32 bytes) */

  // Addr
  status = I2Cdrv->MasterTransmit(SLAVE_ADDR, direccion_dato, 3, false);
  osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
  osDelay(100);
//	// Escribo dato
//	uint8_t dato_escritura = 0xAA;
//	status = I2Cdrv->MasterTransmit(SLAVE_ADDR, &dato_escritura, 1, false);
//	printf("[F] status [%d]\n", status);
//	osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
//	osDelay(100);  // Write Cycle Time
	
  uint8_t dato_lectura;
	
  // Leo dato
  status = I2Cdrv->MasterTransmit(SLAVE_ADDR, direccion_dato, 2, false);
  printf("[G] status [%d]\n", status);
  osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
  osDelay(100);
	
  status = I2Cdrv->MasterReceive(SLAVE_ADDR, &dato_lectura, 1, false);  // Read 1 byte of data
  printf("[H] status [%d]\n", status);
  osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
	
  printf("[memoria::%s] Dato leido [0x%02X]\n", __func__, dato_lectura);

  while (1)
  {
		
  }
}

static void SoftwareReset(void)
{
	int32_t status;
	printf("[memoria::%s]\n", __func__);
	uint8_t buff = 0xFF;
	status = I2Cdrv->MasterTransmit(SLAVE_ADDR, &buff, 1, false);
	osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
	osDelay(100);
}

static void I2C_SignalEvent(uint32_t event) 
{
	printf("I2C_SignalEvent_Memoria [%#x]\n", event);
  if (event & ARM_I2C_EVENT_TRANSFER_DONE) 
  {
    /* Transfer or receive is finished */
		osThreadFlagsSet(e_memoriaThreadId, ARM_I2C_EVENT_TRANSFER_DONE);
  }
}
