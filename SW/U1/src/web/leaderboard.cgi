t  <html>
t    <head>
t      <title>MasterChess - Leaderboard</title>
t      <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t    </head>
## <Fecha, hora y consumo>
t    <table 
t      style = "position: absolute; 
t               top: 10px; 
t               left: 10px;">
t      <tr>
# <Fecha>
t        <td>
t          <span 
t            id    = "dateOut"
t            style = "font-size: 12px; 
t                     background-color: transparent; 
t                     border: 0px">
t            %s
t          </span>
t        </td>
# <Hora>
t        <td>
t          <span 
t            id    = "timeOut"
t            style = "font-size: 12px; 
t                     background-color: transparent; 
t                     border: 0px">
t            %s
t          </span>
t        </td>
# </Hora>
# <Consumo>
t        <td 
t          style = "text-align: right; 
t                   width: 100%;">
t          <span 
t            id    = "consumoActual"
t            style = "font-size: 12px; 
t                     background-color: transparent; 
t                     border: 0px">
t            Consumo actual
t          </span>
t        </td>
# </Consumo>
# </Fecha>
t      </tr>
t    </table>
## </Fecha, hora y consumo>
## <Titulo página>
t    <h2 
t      align = "center">
t      <br>Nueva Partida
t    </h2>
## </Titulo página>
t    <form 
t      action = leaderboard.cgi 
t      method = post 
t      name   = cgi
t      onsubmit = event.preventDefault();>
# <Nombre jugadores>
t      <table>
t        <tr>
t          <td>
t            <b>Blancas
t          </td>
t          <td 
t           style = "padding-right: 30px;">
t            <input 
t              type = "text" 
t              id   = "player1Name" 
t              name = "player1Name" 
t              placeholder = "Nombre jugador">
t          </td>
t          <td>
t            <b>Negras
t          </td>
t          <td>
t            <input 
t              type = "text" 
t              id   = "player2Name" 
t              name = "player2Name" 
t              placeholder = "Nombre jugador">
t          </td>
t        </tr>
t      </table> 
# </Nombre jugadores>
# <Tiempo partida>
t      <table>
t        <tr>
t          <td>
t            <b>Tiempo partida
t          </td>
t          <td>
t            <button 
t              type    = "button" 
t              onclick = changeMatchTime(-5)>
t              &#8595;
t            </button>
t            <span 
t              id    = "matchTime" 
t              style = "display:inline-block; 
t                       width:40px; 
t                       text-align:center; 
t                       border:1px solid #ccc; 
t                       background:#f9f9f9; 
t                       border-radius:4px;">
t              15:00
t            </span>
t            <button 
t              type    = "button" 
t              onclick = changeMatchTime(5)>
t              &#8593;
t            </button>
t          </td>
t        </tr>
# </Tiempo partida>
t        <tr>
t          <td>
t            <input 
t              type = "checkbox" 
t              id   = "incrementEnabled" 
t              checked onchange = toggleIncrement() 
t              style = "vertical-align: middle;">
t            <b>Incremento
t          </td>
t          <td>
t            <button 
t              type    = "button" 
t              onclick = changeIncrement(-2) 
t              id      = "incDown">
t              &#8595;
t            </button>
t            <span 
t              id    = "incrementTime"
t              style = "display:inline-block; 
t                       width:40px; 
t                       text-align:center; 
t                       border:1px solid #ccc; 
t                       background:#f9f9f9; 
t                       border-radius:4px;">
t              +2
t            </span>
t            <button 
t              type    = "button" 
t              onclick = changeIncrement(2) 
t              id      = "incUp">
t              &#8593;
t            </button>
t          </td>
t        </tr>
t        <tr>
t          <td>
t            <input 
t              type = "checkbox" 
t              id   = "ayuda" 
t              name = "ayuda" 
t              checked>
t            <label 
t              for = "ayuda">
t              <b>Ayuda
t            </label>
t          </td>
t        </tr>
t        <input 
t          type  = "hidden" 
t          id    = "matchTimeInput" 
t          name  = "matchTime" 
t          value = "15">
t        <input 
t          type  = "hidden" 
t          id    = "incrementTimeInput" 
t          name  = "incrementTime" 
t          value = "2">
t      </table>
t    </form>
t    <button 
t      type    = "button" 
t      onclick = submitFormAjax()>
t      <b>START
t    </button>
. End of script must be closed with period.