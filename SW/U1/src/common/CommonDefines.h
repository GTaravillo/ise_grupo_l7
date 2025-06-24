#ifndef __COMMONDEFINES_H
#define __COMMONDEFINES_H

/* ARM */
#include "stm32f4xx_hal.h"
/* std */
#include <stdio.h>
#include <string.h>

// Tamaños datos (bytes)
#define TAM_FECHA           8   // fecha juego (dd/mm/aa)
#define TAM_HORA            8   // hora juego (hh:mm:ss)
#define TAM_NOMBRE_JUGADOR 12   // nombre jugador
#define TAM_TIEMPO_JUGADOR  5   // tiempo restante jugador (mmss)
#define TAM_DATOS          64   // casillas tablero 

/**
 * @brief Devuelve la posicion en el tablero
 * @param position la posicion en términos de LED [0, 63]
 * @param outStr string en términos de casilla [A0, H8]
 * @return 1 si position < 63
 */
int PositionToString(uint8_t position, char* outStr);

#endif