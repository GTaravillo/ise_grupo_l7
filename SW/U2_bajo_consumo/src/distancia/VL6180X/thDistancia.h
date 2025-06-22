#ifndef THDISTANCIA_H_
#define THDISTANCIA_H_

#include "vl6180x_api.h"

#define FLAG_EMPIEZA_DIS  0x02U
#define FLAG_PARA_DIS     0X04U

extern int ThDistancia(void);
extern VL6180x_RangeData_t Range;

extern osThreadId_t tid_ThDistancia;

/* Para probación
extern void dis_sim(void* argument);
extern int ThSimDis(void);
*/


#endif