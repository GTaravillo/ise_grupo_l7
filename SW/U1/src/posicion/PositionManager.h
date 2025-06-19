#ifndef __POSITION_MANAGER_H
#define __POSITION_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"

#define HALL_DETECTED_1 0x01  // Flag ligada al primer expansor (filas de casillas 1 y 2)
#define HALL_DETECTED_2 0x02  // Flag ligada al segundo expansor (filas de casillas 3 y 4)
#define HALL_DETECTED_3 0x04  // Flag ligada al tercer expansor (filas de casillas 5 y 6)
#define HALL_DETECTED_4 0x08  // Flag ligada al cuarto expansor (filas de casillas 7 y 8)

extern osThreadId_t e_positionManagerThreadId;
extern osMessageQueueId_t  e_positionMessageId;

void PositionManagerInitialize(void);

typedef struct {
    uint8_t casilla;  // N�mero de la casilla de ajedrez, siguiendo el g_numeroHallMap del .c
    bool ocupada;
} ECasilla;

#endif