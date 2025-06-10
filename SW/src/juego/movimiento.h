#ifndef MOVIMIENTO_H
#define MOVIMIENTO_H

	#include "tablero.h"
	#include "juego.h"

	// Máximo número de casillas que pueden moverse la TORRE
	#define MAX_CASILLAS_TORRE 	7  
	// Máximo número de casillas que pueden moverse el ALFIL
	#define MAX_CASILLAS_ALFIL 	7  
	// Máximo número de casillas que pueden moverse la DAMA
	#define MAX_CASILLAS_DAMA 	7  
	// Máximo número de casillas que puede moverse el PEON
	#define MAX_CASILLAS_PEON 	1
	// Máximo número de casillas que puede moverse el REY
	#define MAX_CASILLAS_REY 		1

	// id casillas involucrados en un enroque
	#define EL_ORIGEN_TORRE_BLANCA   a1
	#define EL_DESTINO_TORRE_BLANCA  d1
	#define EC_ORIGEN_TORRE_BLANCA   h1
	#define EC_DESTINO_TORRE_BLANCA  f1
	#define EL_ORIGEN_TORRE_NEGRA    a8
	#define EL_DESTINO_TORRE_NEGRA   d8
	#define EC_ORIGEN_TORRE_NEGRA    h8
	#define EC_DESTINO_TORRE_NEGRA   f8
	#define INICIO_REY_BLANCO        e1
	#define EL_DESTINO_REY_BLANCO    c1
	#define EC_DESTINO_REY_BLANCO    h1
	#define INICIO_REY_NEGRO         e8
	#define EL_DESTINO_REY_NEGRO     c8
	#define EC_DESTINO_REY_NEGRO     h8

	typedef struct {
			AJD_CasillaPtr origen;
			AJD_CasillaPtr destino;
			uint8_t srcY:3;
			uint8_t srcX:3;
			uint8_t dstY:3;
			uint8_t dstX:3;
			int dy:4;
			int dx:4;  
			int distY:4;
			int distX:4;
	} AJD_MovInfo, *AJD_MovInfoPtr;

	////////////////////////////////////////////////////////////////////////////
	// esMovimientoValido
	//
	//  Comprueba si un movimiento desde una casilla origen a destino
	// es válido.
	//
	int esMovimientoValido (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego);
	////////////////////////////////////////////////////////////////////////////
	// puedeEnrocar
	//
	//  Comprueba si el jugador que mueve puede realizar enroque
	//
	int puedeEnrocar (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego, AJD_Enroque enroque);
	////////////////////////////////////////////////////////////////////////////
	// muevePieza
	//
	//  Mueve una pieza desde una casilla origen a una casilla destino determinado
	// en estado_juego
	//
	void muevePieza (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego);
	////////////////////////////////////////////////////////////////////////////
	// efectuaEnroque
	//
	//  Realiza el movimiento de enroque
	//
	void efectuaEnroque (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego);
	////////////////////////////////////////////////////////////////////////////
	// promocionaPeon
	//
	//  Promociona el peon en la casilla indicada
	//
	void promocionaPeon (AJD_TableroPtr tablero, AJD_CasillaPtr casilla);
	////////////////////////////////////////////////////////////////////////////
	// peonUltimafila
	//
	//  Comprueba si un peon ha efectuado un movimiento a la ultima fila
	//
	int peonUltimaFila (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego);
	////////////////////////////////////////////////////////////////////////////
	// casillaLibre
	//
	//  Comprueba si en la casilla indicada hay una pieza sin importar color
	//
	int casillaOcupada (AJD_CasillaPtr casilla);
	////////////////////////////////////////////////////////////////////////////
	// hayPiezaValida
	//
	//  Comprueba si en la casilla indicada hay una pieza del jugador que 
	// efectua la jugada actual.
	//
	int hayPiezaValida (AJD_TableroPtr tablero, AJD_CasillaPtr casilla, AJD_EstadoPtr estado_juego);


#endif // MOVIMIENTO_H
