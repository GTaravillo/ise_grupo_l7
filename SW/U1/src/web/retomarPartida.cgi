t <html>
t   <head>
t     <title>MasterChess - Retomar Partida</title>
t     <script language="JavaScript"
t             type="text/javascript"
t             src="xml_http.js"></script>
t     <style>
t       .info-cell {
t         font-size: 15px;
t         background-color: transparent;
t         border: 0px;
t       }
t       .time-cell {
t         display: inline-block;
t         width: 40px;
t         text-align: center;
t         border: 1px solid #ccc;
t         background: #f9f9f9;
t         border-radius: 4px;
t       }
t       .header-date {
t         margin-left: 20px;    /* Distance from left limit */
t         margin-right: 10px;   /* Space to time */
t       }
t       .header-time {
t         /* No margin needed here */
t       }
t       .header-consumo {
t         margin-left: auto;    /* Pushes Consumo actual to the right */
t         margin-right: 20px;   /* Distance to right limit */
t       }
t       .header-block {
t         display: flex;
t         align-items: center;
t         margin-top: 30px;          /* Space from top of page */
t         margin-bottom: 32px;       /* Space to "Nueva Partida" */
t         width: 100%;
t       }
t       .main-flex-container {
t         display: flex;
t         flex-direction: column;
t         gap: 12px;
t         width: 100%;
t         max-width: 600px;
t         margin: 0 auto;
t       }
t       .row {
t         display: flex;
t         flex-wrap: wrap;
t         gap: 10px;
t         align-items: center;
t       }
t       .row > div {
t         flex: 1 1 0;
t         min-width: 120px;
t       }
t       .wide-cell {
t         flex: 3 1 0;
t         display: flex;
t         align-items: center;
t         gap: 8px;
t       }
t       .hidden {
t         display: none !important;
t       }
t       h2 {
t         margin-top: 32px;    /* Distance from header to title */
t       }
t       @media (max-width: 600px) {
t         .main-flex-container {
t           max-width: 100%;
t         }
t         .row {
t           flex-direction: column;
t           gap: 6px;
t         }
t         .wide-cell {
t           flex-direction: column;
t           align-items: stretch;
t         }
t       }
t     </style>
t   </head>
t   <body>
## <Fecha, hora y consumo>
t    <div class="header-block">
t      <span id="dateOut" class="info-cell header-date">%s</span>
t      <span id="timeOut" class="info-cell header-time">%s</span>
t      <span id="consumoActual" class="info-cell header-consumo">Consumo actual:</span>
t    </div>
## </Fecha, hora y consumo>
## <Titulo página>
t    <h2 align="center">
t      <br>Nueva Partida
t    </h2>
## </Titulo página>
t     <!-- BUTTONS (hidden by default, moved by JS) -->
t     <button type="button"
t             id="startBtn"
t             class=""
t             onclick="submitFormAjax()">
t       <b>START</b>
t     </button>
t     <button type="button"
t             id="pauseBtn"
t             class="hidden"
t             onclick="pauseGame()">
t       <b>PAUSE</b>
t     </button>
t     <button type="button"
t             id="stopBtn"
t             class="hidden"
t             onclick="stopGame()">
t       <b>STOP</b>
t     </button>
t     <form action="retomarPartida.cgi"
t           method="post"
t           name="cgi"
t           onsubmit="event.preventDefault();">
# <DATOS>
t       <input type="text"
t              id="player1Name"
t              name="player1Name"
t              placeholder="Nombre jugador">
t       <input type="text"
t              id="player2Name"
t              name="player2Name"
t              placeholder="Nombre jugador">
t       <input type="checkbox"
t              id="incrementEnabled"
t              checked
t              onchange="toggleIncrement()">
t       <input type="checkbox"
t              id="ayuda"
t              name="ayuda"
t              checked>
t       <input type="hidden"
t              id="matchTimeInput"
t              name="matchTime"
t              value="15">
t       <input type="hidden"
t              id="incrementTimeInput"
t              name="incrementTime"
t              value="2">
# </DATOS>
# <LAYOUT>
t       <div class="main-flex-container">
t         <div class="row">
t           <div><b>Blancas</b></div>
t           <div id="cell-player1"></div>
t           <div><b>Negras</b></div>
t           <div id="cell-player2"></div>
t         </div>
t         <div class="row">
t           <div><b>Tiempo partida</b></div>
t           <div class="wide-cell">
t             <button type="button"
t                     onclick="changeMatchTime(-5)">
t               &#8595;
t             </button>
t             <span id="matchTime"
t                   class="time-cell">
t               15:00
t             </span>
t             <button type="button"
t                     onclick="changeMatchTime(5)">
t               &#8593;
t             </button>
t           </div>
t         </div>
t         <div class="row">
t           <div>
t             <span id="cell-incrementEnabled"></span>
t             <b>Incremento</b>
t           </div>
t           <div class="wide-cell">
t             <button type="button"
t                     onclick="changeIncrement(-2)"
t                     id="incDown">
t               &#8595;
t             </button>
t             <span id="incrementTime"
t                   class="time-cell">
t               +2
t             </span>
t             <button type="button"
t                     onclick="changeIncrement(2)"
t                     id="incUp">
t               &#8593;
t             </button>
t           </div>
t         </div>
t         <div class="row">
t           <div>
t             <span id="cell-ayuda"></span>
t             <label for="ayuda">
t               <b>Ayuda</b>
t             </label>
t           </div>
t         </div>
t         <div class="row">
t           <div id="cell-startBtn" class="wide-cell"></div>
t         </div>
t       </div>
t     </form>
# </LAYOUT>
t   </body>
t </html>
. End of script must be closed with period.