#ifndef __PRINCIPAL_H
#define __PRINCIPAL_H

#include "cmsis_os2.h"

extern osThreadId_t e_principalThreadId;

void PrincipalInitialize(void);


#endif