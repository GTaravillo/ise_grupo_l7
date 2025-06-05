#include <stdio.h>
#include <string.h>
#include "juego.h"
#include "tablero.h"
#include "movimiento.h"
#include "prediction.h"
#include "../memoria/Memoria.h"
#include "cmsis_os2.h"
#include "../com_placas/ComunicacionPlacas.h"
#include "../posicion/PositionManager.h"

#include <time.h>

typedef enum{
  Init,
  Espera,
  Lectura,
  Idle,
  LevantaPieza,
  CompMov,
  //MovCrct,
  //Error
} modo_t;

////////////////////////////////////////////////////////////////////////////
// FUNCIONES PRIVADAS forward declarations
void _colocaPiezas (AJD_TableroPtr tablero, uint8_t* map);
//void actualizaCrono ();
void newGameMap(void);
uint8_t convertNum(uint8_t n);

////////////////////////////////////////////////////////////////////////////
// VARIABLES PRIVADAS AL M�DULO
AJD_Estado estado_juego;   // Estado del juego
static time_t crono;       // Temporizador para contar tiempo
modo_t modo = Init;
osMessageQueueId_t  e_ConsultPosition;
osMessageQueueId_t  e_ConsultStatus;
osMessageQueueId_t  e_PiezaLevantada;
osMessageQueueId_t  e_juegoTxMessageId;
osMessageQueueId_t  e_juegoRxMessageId;
PAQ_status paq;
uint8_t* map;
uint8_t firstRound = 1;

osStatus_t status;
 osStatus_t flag;
 AJD_Tablero tablero;
 
 ECasilla movedCasilla;
 uint8_t movedId;
 uint8_t predict[64];
 uint64_t predict_64b = 0;
 AJD_CasillaPtr tPromo;
////////////////////////////////////////////////////////////////////////////
// INTERFAZ P�BLICA

////////////////////////////////////////////////////////////////////////////
// M�QUINA DE ESTATOS
void stateMachine(void)
{
 
 

 while(true){
   switch(modo){
      case Init:
         flag = osThreadFlagsWait(FLAG_START | FLAG_RETOCAR, osFlagsWaitAny, osWaitForever);
         memset(map, 0, 64*sizeof(uint8_t));
         //e_ConsultPosition = osMessageQueueNew(5, sizeof(map), NULL);
         //e_ConsultStatus = osMessageQueueNew(5, sizeof(estado), NULL);
         //e_PiezaLevantada = osMessageQueueNew(5, sizeof(uint8_t), NULL);
         e_juegoTxMessageId = osMessageQueueNew(5, sizeof(uint64_t), NULL);
         e_juegoRxMessageId = osMessageQueueNew(5, sizeof(uint8_t), NULL);
         memset(predict,0,8*8*sizeof(uint8_t));
         memset(&estado_juego, 0, sizeof(AJD_Estado));
         inicializa(&tablero);
         modo = Espera;
      break;
      case Espera:
         if(flag == FLAG_RETOCAR){
            //Flagset Placeholder
            status = osMessageQueueGet(e_memoriaTxMessageId, &paq, NULL, osWaitForever); //place holder for the consult of positions
            map = paq.map;
            estado_juego.juegan_blancas = !paq.turno_color;
         }else if(flag == FLAG_START){
            newGameMap();
            nuevoJuego(&tablero);
            estado_juego.juegan_blancas = 1;
         }
         modo = Lectura;
      break;
      case Lectura:
         _colocaPiezas(&tablero, map);
         modo = Idle;
      break;
      case Idle:
         if(!firstRound){
            estado_juego.juegan_blancas = !estado_juego.juegan_blancas;
         }else{
            firstRound = 0;
         }
         modo = LevantaPieza;
      break;
      case LevantaPieza:
         status = osMessageQueueGet(e_positionMessageId, &movedCasilla, NULL, osWaitForever);
         if(status == osOK){
            movedId = convertNum(movedCasilla.casilla);
            estado_juego.casilla_origen = &(tablero.casilla[movedId]);
            estado_juego.casilla_seleccionada = ORIGEN_SELECCIONADO;
            
            // movInfo.srcY = movedId/8;
            // movInfo.srcX = movedId%8;
            // movInfo.origen = &(tablero->casillas[movInfo.srcY*8+movInfo.srcX]);
            predictPosition(&tablero, estado_juego.casilla_origen, predict);
         }
         for(int i=0; i<64; i++){
            predict_64b |= 0x01<<i ? predict[i]==1 : 0;
         } 
         status = osMessageQueuePut(e_juegoTxMessageId, &predict_64b, 1, 0);
         if(status == osOK) modo = CompMov;
      break;
      case CompMov:
         status = osMessageQueueGet(e_positionMessageId, &movedCasilla, NULL, osWaitForever);
         movedId = convertNum(movedCasilla.casilla);
         if(status == osOK){
            estado_juego.casilla_destino = &(tablero.casilla[movedId]);
            estado_juego.casilla_seleccionada = DESTINO_SELECCIONADO;
            if(esMovimientoValido(&tablero, &estado_juego)){
               
               if(peonUltimaFila(&tablero, &estado_juego)){
								  tPromo = estado_juego.casilla_destino;
									muevePieza(&tablero, &estado_juego);
                  promocionaPeon(&tablero, tPromo);
               }else{
									muevePieza(&tablero, &estado_juego);
							 }
            }else{
               status = osThreadFlagsSet(e_comPlacasRxThreadId, FLAG_ERROR_MOV);
            }
            modo = Idle;
            // movInfo.dstY = movedId/8;
            // movInfo.dstX = movedId%8;
            // movInfo.destino = &(tablero->casillas[movInfo.dstY*8+movInfo.dstX]);
         }
      break;

   }
 }
}
////////////////////////////////////////////////////////////////////////////
// INICIALIZA
//
// Pone todas las casillas del tablero a su estado inicial:
//    - Todas las casillas est�n vac�as
//    - El color de cada casilla alterna entre blanco y negro y la
//      casilla inferior derecha es de color blanco
//
void inicializa(AJD_TableroPtr tablero)
{
   AJD_Color color = NEGRO;
   AJD_idCasilla id = 0;

   for (int i=0; i<8; i++) 
   {      
      color ^= 1; // Alterna entre blanco/negro       
      for (int j=0; j<8; j++)    
      {
         // puntero a casilla actual, mejora legibilidad codigo
         AJD_CasillaPtr casilla = &(tablero->casilla[i*8+j]);

         casilla->color = color;
         color ^= 1; // Alterna entre blanco/negro       
         
         // Inicialmente el tablero est� vac�o
         // El color de la pieza cuando la casilla est� vac�a es irrelevante
         casilla->pieza = NONE;
         casilla->color_pieza = BLANCO;
         casilla->id = id++;
      }
   }
   //printf("sizeof(AJD_Tablero) = %ld\n", sizeof (AJD_Tablero));

}

////////////////////////////////////////////////////////////////////////////
// nuevoJuego
//
// Prepara el estado del juego y el tablero para una partida nueva:
//
//    - Estado del juego: ninguna pieza movida, ning�n rey en jaque,
//      turno del jugador 1, juegan blancas
//
//    - Coloca las piezas en el tablero para una partida nueva
//
//    - Turnos jugados = 0
//
void nuevoJuego(AJD_TableroPtr tablero)					// IMPLEMENTACI�N NFC
{
   // Estado del juego
   memset(&estado_juego, 0, sizeof (AJD_Estado));
   estado_juego.juegan_blancas = 1;

   // Turno
   // estado_juego.turno = 1;

   // Restablecer tiempos de ambos jugadores 
   //estado_juego.segundos_blancas = 0;
   //estado_juego.segundos_negras  = 0;

   // Coloca las piezas
   //_colocaPiezas_ini (tablero);

   // El cursor movil y de pieza seleccionada se posicionan a d2
   // El cursor movil es visible y sin flash
   // El cursor de pieza seleccionada no es visible y con flash
//   tablero->cursorMovil.casilla = tablero->cursorPiezaSeleccionada.casilla = &tablero->casilla[d2];   
//   tablero->cursorMovil.visible = 1;
//   tablero->cursorPiezaSeleccionada.visible = 0;
//   tablero->cursorMovil.flash = 0;
//   tablero->cursorPiezaSeleccionada.flash = 1;
}
////////////////////////////////////////////////////////////////////////////
// actualizaJuego
//
// Actualiza el estado del juego
void actualizaJuego (AJD_TableroPtr tablero)
{
   //actualizaCrono ();
	
   switch (estado_juego.casilla_seleccionada)
   {   
   case ORIGEN_SELECCIONADO:
      if (!hayPiezaValida(tablero, estado_juego.casilla_origen, &estado_juego))
      {
         estado_juego.casilla_origen = NULL;
         estado_juego.casilla_seleccionada = NO_SELECCION;
      }
      else
      {
         // Casilla origen seleccionada, muestra el cursor fijo
//         tablero->cursorPiezaSeleccionada.visible = 1;
      }
      break;

   case DESTINO_SELECCIONADO:
      if (esMovimientoValido (tablero, &estado_juego))
      {
         if (estado_juego.enroque_efectuado)
            efectuaEnroque (tablero, &estado_juego);
         else
            muevePieza (tablero, &estado_juego);

         if (peonUltimaFila (tablero, &estado_juego))
            promocionaPeon (tablero, estado_juego.casilla_destino);

         //estado_juego.turno_jugador ^= 1;
         estado_juego.juegan_blancas ^= 1;
         estado_juego.casilla_seleccionada = NO_SELECCION;
         //estado_juego.turno += estado_juego.juegan_blancas;
         estado_juego.casilla_origen = estado_juego.casilla_destino = NULL;
         estado_juego.enroque_efectuado = NO_ENROQUE;

         // movimiento efectuado, oculta el cursor fijo
//         tablero->cursorPiezaSeleccionada.visible = 0;

         // Restablece contadores de tiempo
         time (&crono);
      }
      else
      {
         estado_juego.casilla_destino = NULL;
         estado_juego.casilla_seleccionada = ORIGEN_SELECCIONADO;
      }
      break;

   default:
      break;
   }
	 printf("%d", (uint8_t)(estado_juego.casilla_destino->id));

}

////////////////////////////////////////////////////////////////////////////
//
// Bucle principal, se ejecuta hasta que termina la partida.
//

void ejecutaPartida (AJD_TableroPtr tablero)
{
   while (!estado_juego.fin_juego)
   {
      actualizaJuego (tablero);
   }
}



////////////////////////////////////////////////////////////////////////////
// INTERFAZ PRIVADA
////////////////////////////////////////////////////////////////////////////
// _colocaPiezas
//
// Dispone las piezas en el tablero para una partida nueva
//
// void _colocaPiezas_ini(AJD_TableroPtr tablero)
// {   
//   AJD_Pieza piezasMayores[8] = { TORRE, CABALLO, ALFIL, DAMA, REY, ALFIL, CABALLO, TORRE };
//   for (int col=0; col < 8; col++)
//   {

//      tablero->casilla[col].pieza               = piezasMayores[col]; // fila 1: piezas mayores negras
//      tablero->casilla[col].color_pieza         = NEGRO;

//      tablero->casilla[7*8 + col].pieza         = piezasMayores[col]; // fila 8: piezas mayores blancas
//      tablero->casilla[7*8 + col].color_pieza   = BLANCO;

//      tablero->casilla[8 + col].pieza           = PEON;               // fila 2: peones negros
//      tablero->casilla[8 + col].color_pieza     = NEGRO;
     
//      tablero->casilla[6*8 + col].pieza         = PEON;               // fils 7: peones blancos
//      tablero->casilla[6*8 + col].color_pieza   = BLANCO;          
//   }
// }

void _colocaPiezas(AJD_TableroPtr tablero, uint8_t* map )
{
 osStatus_t status;
 uint8_t pos;
 uint8_t placeHolder;
 uint8_t position;
 uint8_t k1 = 0;
 uint8_t k2 = 0;
 //AJD_Pieza piezasMayores[8] = { TORRE, CABALLO, ALFIL, DAMA, REY, ALFIL, CABALLO, TORRE };
 for (int i = 0; i < 32; i++){
   status = osMessageQueueGet(e_juegoTxMessageId, &pos, NULL, osWaitForever);
   if(status == osOK && pos != 0){
      // do{
      //    if(k2 < 7){
      //       k2++;
      //    }else{
      //       k1++;
      //       k2=0;
      //    }
      // }while(placeHolder != pos && k1*k2 < 49);
      for (int i=0; i<64; i++) {
         if(map[i] == pos){
            map[i] = 0;
            tablero->casilla[i].pieza = pos & 0x0F;
            tablero->casilla[i].color_pieza = (pos & 0x80) >> 7;
            break;
         }else{
            //status = osThreadFlagsSet()
            printf("[Error] Pieza no encontrado\n");
         }
      }
      // if(placeHolder == pos){
      //    position = 8*i+(8-j);
      //    map[i] &= !(0X02 << j);
      //    tablero->casilla[position].pieza = pos & 0x07;
      //    tablero->casilla[position].color_pieza = (pos & 0x08) >> 3;
      
   }
 }
}

//void actualizaCrono()
//{
//    // Actualizaci�n del cronometro
//   if (difftime (time(NULL), crono) >= 1.0)
//   {
//     if (estado_juego.juegan_blancas)
//         estado_juego.segundos_blancas++;
//     else
//         estado_juego.segundos_negras++;
//     time(&crono);
//   }
//}

void newGameMap(void)
{
   AJD_Pieza piezasMayores[8] = {TORRE1, CABALLO1, ALFIL1, DAMA, REY, ALFIL2, CABALLO2, TORRE2};
   AJD_Pieza piezaPeon[8] = {PEON1, PEON2, PEON3, PEON4, PEON5, PEON6, PEON7, PEON8};
   for (int col=0; col < 8; col++)
  {
      map[col] = (piezasMayores[col] | NEGRO);
      map[col + 7*8] = (piezasMayores[col] | BLANCO);

      map[col + 8] = (piezaPeon[col] | NEGRO);
      map[col + 6*8] = (piezaPeon[col] | BLANCO);        
  }

}

uint8_t convertNum(uint8_t n){
   uint8_t y = 0;
   uint8_t x = 0;
   uint8_t m;
   y = n / 8;
   x = n%7 ? y%2 == 0 : (7 - n%7);
   m = y * 7 + x;
   return m;
}
