#ifndef __POSITION_MANAGER_H
#define __POSITION_MANAGER_H

#include <stdint.h>
#include "cmsis_os2.h"

#define HALL_DETECTED 20  // Maximum number of different states the leds can take

extern osThreadId_t e_positionManagerThreadId;

void PositionManagerInitialize(void);

typedef struct {
    uint8_t casilla;  // Número de la casilla de ajedrez, por ejemplo: 54 para e4
} ECasilla;

#endif