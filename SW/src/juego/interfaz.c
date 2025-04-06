#include <interfaz.h>
#include <utils.h>

#include <string.h>

#define TABLERO_ROW_START   10
#define TABLERO_COL_START   10
#define MARCADOR_ROW_START  TABLERO_ROW_START
#define MARCADOR_COL_START  TABLERO_COL_START + 8 * 3 + 2
#define MARCADOR_LAST_ROW   TABLERO_ROW_START + 8 * 3 - 1
AJD_Sprite sprCursorPiezaSeleccionada;
AJD_Sprite sprCursorMovil;

void actualizaTiempoGUI (AJD_Color juegan_blancas, int itime);
////////////////////////////////////////////////////////////////////////////
// _dibujaCasilla
//
// Dibuja una casilla del tablero con su pieza si la tuviera.
//
// ...###
// .p.###
// ...###
//
void _dibujaCasilla(AJD_TableroPtr tablero, int ncasilla, int posx, int posy)
{
    // columna y fila correspondientes a esta casilla en el tablero
    int nrow = ncasilla / 8;
    int ncol = ncasilla & 7;

    // Puntero a la casilla a dibujar
    AJD_CasillaPtr casilla = &(tablero->casilla[nrow * 8 + ncol]);

    // Color principal/fondo a usar dependiendo de la casilla
    int color = casilla->color == BLANCO ? 1 : 2;   
    attron(COLOR_PAIR (color));

    for (nrow = 0; nrow < ALTO_CASILLA; nrow++)
    {       
        move (posy+nrow, posx);
        printw("   ");  // 3 espacios

        // Dibuja pieza si es que hay alguna en la casilla actual
        if (casilla->pieza)
        {           
            dibujaPieza (posy+1, posx+1, casilla->pieza, casilla->color_pieza);
        }

        attron(COLOR_PAIR (color));
    }
}
////////////////////////////////////////////////////////////////////////////
// inicializaPantalla
//
// Inicializa libreria ncurses y la interfaz de usuario en general.
//
void inicializaPantalla()
{
    // Inicializa ncurses
    initscr();

    // Habilita teclas especiales (F1, cursores, ESC, etc.)
    keypad (stdscr, 1);

    // Envío de teclas inmediato (sin pulsar ENTER)
    cbreak();

    // No mostrar caracteres pulsados
    noecho();

    // No espera en lectura de caracter
    nodelay(stdscr, 1);

    // Ocultar cursor de pantalla
    curs_set(0);

    // Inicializa colores 
    if ( has_colors() )
    {
        start_color();

        // Negro sobre fondo blanco para dibujar casilla blanca
        init_pair(1, COLOR_BLACK, COLOR_WHITE);

        // Blanco sobre negro para dibujar casilla negra y textos
        init_pair(2, COLOR_WHITE, COLOR_BLACK);

        // Amarillo sobre negro para los menus
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);

        attron(COLOR_PAIR(1));
    }

    clear();
}

void inicializaSprites(AJD_TableroPtr tablero)
{
    // Definicion de los cursores
    chtype charsCursorMovil[] = {
        ACS_ULCORNER, '.', ACS_URCORNER, 
        '.','.','.', 
        ACS_LLCORNER, '.', ACS_LRCORNER,
    };
    chtype charsCursorPiezaSeleccionada[] = {
        ACS_ULCORNER, ACS_HLINE  , ACS_URCORNER ,
        ACS_VLINE   , '.'        , ACS_VLINE    ,
        ACS_LLCORNER, ACS_HLINE  , ACS_LRCORNER 
    };
    
    memcpy (sprCursorMovil.ch, charsCursorMovil, 
            NCHARS_IN_SPRITE*sizeof(chtype));

    memcpy (sprCursorPiezaSeleccionada.ch, charsCursorPiezaSeleccionada, 
            NCHARS_IN_SPRITE*sizeof(chtype));

    tablero->cursorMovil.sprite = &sprCursorMovil;
    tablero->cursorPiezaSeleccionada.sprite = &sprCursorPiezaSeleccionada;
}
////////////////////////////////////////////////////////////////////////////
// liberaPantalla
//
// Libera los recursos de ncurses.
//
void liberaPantalla()
{
    endwin();
}
////////////////////////////////////////////////////////////////////////////
// dibujaJuego
//
// Dibuja todos los elementos del juego
//
void dibujaJuego(AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego)
{
    dibujaTablero (tablero);
    dibujaMarcadores (estado_juego);
}
////////////////////////////////////////////////////////////////////////////
// dibujaTablero
//
// Dibuja el tablero en pantalla según su estado actual.
//
void dibujaTablero(AJD_TableroPtr tablero)
{
    int row, col, y, x;

    int idCasilla = 0;
    for (row=0; row < 8; row++)
    for (col=0; col < 8; col++)
    {
        _dibujaCasilla (tablero, idCasilla, 
            TABLERO_COL_START + col * ANCHO_CASILLA,
            TABLERO_ROW_START + row * ALTO_CASILLA);

        // Siguiente casilla
        idCasilla++;
    }

    // Dibuja los numeros de las filas
    attron ( COLOR_PAIR (2) );
    row = 8;
    y = TABLERO_ROW_START + 1;
    x = TABLERO_COL_START-2;
    while (row)
    {
        move (y, x);
        printw("%d", row);
        y += 3;
        row--;
    }

    // Dibuja letras de columnas
    col = 8;
    x = TABLERO_COL_START + 8 * 3 - 2;
    y = TABLERO_ROW_START - 2;
    while (col)
    {
        move (y, x);
        printw ("%c", col+'a'-1);
        x -= 3;
        col--;
    }

    // Dibuja el cursor de seleccion
    if (tablero->cursorMovil.visible)
    {
        dibujaCursor(tablero->cursorMovil);
    }
    if (tablero->cursorPiezaSeleccionada.visible)
    {
        dibujaCursor(tablero->cursorPiezaSeleccionada);
    }

}
////////////////////////////////////////////////////////////////////////////
// dibujaPieza Dibuja una pieza del ajedrez en la posición indicada
//
void dibujaPieza (int posy, int posx, AJD_Pieza pieza, AJD_Color color)
{
    // Array de caracteres representado la letra a dibujar de cada pieza
    char *sprPiezas=" PTCADR";
    char sprite = sprPiezas[pieza];

    // Piezas negras las mostraremos en minusculas
    if (color == NEGRO) sprite += 32;
    if (color == BLANCO) color = 1;

    // Dibuja la letra de la pieza en color blanco sobre fondo negro
    attron (COLOR_PAIR (1));
    move (posy, posx);
    printw("%c", sprite);
    attroff (COLOR_PAIR (1));
}
////////////////////////////////////////////////////////////////////////////
// dibujaMarcadores Dibuja los marcadores de tiempo, turno, etc.
//
void dibujaMarcadores(AJD_EstadoPtr estado_juego)
{
    char buff[6];
    int x,y;

    x = MARCADOR_COL_START;
    y = MARCADOR_ROW_START + 1;
    attron (COLOR_PAIR (2));
    mvprintw (y,x, "AJEDREZ 1.0");

    y += 1;
    mvprintw (y,x, "(c) Jun 2020 Andres Mata");

    y += 4;
    mvprintw (y,x, "Blancas %s", strSegundos (buff, estado_juego->segundos_blancas));

    y += 2;
    mvprintw (y,x, "Negras %s", strSegundos (buff, estado_juego->segundos_negras));

    y += 2;
    if (estado_juego->enroque_efectuado)
    {
        mvprintw (y,x, 
                  estado_juego->enroque_efectuado == ENROQUE_LARGO ? 
                  "O-O-O" : "O-O");
    }   

    y = MARCADOR_LAST_ROW - 1;
    mvprintw (y,x, "Turno %02d (Jugador %c)", estado_juego->turno, estado_juego->turno_jugador+'1');

    y += 1;
    mvprintw (y,x, "Juegan %s", estado_juego->juegan_blancas ? "blancas" : "negras ");

    // Debug info
    mvprintw (MARCADOR_LAST_ROW+2, 0, "Enroque largo permitido (BLANCAS): %s", 
              estado_juego->enroque_largo_blanco_invalidado ? "NO" : "SI");
    mvprintw (MARCADOR_LAST_ROW+3, 0, "Enroque corto permitido (BLANCAS): %s", 
              estado_juego->enroque_corto_blanco_invalidado ? "NO" : "SI");
    mvprintw (MARCADOR_LAST_ROW+4, 0, "Enroque largo permitido (NEGRAS): %s",
              estado_juego->enroque_largo_negro_invalidado ? "NO" : "SI");
    mvprintw (MARCADOR_LAST_ROW+5, 0, "Enroque corto permitido (NEGRAS): %s", 
              estado_juego->enroque_corto_negro_invalidado ? "NO" : "SI");


}
////////////////////////////////////////////////////////////////////////////
// dibujaCursor Dibuja el cursor de seleccion
//
void dibujaCursor (AJD_Cursor cursor)
{
    int y, x, flash;
    AJD_CasillaPtr casillaCursor = cursor.casilla;
    
    y = TABLERO_ROW_START + (casillaCursor->id/8) * ALTO_CASILLA;
    x = TABLERO_COL_START + (casillaCursor->id & 7) * ANCHO_CASILLA;

    flash = cursor.flash;

    if (flash)
        attron (A_BLINK);
    else
        attroff (A_BLINK);

    if (casillaCursor->color == BLANCO)
        attron (A_REVERSE);
    else
        attroff (A_REVERSE);

    chtype* ch = &(cursor.sprite->ch[0]);

    for (int dy=0; dy<ALTO_CASILLA; dy++)
    {    
        for (int dx=0; dx<ANCHO_CASILLA; dx++)
        {
            if (*ch != '.')
                mvaddch (y + dy, x + dx, *ch);
            ch++;
        }
        
    }
    attroff (A_BLINK);
    attroff (A_REVERSE);
}
////////////////////////////////////////////////////////////////////////////
// dibujaMenu Dibuja un menu con su titulo y sus elementos.
//
void dibujaMenu(int y, int x, menu_t* menu)
{
    int nitems = menu->nitems;
    int item_id = 1;

    // Menu Title
    attron ( COLOR_PAIR (3) );  
    move (y, x);
    printw (menu->items[0].menuString);

    while (nitems--)
    {
        y += 2;
        menuItem_t* menu_item = &(menu->items[item_id]);        
        attron (COLOR_PAIR (2) );

        // Resaltar la opcion actual del menu
        if (item_id == menu->selected)
            attron (COLOR_PAIR (1) );

        mvprintw (y,x, menu_item->menuString);

        item_id++;
    }

    y += 3;
    attron (COLOR_PAIR (2));
    mvprintw (y,x, "Cursores selecciona opcion, ENTER confirmar");
}

////////////////////////////////////////////////////////////////////////////
// muestraMenu Muestra un menu y espera selección.
//             Devuelve true cuando se ha seleccionado una opción del menu
//
int muestraMenu (int x, int y, menu_t* menu)
{   
    int ch;

    clear();
    do
    {
        dibujaMenu (x, y, menu);
        ch = getch();
        if (ch == KEY_UP)   menu->selected--;
        if (ch == KEY_DOWN) menu->selected++;

        if (menu->selected < 1) menu->selected = menu->nitems;
        if (menu->selected > menu->nitems) menu->selected = 1;
    } while (ch != '\n');

    clear();
    return true;
}

////////////////////////////////////////////////////////////////////////////
// obtenJugada Introducción por teclado de la pieza a mover.
//   Devuelve celda origen y destino de la pieza a mover.
//   Actualmente no se usa, en su lugar se utiliza  'procesaTeclado' 
//   para mover la pieza con los cursores, pero se deja por si en un futuro 
//   se desease introducir jugadas directamente.
//
int obtenJugada (int* celda_origen, int* celda_destino)
{
    // columna/fila
    int columna, fila;
    int ch;

    int y = MARCADOR_ROW_START + 11;
    move (y, MARCADOR_COL_START);
    printw ("Instroduzca casilla ORIGEN");
    move (y+=1, MARCADOR_COL_START);
    printw ("Columna:");
    move (y+=1, MARCADOR_COL_START);    
    printw ("Fila:");

    // Activar que se muestren caracteres tecleados
    curs_set (1);
    echo ();
    nodelay(stdscr, 0);

    move (MARCADOR_ROW_START + 12, MARCADOR_COL_START + 9);
    ch = getch();
    columna = ch - 'a';

    move (MARCADOR_ROW_START + 13, MARCADOR_COL_START + 6);
    ch = getch();
    fila = 8 -  ch + '0';
    *celda_origen = 8 * fila + columna;

    move (0,0);
    printw ("Celda Origen %d", *celda_origen);

    move (y+=2, MARCADOR_COL_START);
    printw ("Instroduzca casilla DESTINO");
    move (y+=1, MARCADOR_COL_START);
    printw ("Columna:");
    move (y+=1, MARCADOR_COL_START);    
    printw ("Fila:");

    move (MARCADOR_ROW_START + 16, MARCADOR_COL_START + 10);
    
    ch = getch();
    columna = ch - 'a';

    move (MARCADOR_ROW_START + 17, MARCADOR_COL_START + 6);
    ch = getch();
    fila = 8 - ch + '0';
    *celda_destino = 8 * fila + columna;

    move (1,0);
    printw ("Celda Destino %d", *celda_destino);

    return true;
}
////////////////////////////////////////////////////////////////////////////
// procesaTeclado Lectura del teclado y actualizacion de cursor
//             Devuelve celda origen y destino de la pieza a mover.
//
int procesaTeclado (AJD_TableroPtr tablero, AJD_EstadoPtr estado_juego)
{
    int ch;    
    AJD_Cursor*  cursorMovil;
    AJD_idCasilla idCasilla;

    cursorMovil = &tablero->cursorMovil;
    idCasilla = cursorMovil->casilla->id;
 
    ch = getch();
    switch (ch)
    {
        case KEY_UP:    
            idCasilla -= 8;
            // Aseguramos que el cursor movil se mantiene en los límites del tablero
            idCasilla &= 63;
            cursorMovil->casilla = &tablero->casilla[idCasilla];
            break;

        case KEY_DOWN:  
            idCasilla += 8;
            // Aseguramos que el cursor movil se mantiene en los límites del tablero
            idCasilla &= 63;
            cursorMovil->casilla = &tablero->casilla[idCasilla];
            break;

        case KEY_LEFT:  
            idCasilla -= 1;
            // Aseguramos que el cursor movil se mantiene en los límites del tablero
            idCasilla &= 63;
            cursorMovil->casilla = &tablero->casilla[idCasilla];
            break;

        case KEY_RIGHT: 
            idCasilla += 1;
            // Aseguramos que el cursor movil se mantiene en los límites del tablero
            idCasilla &= 63;            
            cursorMovil->casilla = &tablero->casilla[idCasilla];
            break;

        case '\n':
            if (estado_juego->casilla_seleccionada == NONE)
            {                
                estado_juego->casilla_seleccionada = ORIGEN_SELECCIONADO;
                estado_juego->casilla_origen = cursorMovil->casilla;
                tablero->cursorPiezaSeleccionada.casilla = cursorMovil->casilla;
            }
            else
            {
                // Si se selecciona como casilla destino la misma casilla
                // origen se cancela la selección.
                estado_juego->casilla_destino = cursorMovil->casilla;
                if (estado_juego->casilla_destino == estado_juego->casilla_origen)
                {
                    estado_juego->casilla_seleccionada = NO_SELECCION;
                    tablero->cursorPiezaSeleccionada.visible = 0;
                }
                else
                    estado_juego->casilla_seleccionada = DESTINO_SELECCIONADO;                
            }
            break;

        case '\033':  // ESC
            estado_juego->fin_juego = 1;    // Salir del juego
    }    
    
    mvprintw (0,0, "cursorMovil.id: %2d", cursorMovil->casilla->id);
    mvprintw (0,25, "Pieza de color: %s", 
              cursorMovil->casilla->color_pieza
              ? "BLANCO" : "NEGRO ");
    mvprintw (0,50, "Casilla de color: %s", 
              cursorMovil->casilla->color
              ? "BLANCO" : "NEGRO ");    
    mvprintw (1,0, "casilla_seleccionada: %d", estado_juego->casilla_seleccionada);
    return 0;
}
////////////////////////////////////////////////////////////////////////////
// FUNCIONES VISUALIZACION PARA DEPURACION
////////////////////////////////////////////////////////////////////////////
// muestraMovInfo
//
void muestraMovInfo (AJD_MovInfo* movInfo)
{
    dibujaPieza (1, 25, movInfo->origen->pieza, movInfo->origen->color_pieza);
    mvprintw (1, 27, "src:%2d dst:%2d dy:%2d dx:%2d distY:%2d distX:%2d",        
        movInfo->origen->id,
        movInfo->destino->id,
        movInfo->dy,
        movInfo->dx,
        movInfo->distY,
        movInfo->distX);
}

void actualizaTiempoGUI (AJD_Color juegan_blancas, int itime)
{
    if (juegan_blancas)
        mvprintw (MARCADOR_ROW_START+5, MARCADOR_COL_START, 
            "Blancas 00:%d", itime);
    else
        mvprintw (MARCADOR_ROW_START+7, MARCADOR_COL_START,
            "Negras 00:%d", itime);
}
