#ifndef __COMMONDEFINES_H
#define __COMMONDEFINES_H

/* ARM */
#include "stm32f4xx_hal.h"
/* std */
#include <stdio.h>
#include <string.h>

/**
 * @brief Devuelve la posicion en el tablero
 * @param position la posicion en términos de LED [0, 63]
 * @param outStr string en términos de casilla [A0, H8]
 * @return 1 si position < 63
 */
int PositionToString(uint8_t position, char* outStr)
{
    if (position > 63)
    {
        return 0;
    }

    uint8_t row = position / 8;
    uint8_t col = position % 8;
    outStr[0] = (row % 2 == 0) ? 'A' + col : 'H' - col;
    outStr[1] = '1' + row;
    outStr[2] = '\0';

    return 1;
}

#endif