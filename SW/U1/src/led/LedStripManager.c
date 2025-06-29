#include "LedStripManager.h"
/* ARM */
#include "driver_SPI.h"
/* std */
#include <stdio.h>
#include <string.h>
/* Interfaces */
#include "../config/Paths.h"
#include PATH_COMMON

/* Registros */
#define  SPICLK               10000000
#define  START                0x00  // 4
#define  STOP                 0xFF  // 4
#define  BRIGHTNESS_MASK      0xE0  // [D7:D5]  // Brightness (0-31)
/* Flags */
#define LCD_TRANSFER_COMPLETE 0xFF  // Used to signal the end of SPI transfer
/* Control */
#define BRILLO_DEFECTO        15
/* Cola mensajes */
#define NUMERO_MENSAJES_MAX   30    // 28 maximo teorico. 2 salvaguarda


/* Public */

osMessageQueueId_t  e_ledStripMessageId;  // Cola recepción datos
osThreadId_t        e_ledStripManagerThreadId;

/* Private */

typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} ColorLed_t;

// Proporciona numero de led a partir de la posicion [columna][fila]
/**
 * Ej: 
 * Quiero encender A2: [A][2]
 * Mando mensaje a LedStripManager con mensaje 0x12 0x00 (padding)
 * Gestión interna: 0x12 -> [1-1][2-1] == [0][1] = se enciende LED 15
 *  */ 
// const static uint8_t g_numeroLedMap[8][8] = {
// //  1   2   3   4   5   6   7   8
//   { 0, 15, 16, 31, 32, 47, 48, 63 }, // A 
//   { 1, 14, 17, 30, 33, 46, 49, 62 }, // B
//   { 2, 13, 18, 29, 34, 45, 50, 61 }, // C
//   { 3, 12, 19, 28, 35, 44, 51, 60 }, // D
//   { 4, 11, 20, 27, 36, 43, 52, 59 }, // E
//   { 5, 10, 21, 26, 37, 42, 53, 58 }, // F
//   { 6,  9, 22, 25, 38, 41, 54, 57 }, // G
//   { 7,  8, 23, 24, 39, 40, 55, 56 }  // H
// };

extern ARM_DRIVER_SPI   Driver_SPI2;
       ARM_DRIVER_SPI*  SPIdrv2 = &Driver_SPI2;


static ColorLed_t g_leds[64];  // Almacena datos colores de los LEDs a encender
static uint8_t brillo = BRILLO_DEFECTO;

static void Run(void *argument);
static void InitializeSpiDriver(void);
static bool RecepcionCorrecta(osStatus_t status);
static bool PosicionRecibidaValida(LedStripMsg_t mensajeRx);
static void ProcesarMensaje(LedStripMsg_t mensajeRx);
static void StartCommunication(void);
static void StopCommunication(void);
static void EnviarComando(unsigned char cmd);
static void GetColor(ETipoJugada tipoJugada, ColorLed_t* colores);
static void EnviarDatos(void);
static void TestLeds(void);
static void TurnOff(void);
static void StartAckPattern(void);
static void StartNackPattern(void);

void  ARM_LedSPI_SignalEvent(uint32_t event);

/**************************************/

void LedStripManagerInitialize(void)
{
  e_ledStripManagerThreadId = osThreadNew(Run, NULL, NULL);
  e_ledStripMessageId       = osMessageQueueNew(NUMERO_MENSAJES_MAX, sizeof(LedStripMsg_t), NULL);

  if ((e_ledStripManagerThreadId == NULL) || (e_ledStripMessageId == NULL))
  {
    //// printf"[lcd::%s] ERROR! osThreadNew [%d]\n", __func__, (e_ledStripManagerThreadId == NULL));
  }
}

static void Run(void *argument)
{
  InitializeSpiDriver();
  TestLeds();
  TurnOff();

  osStatus_t    status;
  LedStripMsg_t mensajeRx;

  while(1)
  {
    osStatus_t status;

    memset(&mensajeRx, 0, sizeof(mensajeRx));
    EnviarDatos();
    status = osMessageQueueGet(e_ledStripMessageId, &mensajeRx, NULL, osWaitForever);
    if (mensajeRx.tipoJugada == ACK)
    {
      TurnOff();
      StartAckPattern();
      TurnOff();
    }
    else if (mensajeRx.tipoJugada == NACK)
    {
      TurnOff();
      StartNackPattern();
      TurnOff();
    }
    else if (RecepcionCorrecta(status) && PosicionRecibidaValida(mensajeRx))
    {
      ProcesarMensaje(mensajeRx);
      EnviarDatos();
    }
  }
}

static void InitializeSpiDriver(void)
{
  /** Inicializacion y configuracion del driver SPI **/
  /* Initialize the SPI driver */
  SPIdrv2->Initialize(ARM_LedSPI_SignalEvent);
  /* Power up the SPI peripheral */
  SPIdrv2->PowerControl(ARM_POWER_FULL);
  /* Configure the SPI to Master, 8-bit mode @20000 kBits/sec */
  SPIdrv2->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8), SPICLK);
}

static bool RecepcionCorrecta(osStatus_t status)
{
  switch (status)
  {
    case osOK:
      return true;

    case osErrorTimeout:
    // // printf"[LedStrip::%s] The message could not be retrieved from the queue in the given time\n", __func__);
      return false;

    case osErrorResource:
     //// printf"[LedStrip::%s] Nothing to get from the queue\n", __func__);
      return false;

    case osErrorParameter:
    // // printf"[LedStrip::%s] Parameter mq_id is NULL or invalid, non-zero timeout specified in an ISR\n", __func__);
      return false;

    default:
    // // printf"[LedStrip::%s] Unknown error [%d]\n", __func__, status);
      return false;
  }
}

static bool PosicionRecibidaValida(LedStripMsg_t mensajeRx)
{
  const uint8_t posicion  = mensajeRx.posicion;
	char* posicionStr;
	PositionToString(posicion, posicionStr);

 //// printf"[LedStrip::%s] RECIBIDO: posicion[%d] (%s)\n", __func__, posicion, posicionStr);
  if (posicion < 64)
  {
    return true;
  }
  
 //// printf"[LedStrip::%s] ERROR! Posicion [%d] invalida\n", __func__, posicion);
  return false;
}

static void ProcesarMensaje(LedStripMsg_t mensajeRx)
{
  // Si es jugada nueva, vacio array de leds
  const bool nuevaJugada = mensajeRx.nuevaJugada;
  if (nuevaJugada)
  {
  // // printf"[LedStrip::%s] RESET LEDS\n", __func__);
    memset(g_leds, 0, sizeof(g_leds));
    return;
  }

  // Cojo datos mensaje
  ColorLed_t colores;
  const ETipoJugada tipoJugada = mensajeRx.tipoJugada;
  GetColor(tipoJugada, &colores);

// // printf"[LedStrip::%s] ENCENDER LED [%d]\n", __func__, mensajeRx.posicion);
  g_leds[mensajeRx.posicion] = colores;
}

static void GetColor(ETipoJugada tipoJugada, ColorLed_t* colores)
{
	if (colores == NULL)
	{
		//printf("[LedStrip::%s] ERROR! Parametro colores es NULL\n", __func__);
		return;
	}
	
	switch (tipoJugada)
	{
		case POSIBLE_MOVIMIENTO:
			colores->red   = 0;
		  colores->green = 0;
		  colores->blue  = 255;
		 //// printf"[LedStrip::%s] COLOR AZUL\n", __func__);
		  break;
		
		case MOVIMIENTO_ILEGAL:
			colores->red   = 255;
		  colores->green = 0;
		  colores->blue  = 0;
		 //// printf"[LedStrip::%s] COLOR ROJO\n", __func__);
		  break;
		
		case CAPTURA:
			colores->red   = 0;
		  colores->green = 255;
		  colores->blue  = 0;
		 //// printf"[LedStrip::%s] COLOR VERDE\n", __func__);
		  break;
		
		case ESPECIAL:
			colores->red   = 255;
		  colores->green = 0;
		  colores->blue  = 255;
		// // printf"[LedStrip::%s] COLOR MORADO\n", __func__);
		  break;
		
		case ACTUAL:
			colores->red   = 255;
		  colores->green = 255;
		  colores->blue  = 255;
		  //printf("[LedStrip::%s] COLOR BLANCO\n", __func__);
      break;
    
    case ACK:
      colores->red   = 0;
		  colores->green = 255;
		  colores->blue  = 0;
    break;
    
    case NACK:
      colores->red   = 255;
		  colores->green = 0;
		  colores->blue  = 0;
    break;
		
		default:
			//printf("[LedStrip::%s] Tipo de jugada desconocida\n", __func__);
			break;
	}
}

static void EnviarDatos(void)
{
	StartCommunication();

	for (int i = 0; i < sizeof(g_leds)/sizeof(g_leds[0]); i++)
	{
    const uint8_t azul  = g_leds[i].blue;
    const uint8_t verde = g_leds[i].green;
    const uint8_t rojo  = g_leds[i].red;
		//printf("[LedStrip::%s] led[%d] R[%d] G[%d] B[%d]\n", __func__, i, azul, verde, rojo);

		EnviarComando(BRIGHTNESS_MASK | brillo);
    EnviarComando(azul);  // B
    EnviarComando(verde); // G
    EnviarComando(rojo);  // R
	}

	StopCommunication();
}

static void StartCommunication(void)
{
  for (int i = 0; i < 4; i++)
  {
    EnviarComando(START);
  }
}

static void StopCommunication(void)
{
  for (int i = 0; i < 4; i++)
  {
    EnviarComando(STOP);
  }
}

static void EnviarComando(unsigned char cmd)
{
  SPIdrv2->Send(&cmd, sizeof(cmd));
  osThreadFlagsWait(LCD_TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
}

static void TestLeds(void)
{
  ColorLed_t colores;
  const ETipoJugada tipoJugada = CAPTURA;
  GetColor(tipoJugada, &colores);

  StartCommunication();
  for (int i = 0; i < sizeof(g_leds)/sizeof(g_leds[0]); i++)
	{
    g_leds[i]           = colores;
    const uint8_t azul  = g_leds[i].blue;
    const uint8_t verde = g_leds[i].green;
    const uint8_t rojo  = g_leds[i].red;
		//printf("[LedStrip::%s] led[%d] R[%d] G[%d] B[%d]\n", __func__, i, azul, verde, rojo);

		EnviarComando(BRIGHTNESS_MASK | brillo);
    EnviarComando(azul);  // B
    EnviarComando(verde); // G
    EnviarComando(rojo);  // R
    osDelay(5);
	}
  StopCommunication();
}

static void TurnOff(void)
{
  memset(g_leds, 0, sizeof(g_leds));
  StartCommunication();
  for (int i = 0; i < sizeof(g_leds)/sizeof(g_leds[0]); i++)
	{
    const uint8_t azul  = g_leds[i].blue;
    const uint8_t verde = g_leds[i].green;
    const uint8_t rojo  = g_leds[i].red;
		//printf("[LedStrip::%s] led[%d] R[%d] G[%d] B[%d]\n", __func__, i, azul, verde, rojo);

		EnviarComando(BRIGHTNESS_MASK | 0);
    EnviarComando(azul);  // B
    EnviarComando(verde); // G
    EnviarComando(rojo);  // R
    osDelay(5);
	}
  StopCommunication();
}

static void StartAckPattern(void)
{
  ColorLed_t colores;
  const ETipoJugada tipoJugada = ACK;
  GetColor(tipoJugada, &colores);

  StartCommunication();
  for (int i = 0; i < sizeof(g_leds)/sizeof(g_leds[0]); i++)
	{
    g_leds[i]           = colores;
    const uint8_t azul  = g_leds[i].blue;
    const uint8_t verde = g_leds[i].green;
    const uint8_t rojo  = g_leds[i].red;
		//printf("[LedStrip::%s] led[%d] R[%d] G[%d] B[%d]\n", __func__, i, azul, verde, rojo);

		EnviarComando(BRIGHTNESS_MASK | brillo);
    EnviarComando(azul);  // B
    EnviarComando(verde); // G
    EnviarComando(rojo);  // R
    osDelay(5);
	}
  StopCommunication();
  osDelay(5);
}

static void StartNackPattern(void)
{
  ColorLed_t colores;
  const ETipoJugada tipoJugada = NACK;
  GetColor(tipoJugada, &colores);

  StartCommunication();
  for (int i = 0; i < sizeof(g_leds)/sizeof(g_leds[0]); i++)
	{
    g_leds[i]           = colores;
    const uint8_t azul  = g_leds[i].blue;
    const uint8_t verde = g_leds[i].green;
    const uint8_t rojo  = g_leds[i].red;
		//printf("[LedStrip::%s] led[%d] R[%d] G[%d] B[%d]\n", __func__, i, azul, verde, rojo);

		EnviarComando(BRIGHTNESS_MASK | brillo);
    EnviarComando(azul);  // B
    EnviarComando(verde); // G
    EnviarComando(rojo);  // R
    osDelay(5);
	}
  StopCommunication();
  osDelay(5);
}

void ARM_LedSPI_SignalEvent(uint32_t event)
{
  if (event & ARM_SPI_EVENT_TRANSFER_COMPLETE)
  {
    // Data Transfer completed
    osThreadFlagsSet(e_ledStripManagerThreadId, LCD_TRANSFER_COMPLETE);
  }
  if (event & ARM_SPI_EVENT_MODE_FAULT)
  {
		printf("[LedStrip::%s] ARM_SPI_EVENT_MODE_FAULT", __func__);
    // Master Mode Fault (SS deactivated when Master)
  }
  if (event & ARM_SPI_EVENT_DATA_LOST)
  {
		printf("[LedStrip::%s] ARM_SPI_EVENT_DATA_LOST", __func__);
    // Data lost: Receive overflow / Transmit underflow
  }
}
