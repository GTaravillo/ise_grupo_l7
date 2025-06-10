#include "notacion.h"
#include <string.h>

char* strCasillas[] = {
   "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
   "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
   "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
   "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
   "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
   "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
   "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
   "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};
/////////////////////////////////////////////////////////////////////////////////////////
// toAlgebrString
//  Convierte la información de una jugada a formato Algebr
//
void toAlgebraString (AJD_JugadaPtr jugada, char* buff)
{
    AJD_CasillaPtr origen = jugada->origen;
    AJD_CasillaPtr destino = jugada->destino;
    char* str_pieza = "  TCADR";
    AJD_Pieza pieza = origen->pieza;
    
    if (jugada->origen->pieza != PEON)
        *buff++ = str_pieza[pieza];

    strncpy (buff, strCasillas[origen->id], 2);

    buff += 2;

    *buff++ = jugada->come_pieza ? 'x' : '-';

    strncpy (buff, strCasillas[destino->id], 2);
    
    buff += 2;

    if (jugada->jaque || jugada->mate) *buff++ = '!';

    if (jugada->mate) *buff++ = '!';

    *buff = '\0';
}
