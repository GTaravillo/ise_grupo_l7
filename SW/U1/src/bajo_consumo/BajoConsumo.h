#ifndef __BAJO_CONSUMO_H
#define __BAJO_CONSUMO_H

#include "cmsis_os2.h"

#define ENTRADA_BAJO_CONSUMO 0x80U
#define SALIDA_BAJO_CONSUMO  0x100U
#define FLAGS_BAJO_CONSUMO SALIDA_BAJO_CONSUMO | ENTRADA_BAJO_CONSUMO

extern osThreadId_t e_bajoConsumoThreadId;

void BajoConsumoInitialize(void);

#endif  // __BAJO_CONSUMO_H
