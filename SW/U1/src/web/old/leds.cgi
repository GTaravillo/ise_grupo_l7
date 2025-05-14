t <html><head><title>LED Control</title>
t <script language=JavaScript>
t function AllSW(st) {
t  for(i=0;i<document.form1.length;i++) {
t   if(document.form1.elements[i].type=="checkbox"){
t    document.form1.elements[i].checked=st;
t   }
t  }
t  document.form1.submit();
t }
t </script></head>
i pg_header.inc
t <h2 align=center><br>Control LEDs on the board</h2>
t <p><font size="2">This page shows you how to use the following http form <b>input</b> objects:
t  <b>checkbox</b>, <b>select</b> and <b>button</b>. It uses also a simple <b>Java Script</b>
t  function to check/uncheck all checkboxes and submit the data.<br><br>
t  This Form uses a <b>POST</b> method to send data to a Web server.</font></p>
t <form action=leds.cgi method=post name=form1>
t <input type=hidden value="led" name=pg>
t <table border=0 width=99%><font size="3">
t <tr bgcolor=#aaccff>
t  <th width=40%>Item</th>
t  <th width=60%>Setting</th></tr>
t <td><img src=pabb.gif>LED control:</td>
t <td><select name="ctrl" onchange="submit();">
c b c <option %s>Browser</option><option %s>Running Lights</option></select></td></tr>
t <tr><td><img src=pabb.gif>LED:</td>
t <td><table><tr valign="middle">
# Here begin the 'checkbox' definitions
t <td width="5%"></td>
c b 2 <td><input type=checkbox name=LD3 OnClick="submit();" %s>LD3(R)</td>
c b 1 <td><input type=checkbox name=LD2 OnClick="submit();" %s>LD2(B)</td>
c b 0 <td><input type=checkbox name=LD1 OnClick="submit();" %s>LD1(G)</td>
t </font></table></td></tr>
t <tr><td><img src=pabb.gif>All LED diodes On or OFF</td>
t <td><input type=button value="&nbsp;&nbsp;ON&nbsp;&nbsp;&nbsp;" onclick="AllSW(true)">
t <input type=button value="&nbsp;&nbsp;OFF&nbsp;&nbsp;" onclick="AllSW(false)"></td></tr>
t </table></form>
i pg_footer.inc
. End of script must be closed with period.


