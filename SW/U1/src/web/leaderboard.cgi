t <html>
t   <head>
t       <title>MasterChess - Leaderboard</title>
#####################################################
t <script language=JavaScript type="text/javascript" src="xml_http.js">
t </script>
t <script language=JavaScript type="text/javascript">
# Define URL and refresh timeout
t var formUpdate = new periodicObj("date.cgx", 1000);
t function updateDateFields() {
t  timeVal = document.getElementById("timeOut").value;
t  document.getElementById("timeOut").value = timeVal;
t  dateVal = document.getElementById("dateOut").value;
t  document.getElementById("dateOut").value = dateVal;
t }
t function periodicUpdateDate() {
t  if (document.getElementById("dateChkBox").checked == true) {
t    updateMultiple(formUpdate, updateDateFields);
t    date_elTime = setTimeout(periodicUpdateDate, formUpdate.period);
t  } else {
t    clearTimeout(date_elTime);
t  }
t }
t </script>
#####################################################
t   </head>
# i   pg_header.inc
t   <h2 align=center>
t       <br>Date Control
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
t   <form action=date.cgi method=post name=cgi>
t       <input type=hidden value="date" name=pg>
t           <table border=0 width=99%>
t               <font size="3">
t                   <tr bgcolor=#aaccff>
t                       <th width=40%>Item</th>
t                       <th width=60%>Setting</th>
t                   </tr>
# Here begin data setting which is formatted in HTTP_CGI.C module
t                   <tr>
t                       <td>
# t                           <img src=pabb.gif>Time
t                       </td>
t                       <td>
t <input type="text" readonly style="background-color: transparent; border: 0px"
c h 1 size="10" id="timeOut" value="%s"></td></tr>
t                   <tr>
t                       <td>
# t                           <img src=pabb.gif>Date
t                       </td>
t                       <td>
t <input type="text" readonly style="background-color: transparent; border: 0px"
c h 2 size="10" id="dateOut" value="%s"></td></tr>
t                   <tr>
t                       <td>
# t                           <img src=pabb.gif>Time
t                       </td>
c h 3 <td><input type=text name=timeIn size=20 maxlength=9 value="%s"></td></tr>
t                   <tr>
t                       <td>
# t                           <img src=pabb.gif>Date
t                       </td>
c h 4 <td><input type=text name=dateIn size=20 maxlength=9 value="%s"></td></tr>
t               </font>
t           </table>
# Here begin button definitions
t <p align=center>
t <input type=submit name=set value="Send" id="sbm">
t <input type=reset           value="Undo">
t <input type=button value="Refresh" onclick="updateMultiple(formUpdate,updateDateFields)">
t Periodic:<input type="checkbox" id="dateChkBox" onclick="periodicUpdateDate()">
t </p></form>
# i   pg_footer.inc
. End of script must be closed with period.
