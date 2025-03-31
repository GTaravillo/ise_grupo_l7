#pragma once

#include <tablero.h>
#include <stdint.h>
#include <time.h>

typedef enum {NO_SELECCION, ORIGEN_SELECCIONADO, DESTINO_SELECCIONADO} AJD_Seleccion;
typedef enum { NO_ENROQUE, ENROQUE_LARGO, ENROQUE_CORTO } AJD_Enroque;
// Tipo para representar el estado
typedef struct
{
   uint16_t       turno;                     // Cuántos turnos se han jugado ya
   uint16_t       segundos_blancas;
   uint16_t       segundos_negras;
   uint8_t        juegan_blancas      : 1; 
   uint8_t        negro_jaque         : 1; 
   uint8_t        blanco_jaque        : 1; 
   uint8_t        enroque_largo_blanco_invalidado  : 1;
   uint8_t        enroque_corto_blanco_invalidado  : 1;
   uint8_t        enroque_largo_negro_invalidado  : 1;
   uint8_t        enroque_corto_negro_invalidado  : 1;
   AJD_Enroque    enroque_efectuado   : 2; 
   uint8_t        turno_jugador       : 1; 
   AJD_Seleccion  casilla_seleccionada: 2; 
   AJD_CasillaPtr casilla_origen;  // casilla origen de la pieza a mover
   AJD_CasillaPtr casilla_destino; // casilla destino de la pieza a mover   
   uint8_t        fin_juego : 1;          // Salir del juego?
} AJD_Estado, *AJD_EstadoPtr;
/////////////////////////////////////////////////////////////////////
// INTERFAZ PUBLICA
//

void inicializa(AJD_TableroPtr tablero);
void nuevoJuego(AJD_TableroPtr tablero);
void ejecutaPartida (AJD_TableroPtr tablero);
void actualizaJuego (AJD_TableroPtr tablero);
void menu();
void liberaRecursos();
