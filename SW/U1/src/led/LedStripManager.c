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
#define LED_TRANSFER_COMPLETE 0xFF  // Used to signal the end of SPI transfer
/* Control */
#define BRILLO_DEFECTO        15
/* Cola mensajes */
#define NUMERO_MENSAJES_MAX   30    // 28 maximo teorico. 2 salvaguarda


/* Public */

osMessageQueueId_t  e_ledStripMessageId;  // Cola recepción datos
osThreadId_t        e_ledStripManagerThreadId;

/* Private */

typedef struct {
  uint8_t brillo;
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

static void Run(void *argument);
static void InitializeSpiDriver(void);

static bool RecepcionCorrecta(osStatus_t status);
static bool PosicionRecibidaValida(LedStripMsg_t mensajeRx);

static void ProcesarMensaje(LedStripMsg_t mensajeRx);
static void ProcesarPosibleMovimiento(LedStripMsg_t mensajeRx);
static void ProcesarMovimientoIlegal(LedStripMsg_t mensajeRx);
static void ProcesarCaptura(LedStripMsg_t mensajeRx);
static void ProcesarJugadaEspecial(LedStripMsg_t mensajeRx);
static void ProcesarPosicionActual(LedStripMsg_t mensajeRx);
static void ProcesarApagarCasilla(LedStripMsg_t mensajeRx);
static void ProcesarApagarTablero(void);
static void ProcesarPatronAck(void);
static void ProcesarPatronNack(void);
static void ProcesarPatronPiezaComida(void);
static void ProcesarPatronTableroPuesto(void);

static void TestLeds(void);
static void TurnOff(void);

static void EnviarDatos(void);
static void StartCommunication(void);
static void EnviarComando(unsigned char cmd);
static void StopCommunication(void);

void  ARM_LedSPI_SignalEvent(uint32_t event);

/**************************************/

void LedStripManagerInitialize(void)
{
  e_ledStripManagerThreadId = osThreadNew(Run, NULL, NULL);
  e_ledStripMessageId       = osMessageQueueNew(NUMERO_MENSAJES_MAX, sizeof(LedStripMsg_t), NULL);

  const bool errorThread = e_ledStripManagerThreadId == NULL;
  const bool errorQueue  = e_ledStripMessageId == NULL;
  if (errorThread || errorQueue)
  {
    // printf("[LED::%s] ERROR! thread[%d] queue[%d]\n", __func__, errorThread, errorQueue);
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

    status = osMessageQueueGet(e_ledStripMessageId, &mensajeRx, NULL, osWaitForever);

    if (RecepcionCorrecta(status))
    {
      ProcesarMensaje(mensajeRx);
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
    //  printf("[LedStrip::%s] The message could not be retrieved from the queue in the given time\n", __func__);
      return false;

    case osErrorResource:
    //  printf("[LedStrip::%s] Nothing to get from the queue\n", __func__);
      return false;

    case osErrorParameter:
      // printf("[LedStrip::%s] Parameter mq_id is NULL or invalid, non-zero timeout specified in an ISR\n", __func__);
      return false;

    default:
      // printf("[LedStrip::%s] Unknown error [%d]\n", __func__, status);
      return false;
  }
}

static bool PosicionRecibidaValida(LedStripMsg_t mensajeRx)
{
  const uint8_t posicion  = mensajeRx.posicion;

  if (posicion < 64)
  {
    return true;
  }
  
  return false;
}

static void ProcesarMensaje(LedStripMsg_t mensajeRx)
{
  switch(mensajeRx.tipoJugada)
  {
    case POSIBLE_MOVIMIENTO:
      ProcesarPosibleMovimiento(mensajeRx);
    break;

    case MOVIMIENTO_ILEGAL:
      ProcesarMovimientoIlegal(mensajeRx);
    break;

    case CAPTURA:
      ProcesarCaptura(mensajeRx);
    break;

    case ESPECIAL:
      ProcesarJugadaEspecial(mensajeRx);
    break;

    case ACTUAL:
      ProcesarPosicionActual(mensajeRx);
    break;

    case APAGAR_CASILLA:
      ProcesarApagarCasilla(mensajeRx);
    break;

    case APAGAR_TABLERO:
      ProcesarApagarTablero();
    break;

    case ACK:
      ProcesarPatronAck();
    break;

    case NACK:
      ProcesarPatronNack();
    break;

    case PIEZA_COMIDA:
      ProcesarPatronPiezaComida();
    break;

    case TABLERO_PUESTO:
      ProcesarPatronTableroPuesto();
    break;

    default:
    break;
  }
}

static void ProcesarPosibleMovimiento(LedStripMsg_t mensajeRx)
{
  if (!PosicionRecibidaValida(mensajeRx))
  {
    return;
  }

  if (mensajeRx.nuevaJugada)
  {
    TurnOff();
  }

  g_leds[mensajeRx.posicion].brillo = BRILLO_DEFECTO;
  g_leds[mensajeRx.posicion].red   = 0;
  g_leds[mensajeRx.posicion].green = 0;
  g_leds[mensajeRx.posicion].blue  = 255;
  
  EnviarDatos();
}

static void ProcesarMovimientoIlegal(LedStripMsg_t mensajeRx)
{
  if (!PosicionRecibidaValida(mensajeRx))
  {
    return;
  }

  if (mensajeRx.nuevaJugada)
  {
    TurnOff();
  }

	g_leds[mensajeRx.posicion].brillo = BRILLO_DEFECTO;
  g_leds[mensajeRx.posicion].red   = 255;
  g_leds[mensajeRx.posicion].green = 0;
  g_leds[mensajeRx.posicion].blue  = 0;
  
  EnviarDatos();
}

static void ProcesarCaptura(LedStripMsg_t mensajeRx)
{
  if (!PosicionRecibidaValida(mensajeRx))
  {
    return;
  }

  if (mensajeRx.nuevaJugada)
  {
    TurnOff();
  }
	
	g_leds[mensajeRx.posicion].brillo = BRILLO_DEFECTO;
  g_leds[mensajeRx.posicion].red   = 0;
  g_leds[mensajeRx.posicion].green = 255;
  g_leds[mensajeRx.posicion].blue  = 0;
  
  EnviarDatos();
}

static void ProcesarJugadaEspecial(LedStripMsg_t mensajeRx)
{
  if (!PosicionRecibidaValida(mensajeRx))
  {
    return;
  }

  if (mensajeRx.nuevaJugada)
  {
    TurnOff();
  }

  g_leds[mensajeRx.posicion].brillo = BRILLO_DEFECTO;
  g_leds[mensajeRx.posicion].red   = 255;
  g_leds[mensajeRx.posicion].green = 0;
  g_leds[mensajeRx.posicion].blue  = 255;
  
  EnviarDatos();
}

static void ProcesarPosicionActual(LedStripMsg_t mensajeRx)
{
  if (!PosicionRecibidaValida(mensajeRx))
  {
    return;
  }

  if (mensajeRx.nuevaJugada)
  {
    TurnOff();
  }

  g_leds[mensajeRx.posicion].brillo = BRILLO_DEFECTO;
  g_leds[mensajeRx.posicion].red   = 255;
  g_leds[mensajeRx.posicion].green = 255;
  g_leds[mensajeRx.posicion].blue  = 255;
  
  EnviarDatos();
}

static void ProcesarApagarCasilla(LedStripMsg_t mensajeRx)
{
  if (!PosicionRecibidaValida(mensajeRx))
  {
    return;
  }

  if (mensajeRx.nuevaJugada)
  {
    TurnOff();
  }

  g_leds[mensajeRx.posicion].brillo = 0;
  g_leds[mensajeRx.posicion].red   = 0;
  g_leds[mensajeRx.posicion].green = 0;
  g_leds[mensajeRx.posicion].blue  = 0;
  
  EnviarDatos();
}

static void ProcesarApagarTablero(void)
{
  TurnOff();
}

static void ProcesarPatronAck(void)
{
  memset(g_leds, 0, sizeof(g_leds));

  StartCommunication();
  for (int i = 0; i < sizeof(g_leds)/sizeof(g_leds[0]); i++)
	{
    const uint8_t azul  = 0;
    const uint8_t verde = 255;
    const uint8_t rojo  = 0;

		EnviarComando(BRIGHTNESS_MASK | BRILLO_DEFECTO);
    EnviarComando(azul);  // B
    EnviarComando(verde); // G
    EnviarComando(rojo);  // R
	}
  StopCommunication();
  osDelay(100);

  TurnOff();
}

static void ProcesarPatronNack(void)
{
  memset(g_leds, 0, sizeof(g_leds));

  StartCommunication();
  for (int i = 0; i < sizeof(g_leds)/sizeof(g_leds[0]); i++)
	{
    const uint8_t azul  = 0;
    const uint8_t verde = 0;
    const uint8_t rojo  = 255;

		EnviarComando(BRIGHTNESS_MASK | BRILLO_DEFECTO);
    EnviarComando(azul);  // B
    EnviarComando(verde); // G
    EnviarComando(rojo);  // R
	}
  StopCommunication();
  osDelay(100);

  TurnOff();
}

static void ProcesarPatronPiezaComida(void)
{
  memset(g_leds, 0, sizeof(g_leds));

  StartCommunication();
  for (int i = 0; i < sizeof(g_leds)/sizeof(g_leds[0]); i++)
	{
    const uint8_t azul  = 255;
    const uint8_t verde = 0;
    const uint8_t rojo  = 0;

		EnviarComando(BRIGHTNESS_MASK | BRILLO_DEFECTO);
    EnviarComando(azul);  // B
    EnviarComando(verde); // G
    EnviarComando(rojo);  // R
	}
  StopCommunication();
  osDelay(100);

  TurnOff();

  StartCommunication();
  for (int i = 0; i < sizeof(g_leds)/sizeof(g_leds[0]); i++)
	{
    const uint8_t azul  = 0;
    const uint8_t verde = 255;
    const uint8_t rojo  = 255;

		EnviarComando(BRIGHTNESS_MASK | BRILLO_DEFECTO);
    EnviarComando(azul);  // B
    EnviarComando(verde); // G
    EnviarComando(rojo);  // R
	}
  StopCommunication();
  osDelay(100);

  TurnOff();
}

static void ProcesarPatronTableroPuesto(void)
{
  memset(g_leds, 0, sizeof(g_leds));

  StartCommunication();
  for (int i = 0; i < sizeof(g_leds)/sizeof(g_leds[0]); i++)
	{
    const uint8_t azul  = 255;
    const uint8_t verde = 255;
    const uint8_t rojo  = 255;

		EnviarComando(BRIGHTNESS_MASK | BRILLO_DEFECTO);
    EnviarComando(azul);  // B
    EnviarComando(verde); // G
    EnviarComando(rojo);  // R
	}
  StopCommunication();
  osDelay(100);

  TurnOff();

  StartCommunication();
  for (int i = 0; i < sizeof(g_leds)/sizeof(g_leds[0]); i++)
	{
    const uint8_t azul  = 255;
    const uint8_t verde = 255;
    const uint8_t rojo  = 255;

		EnviarComando(BRIGHTNESS_MASK | BRILLO_DEFECTO);
    EnviarComando(azul);  // B
    EnviarComando(verde); // G
    EnviarComando(rojo);  // R
	}
  StopCommunication();
  osDelay(100);

  TurnOff();

  StartCommunication();
  for (int i = 0; i < sizeof(g_leds)/sizeof(g_leds[0]); i++)
	{
    const uint8_t azul  = 255;
    const uint8_t verde = 255;
    const uint8_t rojo  = 255;

		EnviarComando(BRIGHTNESS_MASK | BRILLO_DEFECTO);
    EnviarComando(azul);  // B
    EnviarComando(verde); // G
    EnviarComando(rojo);  // R
	}
  StopCommunication();
  osDelay(100);

  TurnOff();
}

static void EnviarDatos(void)
{
	StartCommunication();

	for (int i = 0; i < sizeof(g_leds)/sizeof(g_leds[0]); i++)
	{
    const uint8_t azul  = g_leds[i].blue;
    const uint8_t verde = g_leds[i].green;
    const uint8_t rojo  = g_leds[i].red;

		EnviarComando(BRIGHTNESS_MASK | g_leds[i].brillo);
    EnviarComando(azul);  // B
    EnviarComando(verde); // G
    EnviarComando(rojo);  // R
	}

	StopCommunication();
}

static void TestLeds(void)
{
  memset(g_leds, 0, sizeof(g_leds));

  StartCommunication();
  for (int i = 0; i < sizeof(g_leds)/sizeof(g_leds[0]); i++)
	{
    const uint8_t azul  = 0;
    const uint8_t verde = 255;
    const uint8_t rojo  = 0;

		EnviarComando(BRIGHTNESS_MASK | BRILLO_DEFECTO);
    EnviarComando(azul);  // B
    EnviarComando(verde); // G
    EnviarComando(rojo);  // R
	}
  StopCommunication();
  osDelay(1000);

  TurnOff();
}

static void TurnOff(void)
{
  memset(g_leds, 0, sizeof(g_leds));
  StartCommunication();
  for (int i = 0; i < sizeof(g_leds)/sizeof(g_leds[0]); i++)
	{
    const uint8_t azul  = 0;
    const uint8_t verde = 0;
    const uint8_t rojo  = 0;

		EnviarComando(BRIGHTNESS_MASK | 0);
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

static void EnviarComando(unsigned char cmd)
{
  SPIdrv2->Send(&cmd, sizeof(cmd));
  osThreadFlagsWait(LED_TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
}

static void StopCommunication(void)
{
  for (int i = 0; i < 4; i++)
  {
    EnviarComando(STOP);
  }
}

void ARM_LedSPI_SignalEvent(uint32_t event)
{
  if (event & ARM_SPI_EVENT_TRANSFER_COMPLETE)
  {
    // Data Transfer completed
    osThreadFlagsSet(e_ledStripManagerThreadId, LED_TRANSFER_COMPLETE);
  }
  if (event & ARM_SPI_EVENT_MODE_FAULT)
  {
		//printf("[LedStrip::%s] ARM_SPI_EVENT_MODE_FAULT", __func__);
    // Master Mode Fault (SS deactivated when Master)
  }
  if (event & ARM_SPI_EVENT_DATA_LOST)
  {
		//printf("[LedStrip::%s] ARM_SPI_EVENT_DATA_LOST", __func__);
    // Data lost: Receive overflow / Transmit underflow
  }
}
