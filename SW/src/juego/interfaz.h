#pragma once

#include <tablero.h>
#include <juego.h>
#include <menu.h>
#include <movimiento.h>

#include <ncurses.h>

#define ANCHO_CASILLA 3
#define ALTO_CASILLA  3

extern AJD_Sprite sprCursorPiezaSeleccionada;
extern AJD_Sprite sprCursorMovil;

void inicializaPantalla();
void inicializaSprites(AJD_TableroPtr tablero);
void liberaPantalla();
void dibujaJuego (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego);
void dibujaTablero(AJD_TableroPtr tablero);
void dibujaPieza (int posy, int posx, AJD_Pieza pieza, AJD_Color color);
void dibujaMarcadores(AJD_EstadoPtr estado);
void dibujaCursor (AJD_Cursor cursor);
void dibujaMenu (int y, int x, menu_t* menu);
int  muestraMenu (int x, int y, menu_t* menu);
int  obtenJugada (int* celda_origen, int* celda_destino);
int  procesaTeclado (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego);

// Visualizaciones para depuracion 
void muestraMovInfo (AJD_MovInfoPtr movInfo);
