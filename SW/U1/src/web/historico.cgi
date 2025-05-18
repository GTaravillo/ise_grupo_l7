t <html>
t   <head>
t       <title>LCD Control</title>
t   </head>
i   pg_header.inc
t   <h2 align=center>
t       <br>LCD Module Control
t   </h2>
t   <p>
t       <font size="2">
t           This page allows you to change a message displayed on 
t           <b>LCD Module</b>.
t           Simply enter the text which you want to display and click 
t           <b>Send</b>.
t           <br>
t           <br>
t           This Form uses a 
t           <b>POST</b> 
t           method to send data to a Web server.
t       </font>
t   </p>
t   <form action=lcd.cgi method=post name=cgi>
t       <input type=hidden value="lcd" name=pg>
t           <table border=0 width=99%>
t               <font size="3">
t                   <tr bgcolor=#aaccff>
t                       <th width=40%>Item</th>
t                       <th width=60%>Setting</th>
t                   </tr>
# Here begin data setting which is formatted in HTTP_CGI.C module
t                   <tr>
t                       <td>
t                           <img src=pabb.gif>Line 1 Text
t                       </td>
#t                       <td>
c f 1 <td><input type=text name=lcd1 size=20 maxlength=21 value="%s"></td></tr>
#t                       </td>
#t                   </tr>
t                   <tr>
t                       <td>
t                           <img src=pabb.gif>Line 2 Text
t                       </td>
#t                       <td>
c f 2 <td><input type=text name=lcd2 size=20 maxlength=21 value="%s"></td></tr>
#t                       </td>
#t                   </tr>
t               </font>
t           </table>
# Here begin button definitions
t <p align=center>
t <input type=submit name=set value="Send" id="sbm">
t <input type=reset value="Undo">
t </p></form>
#t   </form>
i   pg_footer.inc
. End of script must be closed with period.
