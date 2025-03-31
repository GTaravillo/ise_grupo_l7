#include <utils.h>

#include <stdio.h>
////////////////////////////////////////////////////////////////////////////
// sign (a)
//   Devuelve -1 si a < 0
//             1 si a > 0
//             0 si a == 0
//
int sign (int a)
{
    return (a < 0 ? -1:
            a > 0 ?  1: 0);
}
/////////////////////////////////////////////////////////////////////////////////////////
// strSegundos
//   Convierte segundos a string con formato MM:SS (MM: minutos, SS: segundos).
char* strSegundos (char* buff, uint16_t segundos)
{       
    sprintf (buff, "%02d:%02d", segundos/60, segundos%60);

    return buff;
}
