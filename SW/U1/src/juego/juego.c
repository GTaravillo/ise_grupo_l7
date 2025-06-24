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
#include "../led/LedStripManager.h"
#include "../lcd/lcd.h"
#include <stdlib.h>

#include "../bajo_consumo/BajoConsumo.h"

#include <time.h>

typedef enum{
  Init,
  Espera,
  Lectura,
  Idle,
  LevantaPieza,
  CompMov,
  Pause,
  Stop,
  Win
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
void tick_segundos_callback(void *argument);
void checkJaque(AJD_TableroPtr tablero);
bool amenazaRey(AJD_CasillaPtr rey, AJD_TableroPtr tablero);

void setTiempoNuevaPartida(uint16_t minutos);

////////////////////////////////////////////////////////////////////////////
// VARIABLES PRIVADAS AL M�DULO
AJD_Estado estado_juego;   // Estado del juego
//static time_t crono;       // Temporizador para contar tiempo
modo_t modo = Init;

// osMessageQueueId_t  e_juegoTxMessageId;
osMessageQueueId_t  e_juegoRxMessageId;
osMutexId_t mapMutex;

 osThreadId_t e_juegoThreadId;
// osThreadId_t e_juegoEsperaThreadId;
 osThreadId_t e_juegoTestbenchThreadId;
osTimerId_t tick_segundos;
MemoriaMsg_t paq;
MemoriaMsg_t paqIn;
uint8_t* map;
//uint8_t mapa[64];
uint8_t firstRound = 0;

osStatus_t status;
osStatus_t flag;
osStatus_t flagPause;
AJD_Tablero tablero;
uint8_t predict[64];
lcdMessage_t lcdMsg;
LedStripMsg_t ledMsg;



 
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

void tick_segundos_callback(void *argument){
   if(estado_juego.juegan_blancas){
      estado_juego.segundos_blancas--;
      //setTiempoPartida((uint8_t)(estado_juego.segundos_blancas/60), (uint8_t)(estado_juego.segundos_blancas%60));
//      lcdMsg.printMsg.printLine = PRINT_LINE_1;
//      sprintf(lcdMsg.printMsg.msg, "BLANCO: %d : %d", estado_juego.segundos_blancas/60, estado_juego.segundos_blancas%60);
//      osMessageQueuePut(e_lcdInputMessageId, &lcdMsg, 1, 0);
      if(estado_juego.segundos_blancas == 0){
         estado_juego.segundos_blancas = 0;
         estado_juego.segundos_negras = 0;
         lcdMsg.printMsg.printLine = PRINT_LINE_1;
         sprintf(lcdMsg.printMsg.msg, "FIN DE LA PARTIDA");
         osMessageQueuePut(e_lcdInputMessageId, &lcdMsg, 1, 0);
         lcdMsg.printMsg.printLine = PRINT_LINE_2;
         sprintf(lcdMsg.printMsg.msg, "NEGRAS GANAN");
         osMessageQueuePut(e_lcdInputMessageId, &lcdMsg, 1, 0);
         modo = Win;
     }
   }else{
      estado_juego.segundos_negras--;
      //setTiempoPartida((uint8_t)(estado_juego.segundos_negras/60), (uint8_t)(estado_juego.segundos_negras%60));
//      lcdMsg.printMsg.printLine = PRINT_LINE_1;
//      sprintf(lcdMsg.printMsg.msg, "NEGRO: %d : %d", estado_juego.segundos_negras/60, estado_juego.segundos_negras%60);
//      osMessageQueuePut(e_lcdInputMessageId, &lcdMsg, 1, 0);
      if(estado_juego.segundos_negras == 0){
         estado_juego.segundos_negras = 0;
         estado_juego.segundos_blancas = 0;
         lcdMsg.printMsg.printLine = PRINT_LINE_1;
         sprintf(lcdMsg.printMsg.msg, "FIN DE LA PARTIDA");
         osMessageQueuePut(e_lcdInputMessageId, &lcdMsg, 1, 0);
         lcdMsg.printMsg.printLine = PRINT_LINE_2;
         sprintf(lcdMsg.printMsg.msg, "BLANCAS GANAN");
         osMessageQueuePut(e_lcdInputMessageId, &lcdMsg, 1, 0);
         modo = Win;
      }
   }
	//osThreadFlagsSet(tid_Thread_Principal, ACTUALIZAR_HORA);
}

static void stateMachine(void* argument)
{
 
char* tiempoBlancasMinStr;
char* tiempoBlancasSegStr;
char* tiempoNegrasMinStr;
char* tiempoNegrasSegStr;

modo_t modot;
MemoriaMsg_t paq;
MemoriaMsg_t paqIn;
ComPlacasMsg_t msg_dis;
uint8_t firstRound = 0;
osStatus_t status;
osStatus_t flag;
osStatus_t flagPause;
ECasilla movedCasilla;
uint8_t movedId;
AJD_CasillaPtr tPromo;

 while(true){

   flagPause = osThreadFlagsWait(FLAG_PAUSE | FLAG_STOP | FLAG_FINALIZAR, osFlagsWaitAny, 50U);
      if(flagPause == FLAG_PAUSE){
         modot = modo;
         modo = Pause;
			}
      else if(flagPause == FLAG_STOP){
          modo = Stop;
      }
      else if(flagPause == FLAG_FINALIZAR){
          modo = Win;
      }
      else if (flagPause == FLAG_FINALIZAR) {
        modo = Win;
      }

   switch(modo){
      case Init:
        printf("ESTAMOS EN MODO: INIT\n");
         flag = osThreadFlagsWait(FLAG_START | FLAG_RETOCAR, osFlagsWaitAny, osWaitForever);
         printf("RECIBIDO FLAG\n");
         map = malloc(64*sizeof(uint8_t));
         memset(map, 0, 64*sizeof(uint8_t));
         //e_ConsultPosition = osMessageQueueNew(5, sizeof(map), NULL);
         //e_ConsultStatus = osMessageQueueNew(5, sizeof(estado), NULL);
         //e_PiezaLevantada = osMessageQueueNew(5, sizeof(uint8_t), NULL);
        //  e_juegoTxMessageId = osMessageQueueNew(5, sizeof(uint64_t), NULL);
        //  e_juegoRxMessageId = osMessageQueueNew(32, sizeof(uint8_t), NULL);
         memset(predict,0,8*8*sizeof(uint8_t));
         memset(&estado_juego, 0, sizeof(AJD_Estado));
         memset(&lcdMsg, 0, sizeof(lcdMessage_t));
         inicializa(&tablero);
         lcdMsg.mode = PRINT_NORMAL;
         ledMsg.nuevaJugada  = false;
         tiempoBlancasMinStr = malloc(2*sizeof(char));
         tiempoNegrasMinStr  = malloc(2*sizeof(char));
         tiempoBlancasSegStr = malloc(2*sizeof(char));
         tiempoNegrasSegStr  = malloc(2*sizeof(char));
         modo = Espera;
      break;
      case Espera:
        printf("ESTAMOS EN MODO: ESPERA\n");
         if(flag == FLAG_RETOCAR){
            //Flagset Placeholder
//            status = osMessageQueueGet(e_memoriaTxMessageId, &paq, NULL, osWaitForever); //place holder for the consult of positions
//            memcpy(map, paq.dato, 64 * sizeof(uint8_t));
//            estado_juego.juegan_blancas = paq.turno_victoria;
//            memcpy(tiempoBlancasMinStr, paq.tiempoBlancas, 2*sizeof(char));
//            memcpy(tiempoNegrasMinStr, paq.tiempoNegras, 2*sizeof(char));
//            memcpy(tiempoBlancasSegStr, paq.tiempoBlancas+2, 2*sizeof(char));
//            memcpy(tiempoNegrasSegStr, paq.tiempoNegras+2, 2*sizeof(char));
//            estado_juego.segundos_blancas = atoi(tiempoBlancasMinStr)*60 + atoi(tiempoBlancasSegStr);
//            estado_juego.segundos_negras = atoi(tiempoNegrasMinStr)*60 + atoi(tiempoNegrasSegStr);
//            free(tiempoBlancasMinStr);
//            free(tiempoNegrasMinStr);
//            free(tiempoBlancasSegStr);
//            free(tiempoNegrasSegStr);
            
             
         }else if(flag == FLAG_START){
            
            newGameMap();
            nuevoJuego(&tablero);
            estado_juego.juegan_blancas = 1;
         }
         firstRound = 1;
         modo = Lectura;
      break;
      case Lectura:
        printf("ESTAMOS EN MODO: LECTURA\n");
         _colocaPiezas(&tablero, map);
         modo = Idle;
         break;
      case Idle:
        printf("ESTAMOS EN MODO: IDLE\n");
         if(!firstRound){
            estado_juego.juegan_blancas = !estado_juego.juegan_blancas;
            estado_juego.casilla_seleccionada = NO_SELECCION;
            // ledMsg.nuevaJugada = true;
            memset(predict, 0, 64*sizeof(uint8_t));
            // osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
          } else {
            osTimerStart(tick_segundos, 1000);
            firstRound = 0;
         }
       //setTurnoActual(estado_juego.turno_blancas == 1);
         modo = LevantaPieza;
         break;
      case LevantaPieza:
        printf("ESTAMOS EN MODO: LEVANTAPIEZA\n");
         //esperaPausaDetener();
         status = osMessageQueueGet(e_positionMessageId, &movedCasilla, NULL, 200);
         if(status == osOK && !movedCasilla.ocupada){
            movedId = convertNum(movedCasilla.casilla);
            estado_juego.casilla_origen = &(tablero.casilla[movedId]);
            estado_juego.casilla_seleccionada = ORIGEN_SELECCIONADO;
            
            // movInfo.srcY = movedId/8;
            // movInfo.srcX = movedId%8;
            // movInfo.origen = &(tablero->casillas[movInfo.srcY*8+movInfo.srcX]);
           printf("Color pieza: %d, Color turno: %d", estado_juego.casilla_origen->color_pieza, estado_juego.juegan_blancas);
            if(estado_juego.casilla_origen->color_pieza == estado_juego.juegan_blancas){
               predictPosition(&tablero, estado_juego.casilla_origen, predict);
            
               ledMsg.nuevaJugada = true;
               osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
               ledMsg.tipoJugada = ACTUAL;
               ledMsg.posicion = movedCasilla.casilla;
               ledMsg.nuevaJugada = false;
               osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
               //predict_64b = 0;
               for(int i=0; i<64; i++){
                  if (predict[i] == 1) {
                     //predict_64b |= (1ULL << i);
                     if(tablero.casilla[i].pieza == NONE){
                        ledMsg.tipoJugada = POSIBLE_MOVIMIENTO;
                        ledMsg.posicion = convertNum(i);
                        ledMsg.nuevaJugada = false;
                        osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
                     }else if(tablero.casilla[i].color_pieza != estado_juego.juegan_blancas){
                        ledMsg.tipoJugada = CAPTURA;
                        ledMsg.posicion = convertNum(i);
                        ledMsg.nuevaJugada = false;
                        osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
                     }
                     
                  }
               } 
               //status = osMessageQueuePut(e_juegoTxMessageId, &predict_64b, 1, 0);
               //if(status == osOK){
               modo = CompMov;
            }else{
               printf("[Error] Movimiento de pieza invalido: origen\n");
               ledMsg.nuevaJugada = true;
               osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
               ledMsg.nuevaJugada = false;
               ledMsg.tipoJugada = MOVIMIENTO_ILEGAL;
               ledMsg.posicion = movedCasilla.casilla;
               osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
            }
         }
         //}
      break;
      case CompMov:
        printf("ESTAMOS EN MODO: COMPMOV\n");
         status = osMessageQueueGet(e_positionMessageId, &movedCasilla, NULL, 200);
         if(status == osOK && movedCasilla.ocupada){
            //ledMsg.nuevaJugada = true;
            //osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
            movedId = convertNum(movedCasilla.casilla);
            estado_juego.casilla_destino = &(tablero.casilla[movedId]);
            estado_juego.casilla_seleccionada = DESTINO_SELECCIONADO;
            if(estado_juego.casilla_origen == estado_juego.casilla_destino){
               ledMsg.nuevaJugada = true;
               osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
              modo = LevantaPieza;
               break;
            }else if(esMovimientoValido(&tablero, &estado_juego)){
               if(estado_juego.casilla_destino->pieza == REY && estado_juego.casilla_destino->color_pieza != estado_juego.juegan_blancas){
                  modo = Win;
                  if(estado_juego.juegan_blancas) printf(" [Test] White Wins\n");
                  else if(!estado_juego.juegan_blancas) printf("  [Test] Black Wins\n");
                  modo = Win;
                 continue;
               }
               
               if(peonUltimaFila(&tablero, &estado_juego)){
						tPromo = estado_juego.casilla_destino;
						muevePieza(&tablero, &estado_juego);
                  promocionaPeon(&tablero, tPromo);
               }else{
						muevePieza(&tablero, &estado_juego);
					}
               ledMsg.nuevaJugada = true;
               osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
               checkJaque(&tablero);
               
               if(estado_juego.blanco_jaque == 1){
                  printf(" [Test] Jaque al blanco\n");
                  estado_juego.blanco_jaque = 0;

               }else if(estado_juego.negro_jaque == 1){
                 printf(" [Test] Jaque al negro\n");
                  estado_juego.negro_jaque = 0;
               } else {
                printf("Esperando flag distancia\n");
                osThreadFlagsWait(FLAG_SENSOR_DISTANCIA, osFlagsWaitAny, osWaitForever);
                printf("Recibido flag distancia\n");
                
                ComPlacasMsg_t msg = {
                  .remitente = MENSAJE_JUEGO,
                  .destinatario = MENSAJE_DISTANCIA,
                  .mensaje[0] = 0x04U
                };
                osMessageQueuePut(e_comPlacasTxMessageId, &msg, 1, 0);
                
                
                ledMsg.tipoJugada = ACK;
                osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
               }
               

            }else{
               //status = osThreadFlagsSet(e_comPlacasRxThreadId, FLAG_ERROR_MOV);
               printf("[Error] Movimiento de pieza invalido: destino\n");
               ledMsg.nuevaJugada = false;
               ledMsg.tipoJugada = MOVIMIENTO_ILEGAL;
               ledMsg.posicion = movedCasilla.casilla;
               osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
            }
            
            //flag = osThreadFlagsWait(FLAG_TURN, osFlagsWaitAny, 200);
            //modo = flag == FLAG_TURN ? Idle : modo;
            modo = Idle;
            // movInfo.dstY = movedId/8;
            // movInfo.dstX = movedId%8;
            // movInfo.destino = &(tablero->casillas[movInfo.dstY*8+movInfo.dstX]);
         }
      break;
      case Pause:
        printf("ESTAMOS EN MODO: PAUSE\n");
			osTimerStop(tick_segundos);
         flag = osThreadFlagsWait(FLAG_RESUME, osFlagsWaitAny, osWaitForever);
         if (flag == FLAG_RESUME){
            modo = modot;
            modot = 0;
					 osTimerStart(tick_segundos, 1000);
         }
      break;
      case Stop:
        printf("ESTAMOS EN MODO: STOP\n");
			osTimerStop(tick_segundos);
      // const char* mapa_prueba = GetMap();
//         memset(&paqIn, 0, sizeof(MemoriaMsg_t));
//         for(int i = 0; i < 64; i++) paqIn.dato[i] = (tablero.casilla[i].pieza - 1) | (tablero.casilla[i].color_pieza << 4);
//         paqIn.tiempoBlancas[0] = (estado_juego.segundos_blancas/60)/10 + '\0';
//         paqIn.tiempoBlancas[1] = (estado_juego.segundos_blancas/60)%10 + '\0';
//         paqIn.tiempoBlancas[2] = (estado_juego.segundos_blancas%60)/10 + '\0';
//         paqIn.tiempoBlancas[3] = (estado_juego.segundos_blancas%60)%10 + '\0';
//         paqIn.tiempoNegras[0] = (estado_juego.segundos_negras/60)/10 + '\0';
//         paqIn.tiempoNegras[1] = (estado_juego.segundos_negras/60)%10 + '\0';
//         paqIn.tiempoNegras[2] = (estado_juego.segundos_negras%60)/10 + '\0';
//         paqIn.tiempoNegras[3] = (estado_juego.segundos_negras%60)%10 + '\0';
//         paqIn.turno_victoria = estado_juego.juegan_blancas;
//         paqIn.tipoPeticion = GUARDAR_PARTIDA_SIN_FINALIZAR;
//         status = osMessageQueuePut(e_memoriaRxMessageId, &paqIn, 1, 0);
         modo = Init;
      break;
      case Win:
        printf("ESTAMOS EN MODO: WIN\n");
         osTimerStop(tick_segundos);
//         memset(&paqIn, 0, sizeof(MemoriaMsg_t));
//         for(int i = 0; i < 64; i++) paqIn.dato[i] = (tablero.casilla[i].pieza - 1) | (tablero.casilla[i].color_pieza << 4);
//         paqIn.tiempoBlancas[0] = (estado_juego.segundos_blancas/60)/10 + '\0';
//         paqIn.tiempoBlancas[1] = (estado_juego.segundos_blancas/60)%10 + '\0';
//         paqIn.tiempoBlancas[2] = (estado_juego.segundos_blancas%60)/10 + '\0';
//         paqIn.tiempoBlancas[3] = (estado_juego.segundos_blancas%60)%10 + '\0';
//         paqIn.tiempoNegras[0] = (estado_juego.segundos_negras/60)/10 + '\0';
//         paqIn.tiempoNegras[1] = (estado_juego.segundos_negras/60)%10 + '\0';
//         paqIn.tiempoNegras[2] = (estado_juego.segundos_negras%60)/10 + '\0';
//         paqIn.tiempoNegras[3] = (estado_juego.segundos_negras%60)%10 + '\0';
//         paqIn.turno_victoria = estado_juego.juegan_blancas;
//         paqIn.tipoPeticion = GUARDAR_PARTIDA_FINALIZADA;
//         status = osMessageQueuePut(e_memoriaRxMessageId, &paqIn, 1, 0);
      
         ledMsg.tipoJugada = NACK;
         osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
         modo = Init;
         //osThreadFlagsSet(e_serverThreadId, FLAG_WIN);
      break;
   }
 }
 //osThreadYield();
}

void setMap(const uint8_t* mapIn, size_t len) {
    if (len > 64) len = 64; // Prevent overflow
    osMutexAcquire(mapMutex, osWaitForever);
    memcpy(map, mapIn, len);
    osMutexRelease(mapMutex);
}

void getMap(uint8_t* mapOut, size_t len) {
    uint8_t map[64];
    if (len > 64) len = 64;
    for(int i = 0; i < 64; i++) {
      map[i] = (tablero.casilla[i].pieza - 1) | (tablero.casilla[i].color_pieza << 4);
      printf("Guarda pieza:%d %s como [%x]", tablero.casilla[i].pieza-1, tablero.casilla[i].color_pieza?"negra":"blanca", map[i]);
    }
    osMutexAcquire(mapMutex, osWaitForever);
    memcpy(mapOut, map, len);
    osMutexRelease(mapMutex);
}

uint8_t GetMinutosBlancas(void)
{
  uint8_t minutosblancas = (uint8_t)(estado_juego.segundos_blancas/60);
  printf("MINUTOS BLANCAS QUE SE ENVIAN A WEB: %d\n", minutosblancas);
  return (uint8_t)(estado_juego.segundos_blancas/60);
}

uint8_t GetSegundosBlancas(void)
{
  uint8_t segundosblancas = (uint8_t)(estado_juego.segundos_blancas%60);
  printf("SEGUNDOS BLANCAS QUE SE ENVIAN A WEB: %d\n", segundosblancas);
  printf("segundos bien blancas: %d\n", estado_juego.segundos_blancas);
  return (uint8_t)(estado_juego.segundos_blancas%60);
}

uint8_t GetMinutosNegras(void)
{
  uint8_t minutosnegras = (uint8_t)(estado_juego.segundos_negras/60);
  printf("segundos bien negras: %d\n", estado_juego.segundos_negras);

  printf("MINUTOS NEGRAS QUE SE ENVIAN A WEB: %d\n", minutosnegras);
  return (uint8_t)(estado_juego.segundos_negras/60);
}

uint8_t GetSegundosNegras(void)
{
  uint8_t segundosnegras = (uint8_t)(estado_juego.segundos_negras%60);
  printf("SEGUNDOS NEGRAS QUE SE ENVIAN A WEB: %d\n", segundosnegras);
  return (uint8_t)(estado_juego.segundos_negras%60);
}

void SetTurno(const bool turnoBlancas)
{
  estado_juego.juegan_blancas = turnoBlancas ? 1 : 0;
  printf("TURNO RETOCADO DESDE WEB %s\n", turnoBlancas ? "BLANCAS" : "NEGRAS");
  printf("TURNO GUARDADO %s\n", estado_juego.juegan_blancas ? "BLANCAS" : "NEGRAS");


}

bool GetTurno(void)
{
  bool turno = (estado_juego.juegan_blancas == 1);
  printf("TURNO QUE SE ENVIA %s\n", turno ? "BLANCAS" : "NEGRAS");
  return (estado_juego.juegan_blancas == 1);
}

void setTiempoBlancas(uint8_t minutos, uint8_t segundos)
{
  estado_juego.segundos_blancas = minutos * 60 + segundos;
  printf("TIEMPO BLANCAS RETOCADO DESDE WEB: minutos %d, segundos %d\n", minutos, segundos);
  printf("TIEMPO GUARDADO: segundos totales %d\n", estado_juego.segundos_blancas);


}

void setTiempoNegras(uint8_t minutos, uint8_t segundos)
{
  estado_juego.segundos_negras = minutos * 60 + segundos;
  printf("TIEMPO NEGRAS RETOCADO DESDE WEB: minutos %d, segundos %d\n", minutos, segundos);
  printf("TIEMPO GUARDADO: segundos totales %d\n", estado_juego.segundos_negras);

}

////////////////////////////////////////////////////////////////////////////
// INICIALIZA
//
// Pone todas las casillas del tablero a su estado inicial:
//    - Todas las casillas est�n vac�as
//    - El color de cada casilla alterna entre blanco y negro y la
//      casilla inferior derecha es de color blanco
//
void checkJaque(AJD_TableroPtr tablero){
   uint8_t vCaballo[2] = {1, 2};
   uint8_t vVertHorz[2] = {0, 1};
   uint8_t vDiag[2] = {1, 1}; 
   int8_t cz[8] = {+1, +1, +1, -1, -1, +1, -1, -1};
   AJD_CasillaPtr rey = NULL;
   //AJD_CasillaPtr reyN = NULL;

   uint8_t id;
   uint8_t x;
   uint8_t y;
   
   // Localiza el rey
   for (int i=0; i<64; i++) 
   {
     if (tablero->casilla[i].pieza == REY && tablero->casilla[i].color_pieza == !estado_juego.juegan_blancas) 
     {
         rey = &(tablero->casilla[i]);
         break;
     }
   }
   
   // if(rey == NULL){
   //   estado_juego.negro_jaque = 0;
   //   estado_juego.blanco_jaque = 0;
   //   return;
   // }

   estado_juego.negro_jaque = estado_juego.juegan_blancas == BLANCO ? amenazaRey(rey, tablero) : 0;
   estado_juego.blanco_jaque = estado_juego.juegan_blancas == NEGRO ? amenazaRey(rey, tablero) : 0;


}

bool amenazaRey(AJD_CasillaPtr rey, AJD_TableroPtr tablero)
{
   uint8_t vCaballo[2] = {1, 2};
   uint8_t vVertHorz[2] = {0, 1};
   uint8_t vDiag[2] = {1, 1}; 
   int8_t cz[8] = {+1, +1, +1, -1, -1, +1, -1, -1};
   //bool enJaque = false;
   uint8_t id;
   uint8_t x;
   uint8_t y;
   uint8_t resposx;
	uint8_t resposy;
   uint8_t cnt;

   id = rey->id;
   x = id%8;
   y = id/8;
   AJD_CasillaPtr casilla;
   AJD_Color rey_color = rey->color_pieza;

   int pawnDir = (rey_color == BLANCO) ? -1 : 1; // 兵攻击方向
   int pawnThreats[2][2] = {{-1, pawnDir}, {1, pawnDir}};

   // Amenaza desde caballos
   for(int i=0; i<2; i++){
      for (int j=0; j<4; j++) 
      {
         casilla = &(tablero->casilla[(x + cz[j*2]*vCaballo[i]) + (y + cz[1+j*2]*vCaballo[1-i])*8]);
         if((casilla->pieza == CABALLO1 || casilla->pieza == CABALLO2) && casilla->color_pieza != rey_color)
         {
            //enJaque = true;
            return true;
         }
      }
   }

   // Amenaza desde peones
   for(int i=0; i<2; i++) {
      resposx = x + pawnThreats[i][0];
      resposy = y + pawnThreats[i][1];
    
      if(resposx >= 0 && resposx < 8 && resposy >= 0 && resposy < 8) {
         casilla = &(tablero->casilla[resposx + resposy*8]);
         if((casilla->pieza >= PEON1 && casilla->pieza <= PEON8) && 
            casilla->color_pieza != rey_color) {
               return true;
         }
      }
}

   // Amenaza desde piezas que se mueve verticales/horizontales
   for(int i=0; i<2; i++){
      for(int j=0; j<4; j++){
         resposx = x+vVertHorz[i]*cz[j*2];
         resposy = y+vVertHorz[1-i]*cz[j*2+1];
         cnt = 0;
         while(resposx >= 0 && resposx < 8 && resposy >= 0 && resposy < 8)
         {
            casilla = &(tablero->casilla[resposx + resposy*8]);
            if(casilla -> pieza == NONE)
            {
               if((casilla->pieza == TORRE1 || casilla->pieza == TORRE2 || casilla->pieza ==  DAMA)
                  && casilla->color_pieza != rey_color)
               {
                  return true;
               }
               break;
            }

            resposx += vVertHorz[i]*cz[j*2];
            resposy += vVertHorz[1-i]*cz[1+j*2];
            cnt++;
         }
      }
   }

   // Amenaza desde piezas que se mueven diagonales
   
   for(int j=0; j<4; j++){
      resposx = x+vDiag[0]*cz[j*2];
      resposy = y+vDiag[1]*cz[j*2+1];
      cnt = 0;
      while(resposx >= 0 && resposx < 8 && resposy >= 0 && resposy < 8)
      {
         casilla = &(tablero->casilla[resposx + resposy*8]);
         if(casilla-> pieza != NONE){
            if((casilla->pieza == ALFIL1 || casilla->pieza == ALFIL2 || casilla->pieza ==  DAMA)
               && casilla->color_pieza != rey_color)
            {
               return true;
            }
            break;
         }
         resposx += vDiag[0]*cz[j*2];
         resposy += vDiag[1]*cz[1+j*2];
         cnt++;
      }
   }

   return false;
}



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

   //estado_juego.segundos_blancas = 600;
   //estado_juego.segundos_negras  = 600;
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
 JuegoMsg_t msgRx = { 0 };
 uint8_t placeHolder;
 //uint8_t position;
 ECasilla position;
 uint8_t k1 = 0;
 uint8_t k2 = 0;
 uint8_t found = 0;
 //AJD_Pieza piezasMayores[8] = { TORRE, CABALLO, ALFIL, DAMA, REY, ALFIL, CABALLO, TORRE };
 for (int i = 0; i < 32;) {
   status = osMessageQueueGet(e_juegoRxMessageId, &msgRx, NULL, osWaitForever);
   printf("[juego::%s] status[%d] remitente[%d] pieza[%d]\n", __func__, status, msgRx.remitente, msgRx.pieza);
	 //posCl[i] = pos;
   if(status == osOK) {
      uint8_t pos = msgRx.pieza;
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
        printf("[RC522::%s] pos[%d] map[%d] = [%d]\n", __func__, pos, j, map[j]);
         if(map[j] == pos) {
            map[j] = 0xFF;
            tablero->casilla[j].pieza       = (AJD_Pieza)((pos & 0x0F) +1);
            tablero->casilla[j].color_pieza = (AJD_Color)((pos & 0x10) >> 4);
            ledMsg.posicion = convertNum(j);
            ledMsg.nuevaJugada = false;
            ledMsg.tipoJugada = POSIBLE_MOVIMIENTO;
            printf("[juego::%s] casilla[%d] pieza[%d] color[%d]\n", __func__, j, tablero->casilla[j].pieza, tablero->casilla[j].color_pieza);
            printf("[juego::%s] Led: posicion[%d]\n", __func__, ledMsg.posicion);
            osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
           do{ 
            status = osMessageQueueGet(e_positionMessageId, &position, NULL, osWaitForever);
           
            if(status == osOK && position.casilla == convertNum(j)){
               ledMsg.nuevaJugada = true;
               osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
            }else{
              //printf("[Error] Posicion no encontrada\n");
               //i--;
               ledMsg.posicion = position.casilla;
               ledMsg.nuevaJugada = false;
               ledMsg.tipoJugada = MOVIMIENTO_ILEGAL;
               osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
            }
          } while(position.casilla != convertNum(j));
            found = 1;
            ComPlacasMsg_t msgTx = {
              .remitente    = MENSAJE_JUEGO,
              .destinatario = MENSAJE_NFC,
              .mensaje[0]   = 0x01U,  // FLAG_PIEZA_LEIDA
              .mensaje[1]   = 0
            };
            printf("[juego::%s] Mensaje a enviar:\n", __func__);
            printf("[juego::%s] Remitente[%d] destinatario[%d] mensaje[0] = [%d] mensaje[1] = [%d]\n", __func__, msgTx.remitente, msgTx.destinatario, msgTx.mensaje[0], msgTx.mensaje[1]);
            osMessageQueuePut(e_comPlacasTxMessageId, &msgTx, 1, 0);
            ledMsg.tipoJugada = ACK;
            osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
            i++;
            break;
         }
      }
      if(!found){
         printf("[Error] Pieza no encontrado\n");
        ComPlacasMsg_t msgTx = {
          .remitente    = MENSAJE_JUEGO,
          .destinatario = MENSAJE_NFC,
          .mensaje[0]   = 0x01U,  // FLAG_PIEZA_LEIDA
          .mensaje[1]   = 0
        };
        osMessageQueuePut(e_comPlacasTxMessageId, &msgTx, 1, 0);
        ledMsg.tipoJugada = NACK;
        osMessageQueuePut(e_ledStripMessageId, &ledMsg, 1, 0);
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
 ComPlacasMsg_t msgTx = {
  .remitente    = MENSAJE_JUEGO,
  .destinatario = MENSAJE_NFC,
  .mensaje[0]   = 0x02U,  // FLAG_FINALIZA
  .mensaje[1]   = 0
 };
 osMessageQueuePut(e_comPlacasTxMessageId, &msgTx, 1, 0);
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
   AJD_Pieza piezasMayores[8] = {TORRE1, CABALLO1, ALFIL1, REY, DAMA, ALFIL2, CABALLO2, TORRE2};
   AJD_Pieza piezaPeon[8] = {PEON1, PEON2, PEON3, PEON4, PEON5, PEON6, PEON7, PEON8};
   for (int col=0; col < 8; col++)
  {
      map[col] = ((piezasMayores[col] -1)| BLACK);
      map[col + 7*8] = ((piezasMayores[col] -1)| WHITE);

      map[col + 8] = ((piezaPeon[col] -1)| BLACK);
      map[col + 6*8] = ((piezaPeon[col] -1)| WHITE);        
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
  tick_segundos = osTimerNew((osTimerFunc_t)tick_segundos_callback, osTimerPeriodic, NULL, NULL);
  e_juegoThreadId = osThreadNew(stateMachine, NULL, NULL);
	e_juegoRxMessageId = osMessageQueueNew(NUMERO_MENSAJES_JUEGO_MAX, sizeof(JuegoMsg_t), NULL);
  mapMutex = osMutexNew(NULL);

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
   MemoriaMsg_t tbPaq;
   uint8_t tbMap[64];
   ECasilla tbCasilla;
   //e_positionMessageId = osMessageQueueNew(10, sizeof(ECasilla), NULL);
   //e_juegoTxMessageId = osMessageQueueNew(10, sizeof(uint64_t), NULL);
   //e_juegoRxMessageId = osMessageQueueNew(10, sizeof(uint8_t), NULL);
   e_memoriaTxMessageId = osMessageQueueNew(32, sizeof(MemoriaMsg_t), NULL);
   e_memoriaRxMessageId = osMessageQueueNew(10, sizeof(MemoriaMsg_t), NULL);
   //tbPaq = malloc(sizeof(PAQ_status));

   osThreadFlagsSet(e_juegoThreadId, FLAG_START);
	//osDelay(300);

   tbPos[0] = (TORRE1 - 1)| WHITE;
   tbPos[1] = (CABALLO1 - 1)| WHITE;
   tbPos[2] = (ALFIL1 - 1)| WHITE;
   tbPos[3] = (DAMA - 1)| WHITE;
   tbPos[4] = (REY - 1)| WHITE;
   tbPos[5] = (ALFIL2 - 1)| WHITE;
   tbPos[6] = (CABALLO2 - 1)| WHITE;
   tbPos[7] = (TORRE2 - 1)| WHITE;
    for (int i = 8; i < 16; i++) {
       tbPos[i] = (PEON1 - 1 + (i - 8)) | WHITE;
       tbPos[i+8] = (PEON1  - 1 + (i - 8)) | BLACK;
    }
    
    // 黑方
   tbPos[24] = (TORRE1 - 1)| BLACK;
   tbPos[25] = (CABALLO1 - 1)| BLACK;
   tbPos[26] = (ALFIL1 - 1)| BLACK;
   tbPos[27] = (DAMA - 1)| BLACK;
   tbPos[28] = (REY - 1)| BLACK;
   tbPos[29] = (ALFIL2 - 1)| BLACK;
   tbPos[30] = (CABALLO2 - 1)| BLACK;
   tbPos[31] = (TORRE2 - 1)| BLACK;

   // tbPaq.map = tbMap;
   // tbPaq.turno_color = 
    osDelay(500);
   for(int i = 0; i < 32; i++){
      osMessageQueuePut(e_juegoRxMessageId, &tbPos[i], 1, osWaitForever);
      osDelay(100); // 100ms delay between sending each piece
   }
   // printChessboard();
   // osDelay(100);
   // printf("[Test] basic move:\n");
   // printf("  [Test] peon:\n");
   
   // tbCasilla.casilla = 49; // b7位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 33; // b5位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 15; // a2位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 31; // a4位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);
   // // 模拟用户操作：选择黑方兵前进

   // tbCasilla.casilla = 48; // b2位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 47; // b3位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // printf("  [Test] captura\n");
   // // 模拟用户操作：选择黑方兵前进
   // tbCasilla.casilla = 31; // b5位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);

   // tbCasilla.casilla = 33; // a4位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   
		
	// //tbCasilla.casilla = 255;
   // osThreadFlagsSet(e_juegoThreadId, FLAG_STOP);
   // printf("[Test] Time remained:  %d:%d for white; %d:%d for black\n", estado_juego.segundos_blancas/60, estado_juego.segundos_blancas%60, estado_juego.segundos_negras/60, estado_juego.segundos_negras%60);
   // //osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
	// osDelay(500);
   // osThreadFlagsSet(e_juegoThreadId, FLAG_RETOCAR);
	// osDelay(500);

   // memset(tbMap, 0, 64*sizeof(uint8_t));

   // tbMap[0] = TORRE1 | BLACK;
   // tbMap[1] = CABALLO1 | BLACK;
   // tbMap[2] = ALFIL1 | BLACK;
   // tbMap[3] = DAMA | BLACK;
   // tbMap[4] = REY | BLACK;
   // tbMap[5] = ALFIL2 | BLACK;
   // tbMap[6] = CABALLO2 | BLACK;
   // tbMap[7] = TORRE2 | BLACK;
   // // 黑方
   // tbMap[56] = TORRE1 | WHITE;
   // tbMap[57] = CABALLO1 | WHITE;
   // tbMap[58] = ALFIL1 | WHITE;
   // tbMap[59] = DAMA | WHITE;
   // tbMap[60] = REY | WHITE;
   // tbMap[61] = ALFIL2 | WHITE;
   // tbMap[62] = CABALLO2 | WHITE;
   // tbMap[63] = TORRE2 | WHITE;

   // //tbPaq.map = tbMap;
	// memcpy(tbPaq.map, tbMap, 64 * sizeof(uint8_t));
   // tbPaq.turno_color = 0;
   // osMessageQueuePut(e_memoriaTxMessageId, &tbPaq, 1, osWaitForever);
   // for(int i = 0; i < 32; i++){
   //    osMessageQueuePut(e_juegoRxMessageId, &tbPos[i], 1, osWaitForever);
   //    osDelay(100); // 10ms delay between sending each piece
   // }

   // printf("  [Test] torre:\n");
   // tbCasilla.casilla = 0; // a1位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 31; // a4位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);
   // // 模拟用户操作：选择黑方兵前进
   // tbCasilla.casilla = 63; // a8位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 32; // a5位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);


   // printf("  [Test] caballo:\n");
   // tbCasilla.casilla = 1; // b1位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 16; // a2位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);
   // // 模拟用户操作：选择黑方兵前进
   // tbCasilla.casilla = 62; // b8位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 47; // a6位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // printf("  [Test] alfil:\n");
   // tbCasilla.casilla = 2; // b1位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 40; // a2位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);
   // // 模拟用户操作：选择黑方前进
   // tbCasilla.casilla = 61; // b8位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 23; // a6位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // printf("  [Test] dama:\n");
   // tbCasilla.casilla = 3; // b1位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 25; // a2位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);
   // // 模拟用户操作：选择黑方前进
   // tbCasilla.casilla = 60; // b8位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 38; // a6位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 25; // b1位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 28; // a2位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);
   // // 模拟用户操作：选择黑方前进
   // tbCasilla.casilla = 38; // b8位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 35; // a6位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);
   

   // printf("  [Test] rey:\n");
   // tbCasilla.casilla = 4; // b1位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 11; // a2位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);
   // // 模拟用户操作：选择黑方前进
   // tbCasilla.casilla = 59; // b8位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);
   
   // tbCasilla.casilla = 52; // a6位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);
   
   // //tbCasilla.casilla = 255;
   // osThreadFlagsSet(e_juegoThreadId, FLAG_STOP);
   // printf("[Test] Time remained:  %d:%d for white; %d:%d for black\n", estado_juego.segundos_blancas/60, estado_juego.segundos_blancas%60, estado_juego.segundos_negras/60, estado_juego.segundos_negras%60);
   // //osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
	// osDelay(500);
   // osThreadFlagsSet(e_juegoThreadId, FLAG_RETOCAR);
	// osDelay(500);

   // memset(tbMap, 0, 64*sizeof(uint8_t));

   // tbMap[11] = DAMA | WHITE;

   //tbPaq.map = tbMap;
// 	memcpy(tbPaq.dato, tbMap, 64 * sizeof(uint8_t));
//    tbPaq.turno = 1;
//    //tbPaq.turno = estado_juego.juegan_blancas;
//    tbPaq.tiempoBlancas[0] = (600/60)/10 + '0';
//    tbPaq.tiempoBlancas[1] = (600/60)%10 + '0';
//    tbPaq.tiempoBlancas[2] = (600%60)/10 + '0';
//    tbPaq.tiempoBlancas[3] = (600%60)%10 + '0';
//    tbPaq.tiempoNegras[0] = (600/60)/10 + '0';
//    tbPaq.tiempoNegras[1] = (600/60)%10 + '0';
//    tbPaq.tiempoNegras[2] = (600%60)/10 + '0';
//    tbPaq.tiempoNegras[3] = (600%60)%10 + '0';
//   // status = osMessageQueuePut(e_memoriaRxMessageId, &paq, 1, 0);

//    osMessageQueuePut(e_memoriaTxMessageId, &tbPaq, 1, osWaitForever);
//    for(int i = 0; i < 32; i++){
//       osMessageQueuePut(e_juegoRxMessageId, &tbPos[i], 1, osWaitForever);
//       osDelay(100); // 10ms delay between sending each piece
//    }
   // printChessboard();
	//  osDelay(100);
	 
   // printf("  [Test] Promoción:\n");
   // tbCasilla.casilla = 12; // b8位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 3; // a6位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // estado_juego.juegan_blancas = 1;
   // printf(" [Test] Manualmente cambia el turno a blanco\n");

   // tbCasilla.casilla = 3; // b8位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 39; // a6位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // estado_juego.juegan_blancas = 1;
   // printf(" [Test] Manualmente cambia el turno a blanco\n");

   // tbCasilla.casilla = 39; // b8位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 32; // a6位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // // Victoria y jaque
   // osThreadFlagsSet(e_juegoThreadId, FLAG_STOP);
   // //osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
	// osDelay(500);
   // osThreadFlagsSet(e_juegoThreadId, FLAG_RETOCAR);
	// osDelay(500);

   // memset(tbMap, 0, 64*sizeof(uint8_t));

   // tbMap[3] = REY | BLACK;
   // tbMap[33] = CABALLO1 | WHITE;
   // tbMap[21] = PEON1 | WHITE;
   // //tbPaq.map = tbMap;
	// memcpy(tbPaq.map, tbMap, 64 * sizeof(uint8_t));
   // tbPaq.turno_color = 1;
   // osMessageQueuePut(e_memoriaTxMessageId, &tbPaq, 1, osWaitForever);
   // for(int i = 0; i < 32; i++){
   //    osMessageQueuePut(e_juegoRxMessageId, &tbPos[i], 1, osWaitForever);
   //    osDelay(100); // 10ms delay between sending each piece
   // }
   // printChessboard();
	//  osDelay(100);
   // printf(" [Test] Jaque y victoria\n");

   // tbCasilla.casilla = 33; // b1位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 18; // a2位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);
   // // 模拟用户操作：选择黑方前进
   // tbCasilla.casilla = 3; // b8位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);
   
   // tbCasilla.casilla = 4; // a6位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 21; // b1位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 10; // a2位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);
   // // 模拟用户操作：选择黑方前进
   // tbCasilla.casilla = 4; // b8位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);
   
   // tbCasilla.casilla = 3; // a6位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);

   // tbCasilla.casilla = 18; // b8位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);
   
   // tbCasilla.casilla = 3; // a6位置
   // osMessageQueuePut(e_positionMessageId, &tbCasilla, 0, osWaitForever);
   // osDelay(500);
   // printChessboard();
   // osDelay(100);
   
	//  osThreadFlagsSet(e_juegoThreadId, FLAG_STOP);
   // printf("[Test] Time remained:  %d:%d for white; %d:%d for black\n", estado_juego.segundos_blancas/60, estado_juego.segundos_blancas%60, estado_juego.segundos_negras/60, estado_juego.segundos_negras%60);
   // printf("[Test] End\n");
   



  


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
 
// void printChessboard() {
//     // 棋子符号映射表
//     const char* pieceSymbolsWhite[] = {
//         "  ",    // NONE
//         "♙",     // PEON1
//         "♙",     // PEON2
//         "♙",     // PEON3
//         "♙",     // PEON4
//         "♙",     // PEON5
//         "♙",     // PEON6
//         "♙",     // PEON7
//         "♙",     // PEON8
//         "♖",     // TORRE1
//         "♖",     // TORRE2
//         "♘",     // CABALLO1
//         "♘",     // CABALLO2
//         "♗",     // ALFIL1
//         "♗",     // ALFIL2
//         "♕",     // DAMA
//         "♔"      // REY
//     };

//     const char* pieceSymbolsBlack[] = {
//         "  ",    // NONE
//         "♟",     // PEON1
//         "♟",     // PEON2
//         "♟",     // PEON3
//         "♟",     // PEON4
//         "♟",     // PEON5
//         "♟",     // PEON6
//         "♟",     // PEON7
//         "♟",     // PEON8
//         "♜",     // TORRE1
//         "♜",     // TORRE2
//         "♞",     // CABALLO1
//         "♞",     // CABALLO2
//         "♝",     // ALFIL1
//         "♝",     // ALFIL2
//         "♛",     // DAMA
//         "♚"      // REY
//     };

//     int index;
//     AJD_Pieza piece;
//     AJD_Color color;
// 		char* symbol;
    
//     printf("     a   b   c   d   e   f   g   h\n");
//     printf("   +---+---+---+---+---+---+---+---+\n");
    
//     for (int row = 0; row < 8; row++) {
//         printf("  %d|", 8 - row); // 行号从8到1
        
//         for (int col = 0; col < 8; col++) {
//             index = row * 8 + col;
//             piece = tablero.casilla[index].pieza; // 获取棋子类型
//             color = tablero.casilla[index].color_pieza;// 获取颜色
            
//             // 根据颜色调整棋子符号（这里假设符号已经区分颜色，实际可能需要调整）
//             if (color ==BLACK) {
//                symbol = pieceSymbolsBlack[piece];
//             }else{
//               symbol = pieceSymbolsWhite[piece];
//             }

//             if (piece == NONE && predict[index] == 1) {
//                symbol = " X";
//             }
            
//             printf(" %s|", symbol);
//         }
        
//         printf("\n   +---+---+---+---+---+---+---+---+\n");
//     }
    
//     printf("     a   b   c   d   e   f   g   h\n");
// }

