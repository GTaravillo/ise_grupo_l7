#include "CommonDefines.h"

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