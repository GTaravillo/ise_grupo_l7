t <!DOCTYPE html>
t <html>
t <head>
t   <meta charset="UTF-8">
t     <title>Nueva Partida</title>
t     <script language="JavaScript" type="text/javascript" src="nuevaPartida.js"></script>
t     <style>
t       :root {
#         Bloque Fecha - Hora - Consumo actual
t         --fecha-font-family:      'Arial', sans-serif;
t         --hora-font-family:       'Arial', sans-serif;
t         --consumo-font-family:    'Arial', sans-serif;
t         --fecha-font-size:        25px;
t         --hora-font-size:         25px;
t         --consumo-font-size:      25px;
t         --fecha-color:            rgb(0, 0, 0);
t         --hora-color:             rgb(0, 0, 0);
t         --consumo-color:          rgb(0, 0, 0);
t         --margen-topbar-izq:      10px;
t         --margen-topbar-der:      10px;
t         --margen-topbar-sup:      10px;
t         --margen-topbar-inf:      10px;
t         --distancia-fecha-hora:   30px;
t         --distancia-Hora-Consumo: 1 1 auto;
#         Titulo
t         --titulo-font-family: 'Arial', sans-serif;
t         --titulo-font-size:   40px;
t         --titulo-color:       rgb(0, 0, 0);
t         --margen-titulo-sup:  5px;
t         --margen-titulo-inf:  5px;
#         Body - General (excepto botones)
t         --margen-izq:      30%;
t         --margen-der:      30%;
t         --margen-body-sup: 32px;
t         --margen-body-inf: 32px;
#         Bloque Jugadores
t         --negras-font-family:              'Arial', sans-serif;
t         --blancas-font-family:             'Arial', sans-serif;
t         --negras-font-size:                25px;
t         --blancas-font-size:               25px;
t         --negras-color:                    rgb(0, 0, 0);
t         --blancas-color:                   rgb(0, 0, 0);
t         --distancia-jugador-nombreJugador: 10px;
t         --distancia-blancas-negras:        1 1 0;
t         --jugador-input-min-width:         0;
t         --jugador-input-box-sizing:        border-box;
t         --jugador-input-width:             200px;
#         Bloque Tiempo partida
t         --tiempo-label-font-family:      'Arial', sans-serif;
t         --tiempo-label-font-size:        25px;
t         --tiempo-label-color:            rgb(0, 0, 0);
t         --distancia-tiempoPartida-boton: 10px;
t         --distancia-boton-tiempoPartida: 3px;
t         --tamaño-celda-tiempoPartida-incremento: 50px;
t         --alineacion-celda-tiempoPartida-incremento: center;
# #         Bloque Incremento
# t         --incremento-label-font-family:     'Arial', sans-serif;
# t         --incremento-label-font-size:       25px;
# t         --incremento-label-color:           rgb(0, 0, 0);
# t         --distancia-checkbox-tagIncremento: 3px;
# t         --distancia-incremento-boton:       10px;
# t         --distancia-boton-tiempoIncremento: 3px;
# #         Bloque Ayuda
# t         --ayuda-label-font-family:     'Arial', sans-serif;
# t         --ayuda-label-font-size:       25px;
# t         --ayuda-label-color:           rgb(0, 0, 0);
# t         --distancia-checkbox-tagAyuda: 3px;
#         Bloque Botones
t         --margen-izq-botones:   20%;
t         --margen-der-botones:   20%;
t         --distancia-pause-stop: 5px;
t       }
t
t       .bloqueFechaHoraConsumo {
t         display:         flex;
t         align-items:     center;
t         justify-content: flex-start;
t         margin-top:      var(--margen-topbar-sup);
t         margin-bottom:   var(--margen-topbar-inf);
t         margin-left:     var(--margen-topbar-izq);
t         margin-right:    var(--margen-topbar-der);
t         padding:         0;
t         width:           auto;
t         box-sizing:      border-box;
t       }
t       .fecha {
t         font-family:  var(--fecha-font-family);
t         font-size:    var(--fecha-font-size);
t         color:        var(--fecha-color);
t         margin-right: var(--distancia-fecha-hora);
t       }
t       .hora {
t         font-family: var(--hora-font-family);
t         font-size:   var(--hora-font-size);
t         color:       var(--hora-color);
t       }
t       .espacio-hora-consumo {
t         flex: var(--distancia-Hora-Consumo);
t       }
t       .consumo {
t         font-family:  var(--consumo-font-family);
t         font-size:    var(--consumo-font-size);
t         color:        var(--consumo-color);
t       }
t       .titulo-nueva-partida {
t         font-family:   var(--titulo-font-family);
t         font-size:     var(--titulo-font-size);
t         color:         var(--titulo-color);
t         margin-top:    var(--margen-titulo-sup);
t         margin-bottom: var(--margen-titulo-inf);
t       }
t       .bloque-jugadores {
t         display:       flex;
t         align-items:   center;
t         margin-top:    var(--margen-body-sup);
t         margin-bottom: var(--margen-body-inf);
t         margin-left:   var(--margen-izq);
t         margin-right:  var(--margen-der);
t       }
t       .blancas {
t         font-family:  var(--blancas-font-family);
t         font-size:    var(--blancas-font-size);
t         color:        var(--blancas-color);
t         margin-right: var(--distancia-jugador-nombreJugador);
t       }
t       .negras {
t         font-family:  var(--negras-font-family);
t         font-size:    var(--negras-font-size);
t         color:        var(--negras-color);
t         margin-right: var(--distancia-jugador-nombreJugador);
t       }
t       .espacio-blancas-negras {
t         flex:   var(--distancia-blancas-negras);
t         min-width:   0;
t       }
t       .jugador1 input,
t       .jugador2 input {
t         width:      var(--jugador-input-width);
t         min-width:  var(--jugador-input-min-width);
t         box-sizing: var(--jugador-input-box-sizing);
t       }
t       .bloque-tiempo {
t         display:       flex;
t         align-items:   center;
t         margin-left:   var(--margen-izq);
t         margin-right:  var(--margen-der);
t         margin-top:    var(--margen-body-sup);
t         margin-bottom: var(--margen-body-inf);
t       }
t       .tiempo-label {
t         font-family:  var(--tiempo-label-font-family);
t         font-size:    var(--tiempo-label-font-size);
t         color:        var(--tiempo-label-color);
t         margin-right: var(--distancia-tiempoPartida-boton);
t       }
t       .tiempo-controles {
t         display:     flex;
t         align-items: center;
t       }
t       .tiempo-controles button {
t       }
t       .tiempo-controles .time-cell {
t       }
t       .time-cell {
t         min-width:    var(--tamaño-celda-tiempoPartida-incremento);
t         text-align:   var(--alineacion-celda-tiempoPartida-incremento);
t         margin-left:  var(--distancia-boton-tiempoPartida);
t         margin-right: var(--distancia-boton-tiempoPartida);
t       }
# t       .bloque-incremento {
# t         display:       flex;
# t         align-items:   center;
# t         margin-left:   var(--margen-izq);
# t         margin-right:  var(--margen-der);
# t         margin-top:    var(--margen-body-sup);
# t         margin-bottom: var(--margen-body-inf);
# t       }
# t       .incremento-label {
# t         font-family:  var(--tiempo-label-font-family);
# t         font-size:    var(--tiempo-label-font-size);
# t         color:        var(--tiempo-label-color);
# t         margin-right: var(--distancia-tiempoPartida-boton);
# t         margin-right: var(--distancia-incremento-boton);
# t       }
# t       .incremento-controles {
# t         display:     flex;
# t         align-items: center;
# t       }
# t       .incremento-controles button {
# t       }
# t       .incremento-controles .time-cell {
# t       }
# t       .bloque-ayuda {
# t         display:       flex;
# t         align-items:   center;
# t         margin-left:   var(--margen-izq);
# t         margin-right:  var(--margen-der);
# t         margin-top:    var(--margen-body-sup);
# t         margin-bottom: var(--margen-body-inf);
# t       }
# t       .ayuda-label {
# t         font-family: var(--ayuda-label-font-family);
# t         font-size:   var(--ayuda-label-font-size);
# t         color:       var(--ayuda-label-color);
# t       }
t       .bloque-botones {
t         display:         flex;
t         align-items:     center;
t         justify-content: flex-start;
t         margin-left:   var(--margen-izq-botones);
t         margin-right:  var(--margen-der-botones);
t         margin-top:    var(--margen-body-sup);
t         margin-bottom: var(--margen-body-inf);
t       }
t       .btnIniciar {
t         width:            200px;
t         height:           40px;
t         font-size:        20px;
t         font-family:      'Arial Black', Arial, sans-serif;
t         color:            white;
t         background-color: rgb(58, 186, 33);
t       }
t       .btnPausar {
t         width:            200px;
t         height:           40px;
t         font-size:        20px;
t         font-family:      'Arial Black', Arial, sans-serif;
t         color:            white;
t         background-color: rgb(255, 255, 0);
t       }
t       .btnSuspender {
t         width:            200px;
t         height:           40px;
t         font-size:        20px;
t         font-family:      'Arial Black', Arial, sans-serif;
t         color:            white;
t         background-color: rgb(255, 150, 0);
t       }
t       .btnRendirse {
t         width:            280px;
t         height:           40px;
t         font-size:        20px;
t         font-family:      'Arial Black', Arial, sans-serif;
t         color:            white;
t         background-color: rgb(255, 0, 0);
t       }
t       button:disabled {
t         background-color: #cccccc !important;
t         color: #888888 !important;
t         border-color: #aaaaaa !important;
t         cursor: not-allowed;
t         opacity: 0.7;
t       }
t     </style>
t   </head>
t   <body>
t     <div class="bloqueFechaHoraConsumo">
t       <span id="dateOut" class="fecha">%s</span>
t       <span id="timeOut" class="hora">%s</span>
t       <span class="espacio-hora-consumo"></span>
t       <span id="consumoActual" class="consumo">Consumo actual: </span>
t     </div>
t     <h2 class="titulo-nueva-partida" align="center"><br>Nueva partida</h2>
t     <form action="nuevaPartida.cgi" method="post" name="nuevaPartida" onsubmit="return submitFormAjax();">
t       <div class="bloque-jugadores">
t         <span class="blancas"><b>Blancas</b></span>
t         <span class="jugador1">
t           <input type="text" id="player1Name" name="player1Name" placeholder="Nombre jugador" maxlength="12">
t         </span>
t         <span class="espacio-blancas-negras"></span>
t         <span class="negras"><b>Negras</b></span>
t         <span class="jugador2">
t           <input type="text" id="player2Name" name="player2Name" placeholder="Nombre jugador" maxlength="12">
t         </span>
t       </div>
t       <div class="bloque-tiempo">
t         <span class="tiempo-label"><b>Tiempo partida</b></span>
t         <span class="tiempo-controles">
t           <button type="button" onclick="changeMatchTime(-5)">&#8595;</button>
t           <span id="matchTime" class="time-cell">15:00</span>
t           <button type="button" onclick="changeMatchTime(5)">&#8593;</button>
t         </span>
t       </div>
# t       <div class="bloque-incremento">
# t         <span class="incremento-label">
# t           <input type="checkbox" id="incrementEnabled" onchange="updateIncrementHidden()">
# t           <input type="hidden" id="incrementEnabledHidden" name="incrementEnabled" value="false">
# t           <b>Incremento</b>
# t         </span>
# t         <span class="incremento-controles">
# t           <button type="button" onclick="changeIncrement(-2)" id="incDown">&#8595;</button>
# t           <span id="incrementTime" class="time-cell">+2</span>
# t           <button type="button" onclick="changeIncrement(2)" id="incUp">&#8593;</button>
# t         </span>
# t       </div>
# t       <div class="bloque-ayuda">
# t         <span class="ayuda-label">
# t           <input type="checkbox" id="ayuda" onchange="updateAyudaHidden()">
# t           <input type="hidden" id="ayudaHidden" name="ayuda" value="false">
# t           <label for="ayuda"><b>Ayuda</b></label>
# t         </span>
# t       </div>
t       <div class="bloque-botones">
t             <button type="submit" id="btnIniciar" class="btnIniciar" onclick="return iniciar(event);">
t               &#9654; INICIAR</button>
t             <button type="submit" id="btnPausar" class="btnPausar" onclick="pausar()" disabled>
t               | |  PAUSAR</button>
t             <button type="submit" id="btnSuspender" class="btnSuspender" onclick="suspender()" disabled>
t               &#9632; SUSPENDER</button>
t             <button type="submit" id="btnRendirse" class="btnRendirse" onclick="rendirse()" disabled>
t               <svg width="18" height="18" viewBox="0 0 18 18" style="vertical-align:middle; margin-right:4px;">
t                 <rect x="2" y="2" width="2" height="14" fill="#888"/>
t                 <polygon points="4,2 16,5 4,8" fill="#fff" stroke="#888" stroke-width="0.5"/>
t               </svg>
t               BLANCAS SE RINDE
t             </button>
t       </div>
t       <input type="hidden" id="matchTimeInput" name="matchTime" value="15">
# t       <input type="hidden" id="incrementTimeInput" name="incrementTime" value="2">
t       <a href="index.htm" style="display:block; margin-top:50px; text-align:center;">Menu principal</a>
t     </form>
t   </body>
t </html>
. End of script must be closed with period.
