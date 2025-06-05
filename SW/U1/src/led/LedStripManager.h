#ifndef __LED_STRIP_MANAGER_H
#define __LED_STRIP_MANAGER_H
/* ARM */
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
/* std */
#include <stdbool.h>

typedef enum {
  POSIBLE_MOVIMIENTO = 0,
  MOVIMIENTO_ILEGAL  = 1,
  CAPTURA            = 2,
  ESPECIAL           = 3,
  ACTUAL             = 4
} ETipoJugada;

typedef struct {
  bool        nuevaJugada;  // Poner a true para limpiar tablero y encender los LEDs hasta proxima nueva jugada
  ETipoJugada tipoJugada;   // Determina el color del LED
  uint8_t     posicion;     // [0, 63]
} LedStripMsg_t;

extern osMessageQueueId_t  e_ledStripMessageId;
extern osThreadId_t e_ledStripManagerThreadId;

void LedStripManagerInitialize(void);

#endif
