#pragma once

/////////////////////////////////////////////////////////////////////
// estructura para representar una entrada de menu de juego
//
// 			y: posicion vertical de la entrada
// 			x: posicion horizotnal de la entrada
// menuString: Texto a mostrar en esta entrada
//
typedef struct 
{
	char* 	menuString;
} menuItem_t;
/////////////////////////////////////////////////////////////////////
// estructura para representar un menu de juego
//
//   nitems: No. de items en el menu.
//    items: array de entradas de menu. La primera entrada 
//		     (items[0]) es el titulo a mostrar en el menu.
// selected: Elemento del menu actualmente seleccionado
//
typedef struct 
{
	int 		nitems;	
	int 		selected;
	menuItem_t 	items[10];
} menu_t;

/////////////////////////////////////////////////////////////////////
// FUNCIONES PUBLICAS
void menuJuego();
