#include <notacion.h>

int main ()
{
    AJD_Casilla origen  = { NEGRO, CABALLO, BLANCO, b5 };
    AJD_Casilla destino = { NEGRO, CABALLO, BLANCO, d6 };
    AJD_Jugada blancas = {
        &origen, &destino, 
        0, 0, 0, 0, 0
    };
    AJD_Casilla origenN  = { NEGRO, TORRE, BLANCO, f5 };
    AJD_Casilla destinoN = { NEGRO, TORRE, BLANCO, f6 };
    AJD_Jugada negras = {
        &origenN, &destinoN, 
        0, 0, 0, 1, 1        
    };

    char buffB[10] = "";
    char buffN[10] = "";
    toAlgebraString (&blancas, buffB);
    toAlgebraString (&negras, buffN);

    uint16_t turno = 25;

    char buff[20] = "";

    sprintf (buff, "\033[32;1m%d. %s\t%s\033[0m", turno, buffB, buffN);

    puts (buff);
}
