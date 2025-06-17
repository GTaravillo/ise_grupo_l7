
#include "Memoria.h"
/* ARM */
#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"
/* std */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include PATH_TEST_MEMORIA

#define SLAVE_ADDR 0x50

#define TAM_MEMORIA      512 * 64 // AT24C256C
#define LAST_ACCESS_BYTE 0x7FFF

// #define ID_FIN_PARTIDA    0xEE     // Byte que identifica el final de una partida en memoria
#define NUM_BYTES_PARTIDA_FINALIZADA    TAM_FECHA + TAM_HORA + (2 * TAM_NOMBRE_JUGADOR) + 1 + (2 * TAM_TIEMPO_JUGADOR) // 45 BYTES
#define NUM_BYTES_PARTIDA_SIN_FINALIZAR NUM_BYTES_PARTIDA_FINALIZADA + TAM_DATOS  // 109 bytes
/**
 * 32768 bytes de almacenamiento
 * Mapa bits: almacena num byte de registro en el que se encuentra la partida. Requiere 2 bytes por partida
 *   Primeros dos bytes: Posicion ultima partida en mapa memoria
 * Solo se puede almacenar una partida sin finalizar, por lo que el número máximo de partidas finalizadas es:
 *   TAM_MEMORIA - NUM_BYTES_PARTIDA_SIN_FINALIZAR - NUM_PARTIDAS_MAX * (NUM_BYTES_PARTIDA_FINALIZADA + 2) > 0
 *   NUM_PARTIDAS_MAX < (TAM_MEMORIA - NUM_BYTES_PARTIDA_SIN_FINALIZAR) / (NUM_BYTES_PARTIDA_FINALIZADA + 2)
 * De lo que se saca que NUM_PARTIDAS_MAX = 694
 */
#define NUM_PARTIDAS_MAX 694

#define OFFSET_NUM_PARTIDAS   0
#define OFFSET_MAPA_REGISTROS OFFSET_NUM_PARTIDAS + 2
#define DIRECCION_PARTIDA_RETOMAR OFFSET_MAPA_REGISTROS + (NUM_PARTIDAS_MAX * 2)
#define DIRECCION_PRIMERA_FINALIZADA DIRECCION_PARTIDA_RETOMAR + NUM_BYTES_PARTIDA_SIN_FINALIZAR

#define OFFSET_FECHA_PARTIDA  0
#define OFFSET_HORA_PARTIDA   OFFSET_FECHA_PARTIDA  + TAM_FECHA
#define OFFSET_NOMBRE_BLANCAS OFFSET_HORA_PARTIDA   + TAM_HORA
#define OFFSET_NOMBRE_NEGRAS  OFFSET_NOMBRE_BLANCAS + TAM_NOMBRE_JUGADOR
#define OFFSET_TURNO_VICTORIA OFFSET_NOMBRE_NEGRAS  + TAM_NOMBRE_JUGADOR
#define OFFSET_TIEMPO_BLANCAS OFFSET_TURNO_VICTORIA + 1
#define OFFSET_TIEMPO_NEGRAS  OFFSET_TIEMPO_BLANCAS + TAM_TIEMPO_JUGADOR
#define OFFSET_TABLERO        OFFSET_TIEMPO_NEGRAS  + TAM_TIEMPO_JUGADOR

/* Driver I2C */
extern ARM_DRIVER_I2C Driver_I2C2;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C2;

osThreadId_t        e_memoriaThreadId;
osMessageQueueId_t  e_memoriaRxMessageId;
osMessageQueueId_t  e_memoriaTxMessageId;

uint16_t numPartidasTerminadas;

/* Private */
static void Run(void *argument);

static void SoftwareReset(void);
static void ActualizarNumPartidasTerminadas(void);

static void LimpiarMemoria(void);

static void ProcesarPeticion(PartidaInMsg_t mensajeRx);
static void ProcesarGuardarPartidaSinFinalizar(PartidaInMsg_t mensajeRx);
static void ProcesarGuardarPartidaFinalizada(PartidaInMsg_t mensajeRx);
static void ProcesarRetomarPartida(void);

static uint8_t LeerByte(uint16_t direccion);
static void EscribirByte(uint16_t direccion, uint8_t dato);

static void  I2C_SignalEvent(uint32_t event);

/**************************************/

void MemoriaInitialize(void)
{
  e_memoriaThreadId    = osThreadNew(Run, NULL, NULL);
  e_memoriaRxMessageId = osMessageQueueNew(TAM_COLA_MSGS_RX, sizeof(PartidaInMsg_t), NULL);
  e_memoriaTxMessageId = osMessageQueueNew(TAM_COLA_MSGS_TX, sizeof(PartidaOutMsg_t), NULL);

  if ((e_memoriaThreadId == NULL) || (e_memoriaRxMessageId == NULL) || (e_memoriaTxMessageId == NULL))
  {
    printf("[memoria::%s] ERROR! osThreadNew [%d]\n", __func__, (e_memoriaThreadId == NULL));
  }
}

static void Run(void *argument)
{
  osStatus_t status;
  printf("[memoria::Run] Initializing I2C\n");
  status = I2Cdrv->Initialize(I2C_SignalEvent);
  osDelay(10);
  status = I2Cdrv->PowerControl (ARM_POWER_FULL);
  osDelay(10);
  status = I2Cdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
  osDelay(10);
  status = I2Cdrv->Control(ARM_I2C_BUS_CLEAR, NULL);
  osDelay(10);

  // EscribirByte(0, 0);
  // EscribirByte(1, 0);
  // osDelay(100000);
  // LimpiarMemoria(); // CUIDADO! Elimina todos los datos en memoria
  ActualizarNumPartidasTerminadas();
  osThreadFlagsSet(e_testMemoriaThreadId, FLAG_INIT_COMPLETE);
  
  //SoftwareReset();
  PartidaInMsg_t mensajeRx;

  while (1)
  {
    memset(&mensajeRx, 0, sizeof(mensajeRx));
	  printf("[memoria::%s] ESPERANDO MENSAJE\n", __func__);
    osThreadFlagsSet(e_testMemoriaThreadId, FLAG_READY);
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
	osDelay(10);
}

static void ActualizarNumPartidasTerminadas(void)
{
  numPartidasTerminadas = 0;

  uint8_t msb = LeerByte(OFFSET_NUM_PARTIDAS);
  uint8_t lsb = LeerByte(OFFSET_NUM_PARTIDAS + 1);

  numPartidasTerminadas = msb | lsb;

  printf("[memoria::%s] Numero partidas terminadas [%d]\n", __func__, numPartidasTerminadas);
}

static void ProcesarPeticion(PartidaInMsg_t mensajeRx)
{
  printf("[memoria::%s] TIPO MENSAJE: [%d]\n", __func__, mensajeRx.tipoPeticion);
  
  switch(mensajeRx.tipoPeticion)
  {
    case GUARDAR_PARTIDA_SIN_FINALIZAR:
      ProcesarGuardarPartidaSinFinalizar(mensajeRx);
    break;

    case GUARDAR_PARTIDA_FINALIZADA:
      ProcesarGuardarPartidaFinalizada(mensajeRx);
    break;

    case RETOMAR_ULTIMA_PARTIDA:
      ProcesarRetomarPartida();
	  break;

    

    case LIMPIAR_MEMORIA:
      LimpiarMemoria();
    break;

	  default:
	  break;
  }
}

static void ProcesarGuardarPartidaSinFinalizar(PartidaInMsg_t mensajeRx)
{
  osStatus_t status;

  uint16_t direccionGuardado = DIRECCION_PARTIDA_RETOMAR;                        

  for (int i = 0; i < TAM_FECHA; i++)
  {
    direccionGuardado = DIRECCION_PARTIDA_RETOMAR + OFFSET_FECHA_PARTIDA + i;
    printf("[Memoria::%s] Fecha[%d]: [0x%04X] = [0x%02X]\n", __func__, i, direccionGuardado, mensajeRx.fechaPartida[i]);
    EscribirByte(direccionGuardado, mensajeRx.fechaPartida[i]);
  }

  for (int i = 0; i < TAM_HORA; i++)
  {
    direccionGuardado = DIRECCION_PARTIDA_RETOMAR + OFFSET_HORA_PARTIDA + i;
    printf("[Memoria::%s] Hora[%d]: [0x%04X] = [0x%02X]\n", __func__, i, direccionGuardado, mensajeRx.horaPartida[i]);
    EscribirByte(direccionGuardado, mensajeRx.horaPartida[i]);
  }

  for (int i = 0; i < TAM_NOMBRE_JUGADOR; i++)
  {
    direccionGuardado = DIRECCION_PARTIDA_RETOMAR + OFFSET_NOMBRE_BLANCAS + i;
    printf("[Memoria::%s] Nombre blancas[%d]: [0x%04X] = [0x%02X]\n", __func__, i, direccionGuardado, mensajeRx.nombreBlancas[i]);
    EscribirByte(direccionGuardado, mensajeRx.nombreBlancas[i]);
  }

  for (int i = 0; i < TAM_NOMBRE_JUGADOR; i++)
  {
    direccionGuardado = DIRECCION_PARTIDA_RETOMAR + OFFSET_NOMBRE_NEGRAS + i;
    printf("[Memoria::%s] Nombre negras[%d]: [0x%04X] = [0x%02X]\n", __func__, i, direccionGuardado, mensajeRx.nombreNegras[i]);
    EscribirByte(direccionGuardado, mensajeRx.nombreNegras[i]);
  }
  
  direccionGuardado = DIRECCION_PARTIDA_RETOMAR + OFFSET_TURNO_VICTORIA;
  printf("[Memoria::%s] Turno: [0x%04X] = [0x%02X]\n", __func__, direccionGuardado, mensajeRx.turno_victoria);
  EscribirByte(direccionGuardado, mensajeRx.turno_victoria);
  
  for (int i = 0; i < TAM_TIEMPO_JUGADOR; i++)
  {
    direccionGuardado = DIRECCION_PARTIDA_RETOMAR + OFFSET_TIEMPO_BLANCAS + i;
    printf("[Memoria::%s] Tiempo blancas[%d]: [0x%04X] = [0x%02X]\n", __func__, i, direccionGuardado, mensajeRx.tiempoBlancas[i]);
    EscribirByte(direccionGuardado, mensajeRx.tiempoBlancas[i]);
  }

  for (int i = 0; i < TAM_TIEMPO_JUGADOR; i++)
  {
    direccionGuardado = DIRECCION_PARTIDA_RETOMAR + OFFSET_TIEMPO_NEGRAS + i;
    printf("[Memoria::%s] Tiempo negras[%d]: [0x%04X] = [0x%02X]\n", __func__, i, direccionGuardado, mensajeRx.tiempoNegras[i]);
    EscribirByte(direccionGuardado, mensajeRx.tiempoNegras[i]);
  }

  for (int i = 0; i < TAM_DATOS; i++)
  {
    direccionGuardado = DIRECCION_PARTIDA_RETOMAR + OFFSET_TABLERO + i;
    printf("[Memoria::%s] Tablero:\n", __func__);
    EscribirByte(direccionGuardado, mensajeRx.dato[i]);
  }
}

static void ProcesarGuardarPartidaFinalizada(PartidaInMsg_t mensajeRx)
{
  osStatus_t status;

  uint16_t direccionMapa;

  if ((numPartidasTerminadas + 1) < NUM_PARTIDAS_MAX)
  {
    numPartidasTerminadas += 1;
    direccionMapa = OFFSET_MAPA_REGISTROS + (numPartidasTerminadas * 2);
  }
  else 
  {
    numPartidasTerminadas = NUM_PARTIDAS_MAX;
    // Si no queda espacio en memoria, sobreescribe la primera partida
    direccionMapa = OFFSET_MAPA_REGISTROS;
  }

  uint16_t direccionGuardado;

  if (numPartidasTerminadas == NUM_PARTIDAS_MAX)
  {
    // Si no queda espacio en memoria, sobreescribe la primera partida
    direccionGuardado = DIRECCION_PRIMERA_FINALIZADA;
  } 
  else 
  {
    direccionGuardado = DIRECCION_PRIMERA_FINALIZADA + (NUM_BYTES_PARTIDA_FINALIZADA * numPartidasTerminadas);
  }
  
  for (int i = 0; i < TAM_FECHA; i++)
  {
    printf("[Memoria::%s] Fecha:\n", __func__);
    EscribirByte(direccionGuardado + OFFSET_FECHA_PARTIDA + i, mensajeRx.fechaPartida[i]);
  }

  for (int i = 0; i < TAM_HORA; i++)
  {
    printf("[Memoria::%s] Hora:\n", __func__);
    EscribirByte(direccionGuardado + OFFSET_HORA_PARTIDA + i, mensajeRx.horaPartida[i]);
  }

  for (int i = 0; i < TAM_NOMBRE_JUGADOR; i++)
  {
    printf("[Memoria::%s] Nombre blancas:\n", __func__);
    EscribirByte(direccionGuardado + OFFSET_NOMBRE_BLANCAS + i, mensajeRx.nombreBlancas[i]);
  }

  for (int i = 0; i < TAM_NOMBRE_JUGADOR; i++)
  {
    printf("[Memoria::%s] Nombre negras:\n", __func__);
    EscribirByte(direccionGuardado + OFFSET_NOMBRE_NEGRAS + i, mensajeRx.nombreNegras[i]);
  }
  
  printf("[Memoria::%s] Turno:\n", __func__);
  EscribirByte(direccionGuardado + OFFSET_TURNO_VICTORIA, mensajeRx.turno_victoria);
  
  for (int i = 0; i < TAM_TIEMPO_JUGADOR; i++)
  {
    printf("[Memoria::%s] Tiempo blancas:\n", __func__);
    EscribirByte(direccionGuardado + OFFSET_TIEMPO_BLANCAS + i, mensajeRx.tiempoBlancas[i]);
  }

  for (int i = 0; i < TAM_TIEMPO_JUGADOR; i++)
  {
    printf("[Memoria::%s] Tiempo negras:\n", __func__);
    EscribirByte(direccionGuardado + OFFSET_TIEMPO_NEGRAS + i, mensajeRx.tiempoNegras[i]);
  }

  printf("[Memoria::%s] Num partidas terminadas:\n", __func__);
  EscribirByte(OFFSET_NUM_PARTIDAS, (numPartidasTerminadas & 0xFF00));
  EscribirByte(OFFSET_NUM_PARTIDAS + 1, (numPartidasTerminadas & 0x00FF));
  printf("[Memoria::%s] Direccion partida:\n", __func__);
  EscribirByte(direccionMapa, (direccionGuardado & 0xFF00));
  EscribirByte(direccionMapa + 1, (direccionGuardado & 0x00FF));
}

static void ProcesarRetomarPartida()
{
  osStatus_t status;

  PartidaOutMsg_t mensajeTx;
  
  mensajeTx.tipoPeticion  = RETOMAR_ULTIMA_PARTIDA;

  printf("[Memoria::%s] Turno:\n", __func__);
  mensajeTx.turno = LeerByte(DIRECCION_PARTIDA_RETOMAR + OFFSET_TURNO_VICTORIA);
  for (int i = 0; i < TAM_TIEMPO_JUGADOR; i++)
  {
    printf("[Memoria::%s] Tiempo blancas:\n", __func__);
    mensajeTx.tiempoBlancas[i] = LeerByte(DIRECCION_PARTIDA_RETOMAR + OFFSET_TIEMPO_BLANCAS + i);
  }

  for (int i = 0; i < TAM_TIEMPO_JUGADOR; i++)
  {
    printf("[Memoria::%s] Tiempo negras:\n", __func__);
    mensajeTx.tiempoNegras[i]  = LeerByte(DIRECCION_PARTIDA_RETOMAR + OFFSET_TIEMPO_NEGRAS + i);
  }
  
  for (int i = 0; i < TAM_DATOS; i++)
  {
    printf("[Memoria::%s] Tablero:\n", __func__);
    mensajeTx.dato[i] = LeerByte(DIRECCION_PARTIDA_RETOMAR + OFFSET_TABLERO + i);
  }

  status = osMessageQueuePut(e_memoriaTxMessageId, &mensajeTx, 1, 0);
}

// Pone a 0 todos los bytes en memoria
static void LimpiarMemoria(void)
{
  for (int direccion = 0; direccion < TAM_MEMORIA; direccion++)
  {
    EscribirByte(direccion, 0);
  }

  numPartidasTerminadas = 0;
}

// Max: 0x7FFF
static void EscribirByte(uint16_t direccion, uint8_t dato)
{
  int32_t  status;

  uint8_t estructura_Wr[3] = 
  {
    (direccion & 0xFF00) >> 8,
    (direccion & 0x00FF), 
    dato
  };

  printf("[memoria::%s] direccion dato [0x%04X] = [0x%02X]\n", __func__, ((estructura_Wr[0] << 8) | estructura_Wr[1]), dato);

  status = I2Cdrv->MasterTransmit(SLAVE_ADDR, estructura_Wr, 3, false);
  osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
  osDelay(20);
}

static uint8_t LeerByte(uint16_t direccion)
{
  int32_t  status;

  uint8_t  byteLeido = 0x00;
  uint8_t direccion_dato[2] = 
  {
    (direccion & 0xFF00) >> 8,
    (direccion & 0x00FF)
  };

  status = I2Cdrv->MasterTransmit(SLAVE_ADDR, direccion_dato, 2, false);

  osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
  osDelay(10);
	
  status = I2Cdrv->MasterReceive(SLAVE_ADDR, &byteLeido, 1, false);  // Read 1 byte of data
  osThreadFlagsWait(ARM_I2C_EVENT_TRANSFER_DONE, osFlagsWaitAll, osWaitForever);
	
  printf("[memoria::%s] Direccion [0x%04X]: Dato leido [0x%02X]\n", __func__, direccion, byteLeido);
  return byteLeido;
}

static void I2C_SignalEvent(uint32_t event) 
{
	// printf("I2C_SignalEvent_Memoria [%#x]\n", event);
  if (event & ARM_I2C_EVENT_TRANSFER_DONE) 
  {
    /* Transfer or receive is finished */
		osThreadFlagsSet(e_memoriaThreadId, ARM_I2C_EVENT_TRANSFER_DONE);
  }
}
