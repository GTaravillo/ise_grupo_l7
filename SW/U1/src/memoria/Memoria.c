
#include "Memoria.h"
/* ARM */
#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
/* std */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
/* Interfaces */
#include "../config/Paths.h"

#define SLAVE_ADDR 0x50
#define PAGE_NUM   127
#define PAGE_SIZE  64
#define PAGINA_MAPA_PARTIDAS 0X00
#define PAGINA_ULT_PARTIDA   0x01


/* Driver I2C */
extern ARM_DRIVER_I2C Driver_I2C2;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C2;

osThreadId_t        e_memoriaThreadId;
osMessageQueueId_t  e_memoriaRxMessageId;
osMessageQueueId_t  e_memoriaTxMessageId;

/* Private */
static void Run(void *argument);

static void SoftwareReset(void);
static void ProcesarPeticion(MemoriaInMsg_t mensajeRx);
static uint8_t LeerByte(uint8_t pagina, uint8_t numByte);
static void EscribirByte(uint8_t pagina, uint8_t numByte, uint8_t dato);

static void  I2C_SignalEvent(uint32_t event);

/**************************************/

void MemoriaInitialize(void)
{
  e_memoriaThreadId    = osThreadNew(Run, NULL, NULL);
  e_memoriaRxMessageId = osMessageQueueNew(TAM_COLA_MSGS_RX, sizeof(MemoriaInMsg_t), NULL);
  e_memoriaTxMessageId = osMessageQueueNew(TAM_COLA_MSGS_TX, sizeof(MemoriaOutMsg_t), NULL);

  if ((e_memoriaThreadId == NULL) || (e_memoriaRxMessageId == NULL) || (e_memoriaTxMessageId == NULL))
  {
    printf("[position::%s] ERROR! osThreadNew [%d]\n", __func__, (e_memoriaThreadId == NULL));
  }
}

static void Run(void *argument)
{
  osStatus_t status;
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
  MemoriaInMsg_t mensajeRx;

  while (1)
  {
    memset(&mensajeRx, 0, sizeof(mensajeRx));
	printf("[memoria::%s] ESPERANDO MENSAJE\n", __func__);
    status = osMessageQueueGet(e_memoriaRxMessageId, &mensajeRx, NULL, osWaitForever);
    printf("[memoria::%s] MENSAJE RECIBIDO\n", __func__);
    ProcesarPeticion(mensajeRx);
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

static void ProcesarPeticion(MemoriaInMsg_t mensajeRx)
{
  osStatus_t status;
  printf("[memoria::%s] TIPO MENSAJE: [%d]\n", __func__, mensajeRx.tipoPeticion);
  
  switch(mensajeRx.tipoPeticion)
  {
    case ESCRIBIR_DATO:
      EscribirByte(PAGINA_ULT_PARTIDA, mensajeRx.posicion, mensajeRx.dato);
      break;

    case RETOMAR_ULTIMA_PARTIDA:
    {
      MemoriaOutMsg_t mensajeTx;
      uint8_t byteLeido;

	  for (int i = 0; i < 64; i++)
	  {
	    byteLeido = 0x00;
        memset(&mensajeTx, 0, sizeof(mensajeTx));

	    uint8_t paginaLectura = PAGINA_ULT_PARTIDA;
	    byteLeido = LeerByte(paginaLectura, i);
	    mensajeTx.dato[i] = byteLeido;

        status = osMessageQueuePut(e_memoriaTxMessageId, &mensajeTx, 1, 0);
	  }
	} break;

	default:
	  break;
  }
}

// Max: 0x7FFF
// Da para 127 páginas de 64 bytes
static void EscribirByte(uint8_t pagina, uint8_t numByte, uint8_t dato)
{
  int32_t  status;
  uint16_t direccion = (pagina * 64) + numByte;  // Direccion del byte en concreto
  
  uint8_t estructura_Wr[3] = 
  {
    (direccion & 0xFF00) >> 8,
    (direccion & 0x00FF), 
    dato
  };

  printf("[memoria::%s] direccion dato [0x%02X 0x%02X] = [%d]\n", __func__, estructura_Wr[0], estructura_Wr[1], dato);

  /* Escribo en las direcciones 0x0000 a 0x0020 (32 bytes) */

  // Addr
  status = I2Cdrv->MasterTransmit(SLAVE_ADDR, estructura_Wr, 3, false);
  osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
  osDelay(100);
}

static uint8_t LeerByte(uint8_t pagina, uint8_t numByte)
{
  int32_t  status;
  uint16_t direccion = (pagina * 64) + numByte;  // Direccion del byte en concreto
  uint8_t  byteLeido = 0x00;

  uint8_t direccion_dato[2] = 
  {
    (direccion & 0xFF00) >> 8,
    (direccion & 0x00FF)
  };

  status = I2Cdrv->MasterTransmit(SLAVE_ADDR, direccion_dato, 2, false);
  printf("[G] status [%d]\n", status);

  osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
  osDelay(100);
	
  status = I2Cdrv->MasterReceive(SLAVE_ADDR, &byteLeido, 1, false);  // Read 1 byte of data
  printf("[H] status [%d]\n", status);
  osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
	
  printf("[memoria::%s] Dato leido [0x%02X]\n", __func__, byteLeido);
  return byteLeido;
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
