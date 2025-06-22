#ifndef JUEGO_H
#define JUEGO_H

	#include "tablero.h"
	#include <stdint.h>
	#include <time.h>
	
  #include "../Config/Paths.h"
  #include PATH_COM_PLACAS
  
  #define FLAG_START   0x01U
  #define FLAG_RETOCAR 0x02U
 	#define FLAG_PAUSE   0X04U
	#define FLAG_STOP    0X08U
	#define FLAG_RESUME  0X10U
  #define FLAG_TURN    0x20U

  #define NUMERO_MENSAJES_JUEGO_MAX 8

  //typedef enum { NONE, PEON, TORRE, CABALLO, ALFIL, DAMA, REY } AJD_Pieza;
	// #define NONE	0x00
	// #define PEON	0x01
	// #define TORRE	0x02
	// #define CABALLO	0x03
	// #define ALFIL	0x04
	// #define DAMA	0x05
	// #define REY		0x06

	#define WHITE	0X10
	#define BLACK	0X00

	#define FLAG_ERROR_MOV 0X02


	typedef enum { NO_SELECCION, ORIGEN_SELECCIONADO, DESTINO_SELECCIONADO } AJD_Seleccion;
	typedef enum { NO_ENROQUE, ENROQUE_LARGO, ENROQUE_CORTO } AJD_Enroque;
	typedef enum { negeo, blanco } PAQ_Turno;
	// Paquete que se recibe
	typedef struct {
		uint8_t turno_color : 1;
		uint8_t map[64];
	} PAQ_status;

	//Manejo deln tiempo
	
	// Tipo para representar el estado
	typedef struct
	{
		//  uint16_t       turno;                     // Cu�ntos turnos se han jugado ya
		 uint16_t       segundos_blancas;
		 uint16_t       segundos_negras;
		 uint8_t        juegan_blancas      : 1; 
		 uint8_t        negro_jaque         : 1; 
		 uint8_t        blanco_jaque        : 1; 
		 uint8_t        enroque_largo_blanco_invalidado  : 1;
		 uint8_t        enroque_corto_blanco_invalidado  : 1;
		 uint8_t        enroque_largo_negro_invalidado  : 1;
		 uint8_t        enroque_corto_negro_invalidado  : 1;
		 AJD_Enroque    enroque_efectuado   : 2; 
		//  uint8_t        turno_jugador       : 1; 
		 AJD_Seleccion  casilla_seleccionada: 2; 
		 AJD_CasillaPtr casilla_origen;  // casilla origen de la pieza a mover
		 AJD_CasillaPtr casilla_destino; // casilla destino de la pieza a mover   
		 uint8_t        fin_juego : 1;          // Salir del juego?
	} AJD_Estado, *AJD_EstadoPtr;
  
  typedef struct {
    EModulos remitente;
    uint8_t pieza;
  } JuegoMsg_t;
  
  extern osMessageQueueId_t e_juegoRxMessageId;

	/////////////////////////////////////////////////////////////////////
	// INTERFAZ PUBLICA
	//
	void inicializa(AJD_TableroPtr tablero);
	void nuevoJuego(AJD_TableroPtr tablero);
	void ejecutaPartida (AJD_TableroPtr tablero);
	void actualizaJuego (AJD_TableroPtr tablero);
//	void menu();
//	void liberaRecursos();
	
	void juegoInitialize(void);
	void juegoTbInitialize(void);

#endif // JUEGO_H

