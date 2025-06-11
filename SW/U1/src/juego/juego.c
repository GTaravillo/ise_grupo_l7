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
  Pause,
  Stop
} modo_t;

////////////////////////////////////////////////////////////////////////////
// FUNCIONES PRIVADAS forward declarations
void _colocaPiezas (AJD_TableroPtr tablero, uint8_t* map);
static void stateMachine(void* argument);
//void actualizaCrono ();
void newGameMap(void);
uint8_t convertNum(uint8_t n);
void juegoInitialize(void);
static void juegoTestBench(void* argument);
void printChessboard();
void juegoTbInitialize(void);
void esperaPausaDetener(void);
void juegoEsperaInitialize(void);

////////////////////////////////////////////////////////////////////////////
// VARIABLES PRIVADAS AL M�DULO
AJD_Estado estado_juego;   // Estado del juego
//static time_t crono;       // Temporizador para contar tiempo
modo_t modo = Init;
modo_t modot;
// osMessageQueueId_t  e_ConsultPosition;
// osMessageQueueId_t  e_ConsultStatus;
// osMessageQueueId_t  e_PiezaLevantada;
osMessageQueueId_t  e_juegoTxMessageId;
osMessageQueueId_t  e_juegoRxMessageId;

//osMessageQueueId_t e_positionMessageId;
//   //osMessageQueueId_t e_juegoTxMessageId;
//   //osMessageQueueId_t e_juegoRxMessageId;
//   osMessageQueueId_t e_memoriaTxMessageId;
//   osMessageQueueId_t e_memoriaRxMessageId;
osThreadId_t e_juegoThreadId;
osThreadId_t e_juegoEsperaThreadId;
osThreadId_t e_juegoTestbenchThreadId;
PAQ_status paq;
uint8_t* map;
uint8_t firstRound = 0;

osStatus_t status;
 osStatus_t flag;
 osStatus_t flagPause;
 AJD_Tablero tablero;
 
 uint8_t posCl[32];
 
 ECasilla movedCasilla;
 uint8_t movedId;
 uint8_t predict[64];
 uint64_t predict_64b = 0;
 AJD_CasillaPtr tPromo;
////////////////////////////////////////////////////////////////////////////
// INTERFAZ P�BLICA

////////////////////////////////////////////////////////////////////////////
// M�QUINA DE ESTATOS

//void juegoEsperaInitialize(void)
//{
//  e_juegoEsperaThreadId = osThreadNew(esperaPausaDetener, NULL, NULL);
//	

//  if ((e_juegoEsperaThreadId == NULL))
//  {
//    printf("[position::%s] ERROR! osThreadNew [%d]\n", __func__, (e_juegoEsperaThreadId == NULL));
//  }
//}


static void stateMachine(void* argument)
{
 
 

 while(true){

   flagPause = osThreadFlagsWait(FLAG_PAUSE | FLAG_STOP, osFlagsWaitAny, 50U);
      if(flagPause == FLAG_PAUSE){
         modot = modo;
         modo = Pause;
			}
      else if(flagPause == FLAG_STOP){
         modo = Stop;
      }

   switch(modo){
      case Init:
         flag = osThreadFlagsWait(FLAG_START | FLAG_RETOCAR, osFlagsWaitAny, osWaitForever);
         map = malloc(64*sizeof(uint8_t));
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
            memcpy(map, paq.map, 64 * sizeof(uint8_t));
            estado_juego.juegan_blancas = (uint8_t)paq.turno_color;
         }else if(flag == FLAG_START){
            
            newGameMap();
            nuevoJuego(&tablero);
            estado_juego.juegan_blancas = 1;
         }
         firstRound = 1;
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
				//esperaPausaDetener();
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
         predict_64b = 0;
         for(int i=0; i<64; i++){
            if (predict[i] == 1) {
               predict_64b |= (1ULL << i);
            }
         } 
         //status = osMessageQueuePut(e_juegoTxMessageId, &predict_64b, 1, 0);
         //if(status == osOK)
				 modo = CompMov;
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
               //status = osThreadFlagsSet(e_comPlacasRxThreadId, FLAG_ERROR_MOV);
               printf("[Error] Movimiento de pieza invalido\n");
            }
            modo = Idle;
            // movInfo.dstY = movedId/8;
            // movInfo.dstX = movedId%8;
            // movInfo.destino = &(tablero->casillas[movInfo.dstY*8+movInfo.dstX]);
         }
      break;
      case Pause:
         flag = osThreadFlagsWait(FLAG_RESUME, osFlagsWaitAny, osWaitForever);
         if (flag == FLAG_RESUME){
            modo = modot;
            modot = 0;
         }
      break;
      case Stop:
         memset(&paq, 0, sizeof(PAQ_status));
         for(int i = 0; i < 64; i++) paq.map[i] = tablero.casilla[i].pieza | (tablero.casilla[i].color_pieza << 7);
         paq.turno_color = estado_juego.juegan_blancas;
         status = osMessageQueuePut(e_memoriaRxMessageId, &paq, 1, 0);
         modo = Init;
      break;
   }
 }
 //osThreadYield();
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
         //time (&crono);
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
 uint8_t found = 0;
 //AJD_Pieza piezasMayores[8] = { TORRE, CABALLO, ALFIL, DAMA, REY, ALFIL, CABALLO, TORRE };
 for (int i = 0; i < 32; i++){
   status = osMessageQueueGet(e_juegoRxMessageId, &pos, NULL, osWaitForever);
	 posCl[i] = pos;
   if(status == osOK && pos != 0){
      found = 0;
      // do{
      //    if(k2 < 7){
      //       k2++;
      //    }else{
      //       k1++;
      //       k2=0;
      //    }
      // }while(placeHolder != pos && k1*k2 < 49);
      for (int j=0; j<64; j++) {
         if(map[j] == pos){
            map[j] = 0;
            tablero->casilla[j].pieza = (AJD_Pieza)(pos & 0x1F);
            tablero->casilla[j].color_pieza = (AJD_Color)((pos & 0x80) >> 7);
            found = 1;
            break;
         }
      }
      if(!found){
         printf("[Error] Pieza no encontrado\n");
         //i--;
      }
			pos = 0;
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
      map[col] = (piezasMayores[col] | BLACK);
      map[col + 7*8] = (piezasMayores[col] | WHITE);

      map[col + 8] = (piezaPeon[col] | BLACK);
      map[col + 6*8] = (piezaPeon[col] | WHITE);        
  }

}

uint8_t convertNum(uint8_t n){
   uint8_t y = 0;
   uint8_t x = 0;
   uint8_t m;
   y = n / 8;
   x = y%2 == 0 ? n%8 : (7 - n%8);
   m = y * 8 + x;
   return m;
}

void juegoInitialize(void)
{
  e_juegoThreadId = osThreadNew(stateMachine, NULL, NULL);
	

  if ((e_juegoThreadId == NULL))
  {
    printf("[position::%s] ERROR! osThreadNew [%d]\n", __func__, (e_juegoThreadId == NULL));
  }
}

void juegoTbInitialize(void)
{
  e_juegoTestbenchThreadId = osThreadNew(juegoTestBench, NULL, NULL);
	

  if ((e_juegoTestbenchThreadId == NULL))
  {
    printf("[position::%s] ERROR! osThreadNew [%d]\n", __func__, (e_juegoTestbenchThreadId == NULL));
  }
}

static void juegoTestBench(void* argument){



   
   //osMessageQueueId_t e_comPlacasRxThreadId;

   uint8_t tbPos[32];
   PAQ_status tbPaq;
   uint8_t tbMap[64];
   ECasilla tbCasilla;
   e_positionMessageId = osMessageQueueNew(10, sizeof(ECasilla), NULL);
   //e_juegoTxMessageId = osMessageQueueNew(10, sizeof(uint64_t), NULL);
   //e_juegoRxMessageId = osMessageQueueNew(10, sizeof(uint8_t), NULL);
   e_memoriaTxMessageId = osMessageQueueNew(10, sizeof(PAQ_status), NULL);
   e_memoriaRxMessageId = osMessageQueueNew(10, sizeof(PAQ_status), NULL);
   //tbPaq = malloc(sizeof(PAQ_status));

   osThreadFlagsSet(e_juegoThreadId, FLAG_START);
	osDelay(300);

   tbPos[0] = TORRE1 | WHITE;
   tbPos[1] = CABALLO1 | WHITE;
   tbPos[2] = ALFIL1 | WHITE;
   tbPos[3] = DAMA | WHITE;
   tbPos[4] = REY | WHITE;
   tbPos[5] = ALFIL2 | WHITE;
   tbPos[6] = CABALLO2 | WHITE;
   tbPos[7] = TORRE2 | WHITE;
    for (int i = 8; i < 16; i++) {
       tbPos[i] = (PEON1 + (i - 8)) | WHITE;
       tbPos[i+8] = (PEON1 + (i - 8)) | BLACK;
    }
    
    // 黑方
   tbPos[24] = TORRE1 | BLACK;
   tbPos[25] = CABALLO1 | BLACK;
   tbPos[26] = ALFIL1 | BLACK;
   tbPos[27] = DAMA | BLACK;
   tbPos[28] = REY | BLACK;
   tbPos[29] = ALFIL2 | BLACK;
   tbPos[30] = CABALLO2 | BLACK;
   tbPos[31] = TORRE2 | BLACK;

   // tbPaq.map = tbMap;
   // tbPaq.turno_color = 
   for(int i = 0; i < 32; i++){
      osMessageQueuePut(e_juegoRxMessageId, &tbPos[i], 1, osWaitForever);
      osDelay(100); // 10ms delay between sending each piece
   }
   printChessboard();
   osDelay(100);
   printf("[Test] basic move:\n");
   printf("  [Test] peon:\n");

   tbCasilla.casilla = 15; // a2位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 31; // a4位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);
   // 模拟用户操作：选择黑方兵前进
   tbCasilla.casilla = 49; // b7位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 33; // b5位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);

   tbCasilla.casilla = 14; // b2位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 17; // b3位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);

   printf("  [Test] captura\n");
   // 模拟用户操作：选择黑方兵前进
   tbCasilla.casilla = 33; // b5位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 31; // a4位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);

   
		
	tbCasilla.casilla = 255;
   osThreadFlagsSet(e_juegoThreadId, FLAG_STOP);
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
	osDelay(500);
   osThreadFlagsSet(e_juegoThreadId, FLAG_RETOCAR);
	osDelay(500);

   memset(tbMap, 0, 64*sizeof(uint8_t));

   tbMap[0] = TORRE1 | WHITE;
   tbMap[1] = CABALLO1 | WHITE;
   tbMap[2] = ALFIL1 | WHITE;
   tbMap[3] = DAMA | WHITE;
   tbMap[4] = REY | WHITE;
   tbMap[5] = ALFIL2 | WHITE;
   tbMap[6] = CABALLO2 | WHITE;
   tbMap[7] = TORRE2 | WHITE;
   // 黑方
   tbMap[56] = TORRE1 | BLACK;
   tbMap[57] = CABALLO1 | BLACK;
   tbMap[58] = ALFIL1 | BLACK;
   tbMap[59] = DAMA | BLACK;
   tbMap[60] = REY | BLACK;
   tbMap[61] = ALFIL2 | BLACK;
   tbMap[62] = CABALLO2 | BLACK;
   tbMap[63] = TORRE2 | BLACK;

   //tbPaq.map = tbMap;
	memcpy(tbPaq.map, tbMap, 64 * sizeof(uint8_t));
   tbPaq.turno_color = 1;
   osMessageQueuePut(e_memoriaTxMessageId, &tbPaq, 1, osWaitForever);
   for(int i = 0; i < 32; i++){
      osMessageQueuePut(e_juegoRxMessageId, &tbPos[i], 1, osWaitForever);
      osDelay(100); // 10ms delay between sending each piece
   }

   printf("  [Test] torre:\n");
   tbCasilla.casilla = 0; // a1位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 31; // a4位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);
   // 模拟用户操作：选择黑方兵前进
   tbCasilla.casilla = 63; // a8位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 32; // a5位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);

   printf("  [Test] caballo:\n");
   tbCasilla.casilla = 1; // b1位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 16; // a2位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);
   // 模拟用户操作：选择黑方兵前进
   tbCasilla.casilla = 62; // b8位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 47; // a6位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);

   printf("  [Test] alfil:\n");
   tbCasilla.casilla = 2; // b1位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 40; // a2位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);
   // 模拟用户操作：选择黑方前进
   tbCasilla.casilla = 61; // b8位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 23; // a6位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);

   printf("  [Test] dama:\n");
   tbCasilla.casilla = 3; // b1位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 25; // a2位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);
   // 模拟用户操作：选择黑方前进
   tbCasilla.casilla = 60; // b8位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 38; // a6位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);

   tbCasilla.casilla = 25; // b1位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 28; // a2位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);
   // 模拟用户操作：选择黑方前进
   tbCasilla.casilla = 38; // b8位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 35; // a6位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);

   printf("  [Test] rey:\n");
   tbCasilla.casilla = 4; // b1位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 11; // a2位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);
   // 模拟用户操作：选择黑方前进
   tbCasilla.casilla = 59; // b8位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 52; // a6位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);

   tbCasilla.casilla = 255;
   osThreadFlagsSet(e_juegoThreadId, FLAG_STOP);
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
	osDelay(500);
   osThreadFlagsSet(e_juegoThreadId, FLAG_RETOCAR);
	osDelay(500);

   memset(tbMap, 0, 64*sizeof(uint8_t));

   tbMap[11] = PEON1 | WHITE;

   //tbPaq.map = tbMap;
	memcpy(tbPaq.map, tbMap, 64 * sizeof(uint8_t));
   tbPaq.turno_color = 1;
   osMessageQueuePut(e_memoriaTxMessageId, &tbPaq, 1, osWaitForever);
   for(int i = 0; i < 32; i++){
      osMessageQueuePut(e_juegoRxMessageId, &tbPos[i], 1, osWaitForever);
      osDelay(100); // 10ms delay between sending each piece
   }
   printChessboard();
	 osDelay(100);
	 
   printf("  [Test] Promoción:\n");
   tbCasilla.casilla = 12; // b8位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 3; // a6位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);

   tbCasilla.casilla = 3; // b8位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 39; // a6位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);

   tbCasilla.casilla = 39; // b8位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);

   tbCasilla.casilla = 32; // a6位置
   osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   osDelay(500);
   printChessboard();
   osDelay(100);

   printf("  [Test] End\n");



  


   // 模拟兵升变
   // movedCasilla.casilla = 15; // h2位置
   // osMessageQueuePut(e_positionMessageId, &movedCasilla, 0, 0);
   // osDelay(100);
   
   // movedCasilla.casilla = 55; // h7位置
   // osMessageQueuePut(e_positionMessageId, &movedCasilla, 0, 0);
   // osDelay(100);



}


/**
 * 打印棋盘函数
 * 棋盘使用8x8的二维数组表示，每个位置显示对应的棋子或空位
 */
 
void printChessboard() {
    // 棋子符号映射表
    const char* pieceSymbolsWhite[] = {
        "  ",    // NONE
        "♙",     // PEON1
        "♙",     // PEON2
        "♙",     // PEON3
        "♙",     // PEON4
        "♙",     // PEON5
        "♙",     // PEON6
        "♙",     // PEON7
        "♙",     // PEON8
        "♖",     // TORRE1
        "♖",     // TORRE2
        "♘",     // CABALLO1
        "♘",     // CABALLO2
        "♗",     // ALFIL1
        "♗",     // ALFIL2
        "♕",     // DAMA
        "♔"      // REY
    };

    const char* pieceSymbolsBlack[] = {
        "  ",    // NONE
        "♟",     // PEON1
        "♟",     // PEON2
        "♟",     // PEON3
        "♟",     // PEON4
        "♟",     // PEON5
        "♟",     // PEON6
        "♟",     // PEON7
        "♟",     // PEON8
        "♜",     // TORRE1
        "♜",     // TORRE2
        "♞",     // CABALLO1
        "♞",     // CABALLO2
        "♝",     // ALFIL1
        "♝",     // ALFIL2
        "♛",     // DAMA
        "♚"      // REY
    };

    int index;
    AJD_Pieza piece;
    AJD_Color color;
		char* symbol;
    
    printf("     a   b   c   d   e   f   g   h\n");
    printf("   +---+---+---+---+---+---+---+---+\n");
    
    for (int row = 0; row < 8; row++) {
        printf("  %d|", 8 - row); // 行号从8到1
        
        for (int col = 0; col < 8; col++) {
            index = row * 8 + col;
            piece = tablero.casilla[index].pieza; // 获取棋子类型
            color = tablero.casilla[index].color_pieza;// 获取颜色
            
            // 根据颜色调整棋子符号（这里假设符号已经区分颜色，实际可能需要调整）
            if (color ==BLACK) {
               symbol = pieceSymbolsBlack[piece];
            }else{
              symbol = pieceSymbolsWhite[piece];
            }
            
            printf(" %s|", symbol);
        }
        
        printf("\n   +---+---+---+---+---+---+---+---+\n");
    }
    
    printf("     a   b   c   d   e   f   g   h\n");
}

