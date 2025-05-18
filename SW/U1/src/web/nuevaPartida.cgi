t  <html>
t    <head>
t      <title>Nueva partida</title>
t      <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t      <script language=JavaScript type="text/javascript">
t        var formUpdate = new periodicObj("date.cgx", 1000);
t        function updateDateFields() 
t        {
t          timeVal = document.getElementById("timeOut").value;
t          document.getElementById("timeOut").value = timeVal;
t          dateVal = document.getElementById("dateOut").value;
t          document.getElementById("dateOut").value = dateVal;
t        }
t        function periodicUpdateDate() 
t        {
t          if (document.getElementById("dateChkBox").checked == true) {
t            updateMultiple(formUpdate, updateDateFields);
t            date_elTime = setTimeout(periodicUpdateDate, formUpdate.period);
t          } 
t          else {
t            clearTimeout(date_elTime);
t          }
t        }
t      </script>
t    </head>
i    pg_header.inc
t    <h2 align=center><br>Nueva partida</h2>
t    <p>
t      <font size="2">
t      </font>
t    </p>
t    <form action=nuevaPartida.cgi method=post name=nuevaPartida>
t      <input type=hidden value="date" name=pg>
t        <table border=0 width=99%>
t          <font size="3">
t            <tr bgcolor=#aaccff>
t              <th width=40%>Item</th>
t              <th width=60%>Setting</th>
t            </tr>
# Here begin data setting which is formatted in HTTP_CGI.C module
t            <tr>
t              <td>Time</td>
t              <td>
t                <input type="text" readonly style="background-color: transparent; border: 0px"
c h 1 size="10" id="timeOut" value="%s"></td></tr>
t            <tr>
t              <td>Date</td>
t              <td>
t                <input type="text" readonly style="background-color: transparent; border: 0px"
c h 2 size="10" id="dateOut" value="%s"></td></tr>
t            <tr>
t              <td>Time</td>
c h 3 <td><input type=text name=timeIn size=20 maxlength=9 value="%s"></td></tr>
t            <tr>
t              <td>Date</td>
c h 4 <td><input type=text name=dateIn size=20 maxlength=9 value="%s"></td></tr>
t          </font>
t        </table>
# Here begin button definitions
t        <p align=center>
t          <input type=submit name=set value="Send" id="sbm">
t          <input type=reset           value="Undo">
t          <input type=button value="Refresh" onclick="updateMultiple(formUpdate,updateDateFields)">
t          Periodic:<input type="checkbox" id="dateChkBox" onclick="periodicUpdateDate()">
t        </p>
t    </form>
. End of script must be closed with period.
