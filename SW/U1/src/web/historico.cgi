t <!DOCTYPE html>
t <html>
t <head>
t   <title>Historico de Partidas</title>
t </head>
t <h2 align=center>Historico de Partidas</h2>
t <form action=historico.cgi method=post name=historico>
t   <table border=1 width=99% align=center>
t     <tr bgcolor=#aaccff>
t       <th width=20%%>Fecha</th>
t       <th width=30%%>Blancas</th>
t       <th width=30%%>Negras</th>
t       <th width=20%%>Victoria</th>
t     </tr>
# 10 rows of data
t     <tr>
t       <td>
t         <span>%s</span>
t       </td>
t       <td>
t         <input type=text name=nombreBlancas1 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreNegras1 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=victoria1 size=8 maxlength=8 value="%s" readonly>
t       </td>
t     </tr>
t     <tr>
t       <td>
t         <input type=text name=fechaPartida2 size=9 maxlength=8 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreBlancas2 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreNegras2 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=victoria2 size=8 maxlength=8 value="%s" readonly>
t       </td>
t     </tr>
t     <tr>
t       <td>
t         <input type=text name=fechaPartida3 size=9 maxlength=8 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreBlancas3 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreNegras3 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=victoria3 size=8 maxlength=8 value="%s" readonly>
t       </td>
t     </tr>
t     <tr>
t       <td>
t         <input type=text name=fechaPartida4 size=9 maxlength=8 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreBlancas4 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreNegras4 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=victoria4 size=8 maxlength=8 value="%s" readonly>
t       </td>
t     </tr>
t     <tr>
t       <td>
t         <input type=text name=fechaPartida5 size=9 maxlength=8 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreBlancas5 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreNegras5 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=victoria5 size=8 maxlength=8 value="%s" readonly>
t       </td>
t     </tr>
t     <tr>
t       <td>
t         <input type=text name=fechaPartida6 size=9 maxlength=8 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreBlancas6 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreNegras6 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=victoria6 size=8 maxlength=8 value="%s" readonly>
t       </td>
t     </tr>
t     <tr>
t       <td>
t         <input type=text name=fechaPartida7 size=9 maxlength=8 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreBlancas7 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreNegras7 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=victoria7 size=8 maxlength=8 value="%s" readonly>
t       </td>
t     </tr>
t     <tr>
t       <td>
t         <input type=text name=fechaPartida8 size=9 maxlength=8 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreBlancas8 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreNegras8 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=victoria8 size=8 maxlength=8 value="%s" readonly>
t       </td>
t     </tr>
t     <tr>
t       <td>
t         <input type=text name=fechaPartida9 size=9 maxlength=8 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreBlancas9 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreNegras9 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=victoria9 size=8 maxlength=8 value="%s" readonly>
t       </td>
t     </tr>
t     <tr>
t       <td>
t         <input type=text name=fechaPartida10 size=9 maxlength=8 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreBlancas10 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=nombreNegras10 size=12 maxlength=12 value="%s" readonly>
t       </td>
t       <td>
t         <input type=text name=victoria10 size=8 maxlength=8 value="%s" readonly>
t       </td>
t     </tr>
t   </table>
t   <a href="index.htm" style="display:block; margin-top:50px; text-align:center;">Menu principal</a>
t </form>
. End of script must be closed with period.
