#include "tablero.h"

#include <stdint.h>

#define Algebr_MAX_ENTRIES 1000

typedef struct 
{
    AJD_CasillaPtr  origen;
    AJD_CasillaPtr  destino;
    uint8_t         enroque_largo:1;
    uint8_t         enroque_corto:1;
    uint8_t         come_pieza:1;
    uint8_t         jaque:1;
    uint8_t         mate:1;
} AJD_Jugada, *AJD_JugadaPtr;

extern char* strCasillas[];

/////////////////////////////////////////////////////////////////////////////////////////
// toAlgebrString
//  Convierte la información de una jugada a formato Algebr
//
void toAlgebraString (AJD_JugadaPtr jugada, char* buff);
