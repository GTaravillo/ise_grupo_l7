#include "movimiento.h"
//#include <utils.h>
#include <stdio.h>


#include <stdlib.h>
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// INTERFAZ PRIVADA
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Variable privada con informacion adicional del movimiento
AJD_MovInfo movInfo;
/////////////////////////////////////////////////////////////////////////////////////////
void obtenMovInfo (AJD_EstadoPtr estado_juego);
int cumpleReglasMovimiento (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego);
int cumpleReglasComerPeon ();
int cumpleReglasMovimientoPeon (AJD_TableroPtr tablero);
int cumpleReglasMovimientoCaballo ();
int cumpleReglasMovimientoRey (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego);
int seMueveEnVertHorz (AJD_TableroPtr tablero);
int seMueveEnDiagonal (AJD_TableroPtr tablero);
int caminoLibre (AJD_TableroPtr tablero, AJD_CasillaPtr origen, AJD_CasillaPtr destino);
void actualizaOpcionesDeEnroque (AJD_CasillaPtr origen, AJD_EstadoPtr estado_juego);
/////////////////////////////////////////////////////////////////////////////////////////
// obtenMovInfo
//
//  Obtiene detalles necesarios para realizar comprobaciones posteriores
// respecto las casillas origen y destino de un movimiento.
//
void obtenMovInfo (AJD_EstadoPtr estado_juego)
{
    AJD_CasillaPtr origen, destino;
    int dir;
    
    origen = estado_juego->casilla_origen;
    destino = estado_juego->casilla_destino;
    // Si el color de la pieza es BLANCO se mueven 1 hacia arriba (-1), si no,
    // se mueven 1 hacia abajo (+1)
    dir = origen->color_pieza ? -1 : 1;
    
    movInfo.origen  = origen;
    movInfo.destino = destino;    
    movInfo.srcY    = origen->id /8;
    movInfo.srcX    = origen->id &7;
    movInfo.dstY    = destino->id /8;
    movInfo.dstX    = destino->id &7;    
    movInfo.dy      = movInfo.dstY - movInfo.srcY;
    movInfo.dx      = movInfo.dstX - movInfo.srcX;
    // Multiplicando dy * dir nos da la "distancia" en vertical.
    // Realmente no se trata de distancia en el sentido matemático, ya que
    // tiene signo, dependiendo de si la pieza se ha movido hacia adelante (positivo)
    // o hacia atrás (negativo)
    movInfo.distY = movInfo.dy * dir;
    // Distancia en horizontal (en el sentido matematico, 
    // (siempre positivo, nunca negativo) del movimiento
    movInfo.distX = abs (movInfo.dx);
}
/////////////////////////////////////////////////////////////////////////////////////////
// cumpleReglasMovimiento
//
int cumpleReglasMovimiento (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego)
{
    AJD_Pieza pieza = movInfo.origen->pieza;

    switch (pieza)
    {
        case PEON:
            return cumpleReglasMovimientoPeon (tablero);

        case CABALLO:
            return cumpleReglasMovimientoCaballo ();

        case ALFIL:
            return seMueveEnDiagonal (tablero);

        case TORRE:        
            return seMueveEnVertHorz (tablero);

        case DAMA:
            return seMueveEnDiagonal (tablero)
                || seMueveEnVertHorz (tablero);

        case REY:
        {
            return cumpleReglasMovimientoRey (tablero, estado_juego);
        }
        default:
            return 0;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////
// cumpleReglasComerPeon
//
int cumpleReglasComerPeon ()
{
    return movInfo.distX == 1 && movInfo.distY == 1;
}
/////////////////////////////////////////////////////////////////////////////////////////
// cumpleReglasMovimientoPeon
//
int cumpleReglasMovimientoPeon (AJD_TableroPtr tablero)
{
    AJD_Color colorPiezaOrigen = movInfo.origen->color_pieza;
    int distY = movInfo.distY;
    int distX = movInfo.distX;
    int filaInicioPeones = colorPiezaOrigen ? 6 : 1;    

    if (distX == 0)
    {
        // Movimiento vertical de dos casillas, sólo es válido si 
        // es 1er movimiento o lo que es lo mismo, si se parte de 
        // las filas de inicio de PEON.
        return (distY == 2 && movInfo.srcY == filaInicioPeones 
                && caminoLibre (tablero, movInfo.origen, movInfo.destino))
        // Movimiento vertical de una casilla
            || distY == 1;
    }
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
// seMueveEnVertHorz
//
int seMueveEnVertHorz (AJD_TableroPtr tablero)
{         
    return (movInfo.dy == 0 || movInfo.dx == 0) 
        && caminoLibre (tablero, movInfo.origen, movInfo.destino);
}
/////////////////////////////////////////////////////////////////////////////////////////
// seMueveEnDiagonal
//
int seMueveEnDiagonal (AJD_TableroPtr tablero)
{
    return (abs(movInfo.dy) == abs(movInfo.dx))
        && caminoLibre (tablero, movInfo.origen, movInfo.destino);
}
/////////////////////////////////////////////////////////////////////////////////////////
// cumpleReglasMovimientoCaballo
//
int cumpleReglasMovimientoCaballo ()
{
    // La "distancia" movInfo.distY es un tanto especial, ya que tiene
    // signo para poder saber si la pieza se desplaza hacia adelante o atrás.
    // Necesitamos el valor absoluto para obtener la distancia en el sentido
    // matemático.
    int distY = abs(movInfo.distY);
    int distX = movInfo.distX;

    // Movmiento en "L"
    return (distY == 2 && distX == 1) || (distX == 2 && distY == 1);
}
/////////////////////////////////////////////////////////////////////////////////////////
// cumpleReglasMovimientoRey
//
int cumpleReglasMovimientoRey (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego)
{
    int dx = movInfo.dx;
    int dy = abs (movInfo.dy);

    // Intenta efectuar un enroque?
    if (abs(dx)==2 && dy == 0) 
    {
        if (dx < 0) 
        {            
            if (puedeEnrocar (tablero, estado_juego, ENROQUE_LARGO))
            {
                estado_juego->enroque_efectuado = ENROQUE_LARGO;
                return 1;
            }

        }
        else if (dx > 0) 
        {
            if (puedeEnrocar (tablero, estado_juego, ENROQUE_CORTO))
            {
                estado_juego->enroque_efectuado = ENROQUE_CORTO;
                return 1;
            }
        }
    }
    // Movimiento normal
    dx = abs(dx);
    return (dx==1 && dy==1) 
        ||  (dx==0 && dy==1)
        ||  (dx==1 && dy==0);
}
/////////////////////////////////////////////////////////////////////////////////////////
// caminoLibre
//
int caminoLibre (AJD_TableroPtr tablero, AJD_CasillaPtr origen, AJD_CasillaPtr destino)
{
    int dx, dy;    
    AJD_idCasilla idOrigen  = origen->id;
    AJD_idCasilla idDestino = destino->id;    
    dx = dy = 0;

//    dx = sign (movInfo.dx);
//    dy = sign (movInfo.dy) * 8;

    for (AJD_idCasilla idCasilla = idOrigen+dy+dx; idCasilla != idDestino; idCasilla += dy+dx)
    {
        AJD_CasillaPtr casilla = &tablero->casilla[idCasilla];
        if (casillaOcupada (casilla)) 
            return 0;
    }
    return 1;
}
////////////////////////////////////////////////////////////////////////////
// actualizaOpcionesDeEnroque
//
void actualizaOpcionesDeEnroque (AJD_CasillaPtr origen, AJD_EstadoPtr estado_juego)
{    
    // Invalidar enroque largo o corto si se ha movido torre o rey
    if (origen->pieza == REY)
    {
        if (estado_juego->juegan_blancas)
        {
            estado_juego->enroque_largo_blanco_invalidado = 
            estado_juego->enroque_corto_blanco_invalidado = 1;
        }
        else
        {
            estado_juego->enroque_largo_negro_invalidado = 
            estado_juego->enroque_corto_negro_invalidado = 1;
        }
    }
    else if (origen->pieza == TORRE)
    {
        switch (origen->id)
        {
            case a1: estado_juego->enroque_largo_blanco_invalidado = 1; break;
            case a8: estado_juego->enroque_largo_negro_invalidado  = 1; break;
            case h1: estado_juego->enroque_corto_blanco_invalidado = 1; break;
            case h8: estado_juego->enroque_corto_negro_invalidado  = 1; break;
            default: break;
        }
        
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// INTERFAZ PUBLICA
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// esMovimientoValido
//
int esMovimientoValido (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego)
{
    obtenMovInfo (estado_juego);
    
    // Primero que cumpla las reglas de movimiento especifico de la pieza.
    // Si lo cumple, mira si la casilla destino está ocupada.
    // Si la pieza ocupada es del mismo "bando", el movimiento no es valido 
    // Si la casilla es del mismo color, come la pieza. 
    // Si la casilla está libre, es un movimiento valido.
    // El PEON es la única pieza que come de forma diferente, así que se 
    // comprueba en un caso aparte si se detecta movimiento de pieza no válido.
    if (cumpleReglasMovimiento (tablero, estado_juego))
    {
        if (casillaOcupada (movInfo.destino))
        {               
            if (movInfo.origen->pieza != PEON)
                return  (movInfo.destino->color_pieza != movInfo.origen->color_pieza);
        }           
        else            
            return 1;
    }    
    if (movInfo.origen->pieza == PEON)
        return cumpleReglasComerPeon() && casillaOcupada (movInfo.destino);
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
// puedeEnrocar
//
int puedeEnrocar (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego, AJD_Enroque enroque)
{
    AJD_CasillaPtr destino, casillaRey;
    AJD_Color juegan_blancas;

    juegan_blancas = estado_juego->juegan_blancas;
    destino = casillaRey = NULL;
// El enroque sólo es admisible si todos cumplen las siguientes condiciones:
//
// Ninguna de las piezas que intervienen en el enroque puede haber sido movido previamente.
// No debe haber ninguna pieza entre el rey y la torre;
// El rey no puede estar en jaque, ni tampoco podrá pasar a través de casillas que están 
// bajo ataque por parte de las piezas enemigas. Al igual que con cualquier movimiento, 
// el enroque es ilegal si pusiera al rey en jaque.    
    casillaRey = juegan_blancas ? 
        &(tablero->casilla[INICIO_REY_BLANCO]) :
        &(tablero->casilla[INICIO_REY_NEGRO]);

    switch (enroque)
    {
        case ENROQUE_LARGO:
            if (juegan_blancas)
                if (estado_juego->enroque_largo_blanco_invalidado) return 0;
                else destino = &(tablero->casilla[EL_ORIGEN_TORRE_BLANCA]);
            else
                if (estado_juego->enroque_largo_negro_invalidado) return 0;
                else destino = &(tablero->casilla[EL_ORIGEN_TORRE_NEGRA]);
            break;
            
        case ENROQUE_CORTO:
            if (juegan_blancas)
                if (estado_juego->enroque_corto_blanco_invalidado) return 0;
                else destino = &(tablero->casilla[EC_ORIGEN_TORRE_BLANCA]);
            else
                if (estado_juego->enroque_corto_negro_invalidado) return 0;
                else destino = &(tablero->casilla[EC_ORIGEN_TORRE_NEGRA]);
            break;

        default:
            break;
    }
    return caminoLibre (tablero, casillaRey, destino);
}
/////////////////////////////////////////////////////////////////////////////////////////
// muevePieza
//
void muevePieza (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego)
{
    AJD_CasillaPtr origen, destino;    
    origen = estado_juego->casilla_origen;    
    destino = estado_juego->casilla_destino;

    // Si el movimiento es de alguna de las dos TORRES o el REY, invalidar la opción
    // de enroque correspondiente.
    actualizaOpcionesDeEnroque (origen, estado_juego);

    destino->pieza = origen->pieza;
    destino->color_pieza = origen->color_pieza;        
    origen->pieza = NONE;
}
////////////////////////////////////////////////////////////////////////////
// promocionaPeon
//
//  Promociona el peon en la casilla indicada
//
void promocionaPeon (AJD_TableroPtr tablero, AJD_CasillaPtr casilla)
{
    printf ("Promocionando PEON      ");
    // TODO: De momento siempre promociona a DAMA, queda pendiente
    //       poder seleccionar (con cursores p.ej.) el tipo de pieza
    casilla->pieza = DAMA;
}
////////////////////////////////////////////////////////////////////////////
// peonUltimafila
//
//  Comprueba si un peon ha efectuado un movimiento a la ultima fila
//
int peonUltimaFila (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego)
{
    AJD_idCasilla limites[2][2] = { 
    // 1a y ultima casilla fila promocion NEGRAS
        {a1, h1},
    // 1a y ultima casilla fila promocion BLANCAS
        {a8, h8}
    };
    AJD_idCasilla idCasilla;
    AJD_Color juegan_blancas;    

    // Si no es un peon no hagas más nada
    if (estado_juego->casilla_destino->pieza != PEON) return 0;


    idCasilla = estado_juego->casilla_destino->id;    
    juegan_blancas = estado_juego->juegan_blancas;

    printf ("[%d] in [%d]..[%d]?     ", 
              idCasilla, 
              limites[juegan_blancas][0],
              limites[juegan_blancas][1]);

    if (idCasilla >= limites[juegan_blancas][0] 
        &&  idCasilla <= limites[juegan_blancas][1])
    {
        printf ("SI!!");
        return 1;
    }
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
// efectuaEnroque
//
void efectuaEnroque (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego)
{
    AJD_idCasilla origen, destino;

    printf("ENROCANDO!!!");
    // Mueve primero el rey
    muevePieza (tablero, estado_juego);

    origen = destino = 0;
    // Selecciona la torre origen/destino correspondiente al enroque
    if (estado_juego->juegan_blancas)
    {
        if (estado_juego->enroque_efectuado == ENROQUE_LARGO)
        {
            origen = EL_ORIGEN_TORRE_BLANCA;
            destino = EL_DESTINO_TORRE_BLANCA;
        }
        else if (estado_juego->enroque_efectuado == ENROQUE_CORTO)
        {
            origen = EC_ORIGEN_TORRE_BLANCA;
            destino = EC_DESTINO_TORRE_BLANCA;
        }
    }
    else
    {
        if (estado_juego->enroque_efectuado == ENROQUE_LARGO)
        {
            origen = EL_ORIGEN_TORRE_NEGRA;
            destino = EL_DESTINO_TORRE_NEGRA;
        }
        else if (estado_juego->enroque_efectuado == ENROQUE_CORTO)
        {
            origen = EC_ORIGEN_TORRE_NEGRA;
            destino = EC_DESTINO_TORRE_NEGRA;
        }
    }
    // ... y realiza su movimiento correspondiente
    estado_juego->casilla_origen = &(tablero->casilla[origen]);
    estado_juego->casilla_destino = &(tablero->casilla[destino]);
    muevePieza (tablero, estado_juego);
}
/////////////////////////////////////////////////////////////////////////////////////////
// casillaOcupada
//
int casillaOcupada (AJD_CasillaPtr casilla)
{
    return casilla->pieza != NONE;
}
/////////////////////////////////////////////////////////////////////////////////////////
// hayPiezaValida
//
int hayPiezaValida (AJD_TableroPtr tablero, AJD_CasillaPtr casilla, AJD_EstadoPtr estado_juego)
{
    return casillaOcupada (casilla)
        && casilla->color_pieza == estado_juego->juegan_blancas;
}
/////////////////////////////////////////////////////////////////////////////////////////
