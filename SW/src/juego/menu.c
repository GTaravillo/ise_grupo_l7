#include <menu.h>
#include <interfaz.h>

/////////////////////////////////////////////////////////////////////
// strings y posicion del menu eleccion del tipo de jugador
//
menu_t menuJugadores = 
{	3, 	// 3 items in the menu (title does not count)
	1,	// First item selected
	{
		{ "Elige jugadores"	 },
		{ "     CPU vs HUMANO   " },
		{ "  HUMANO vs HUMANO   " }, 
		{ "     CPU vs CPU      " },
	},
};

/////////////////////////////////////////////////////////////////////
// strings y posicion del menu eleccion de color
//
menu_t menuColorJugador = 
{	2, 	// 3 items in the menu (title does not count)
	1,	// First item selected
	{
		{ "Jugador 1: Blancas o Negras"	 },
		{ "   Blancas   " },
		{ "   Negras    " }, 
	},
};


/////////////////////////////////////////////////////////////////////
// menuJuego Elección de tipo de jugadores, humano vs cpu y color
//
void menuJuego(AJD_EstadoPtr estado_juego)
{
	if (muestraMenu (5, 7, &menuJugadores))
	{
		if ( muestraMenu (5, 7, &menuColorJugador) )
		{
			estado_juego->turno_jugador = menuColorJugador.selected-1;
		}
	}


}
