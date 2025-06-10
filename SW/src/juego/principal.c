#include "juego.h"
#include "tablero.h"
#include <stdio.h>
#include "principal.h"

///////////////////////////////////////////////////////////////////////////////
// MAIN
//

osThreadId_t e_principalThreadId;

/* Private */
static  void  Run(void *argument);

void  I2C_SignalEvent(uint32_t event);

/**************************************/

void PrincipalInitialize(void)
{
  e_principalThreadId = osThreadNew(Run, NULL, NULL);

  if ((e_principalThreadId == NULL))
  {
    printf("[position::%s] ERROR! osThreadNew [%d]\n", __func__, (e_principalThreadId == NULL));
  }
}

static void Run(void *argument)
{
	int32_t status;
	
	 AJD_Tablero tablero;

   inicializa (&tablero);

   //nuevoJuego (&tablero);
   
   //ejecutaPartida (&tablero);
	
  while (1)
  {
		
  }
}
