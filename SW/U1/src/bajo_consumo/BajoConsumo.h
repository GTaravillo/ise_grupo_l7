#ifndef __BAJO_CONSUMO_H
#define __BAJO_CONSUMO_H

#include "cmsis_os2.h"

#define ENTRADA_BAJO_CONSUMO 0x01
#define SALIDA_BAJO_CONSUMO  0x02
#define FLAGS_BAJO_CONSUMO SALIDA_BAJO_CONSUMO | ENTRADA_BAJO_CONSUMO

extern osThreadId_t e_bajoConsumoThreadId;

void BajoConsumoInitialize(void);

#endif  // __BAJO_CONSUMO_H
