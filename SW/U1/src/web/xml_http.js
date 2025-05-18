function updateMultiple(formUpd, callBack, userName, userPassword) 
{
  xmlHttp = GetXmlHttpObject();
  if(xmlHttp == null) 
  {
    alert("XmlHttp not initialized!");
    return 0;
  }
  xmlHttp.onreadystatechange = responseHandler; 
  xmlHttp.open("GET", formUpd.url, true, userName, userPassword);
  xmlHttp.send(null);

  function responseHandler()
  {
    if(xmlHttp.readyState == 4) 
    { //response ready 
      if(xmlHttp.status == 200) 
      { //handle received data
        var xmlDoc = xmlHttp.responseXML;
        if(xmlDoc == null)
          return 0;
        try 
        {  //catching IE bug
          processResponse(xmlDoc);
        }
        catch(e) 
        {
          return 0;
        }
        /* Callback function for custom update. */
        if (callBack != undefined)
          callBack();
      }
      else if(xmlHttp.status == 401)
        alert("Error code 401: Unauthorized");
      else if(xmlHttp.status == 403)
        alert("Error code 403: Forbidden");
      else if(xmlHttp.status == 404)
        alert("Error code 404: URL not found!");
    }
  }
}

function processResponse(xmlDoc) 
{
  textElementArr = xmlDoc.getElementsByTagName("text");
  for(var i = 0; i < textElementArr.length; i++) 
  {
    try 
    {
      elId    = textElementArr[i].childNodes[0].childNodes[0].nodeValue;
      elValue = textElementArr[i].childNodes[1].childNodes[0].nodeValue;
      document.getElementById(elId).value = elValue;
    }
    catch(error) 
    {
      if(elId == undefined)
      {
        continue;
      }
      else if(elValue == undefined) 
      {
        elValue = "";
        document.getElementById(elId).textContent = elValue;
      }
    }
  }
  checkboxElementArr = xmlDoc.getElementsByTagName("checkbox");
  for(var i = 0; i < checkboxElementArr.length; i++) 
  {
    try 
    {
      elId = checkboxElementArr[i].childNodes[0].childNodes[0].nodeValue;
      elValue = checkboxElementArr[i].childNodes[1].childNodes[0].nodeValue;
      if(elValue.match("true"))
        document.getElementById(elId).checked = true;
      else
        document.getElementById(elId).checked = false;
    }
    catch(error) 
    {
      if(elId == undefined) 
      {
        continue;
      }
      else if(elValue == undefined) //we leave current state
        continue;
    }
  }
  selectElementArr = xmlDoc.getElementsByTagName("select");
  for(var i = 0; i < selectElementArr.length; i++) 
  {
    try 
    {
      elId = selectElementArr[i].childNodes[0].childNodes[0].nodeValue;
      elValue = selectElementArr[i].childNodes[1].childNodes[0].nodeValue;
      document.getElementById(elId).value = elValue;
      if(elValue.match("true"))
        document.getElementById(elId).selected = true;
      else
        document.getElementById(elId).selected = false;
    }
    catch(error) 
    {
      if(elId == undefined) 
      {
        continue;
      }
      else if(elValue == undefined) 
      {
        elValue = "";
        document.getElementById(elId).value = elValue;
      }
    }
  }
  radioElementArr = xmlDoc.getElementsByTagName("radio");
  for(var i = 0; i < radioElementArr.length; i++) 
  {
    try 
    {
      elId = radioElementArr[i].childNodes[0].childNodes[0].nodeValue;
      elValue = radioElementArr[i].childNodes[1].childNodes[0].nodeValue;
      if(elValue.match("true"))
        document.getElementById(elId).checked = true;
      else
        document.getElementById(elId).checked = false;
    }
    catch(error) 
    {
      if(elId == undefined) 
      {
        continue;
      }
      else if(elValue == undefined) //we leave current state
        continue;
    }
  }
}

/* XMLHttpRequest object specific functions */
function GetXmlHttpObject() 
{ //init XMLHttp object
  var xmlHttp=null;
  try 
  {
    xmlHttp=new XMLHttpRequest(); // Firefox, Opera 8.0+, Safari
  }
  catch (e) 
  {
    try 
    {   // Internet Explorer
      xmlHttp=new ActiveXObject("Msxml2.XMLHTTP");
    }
    catch (e) 
    {
      xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
    }
  }
  return xmlHttp;
}

function periodicObj(url, period) 
{
  this.url = url;
  this.period = (typeof period == "undefined") ? 0 : period;
}

function updateTimeField() 
{
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() 
  {
    if (xhr.readyState == 4 && xhr.status == 200) 
    {
      var xmlDoc = xhr.responseXML;
      if (xmlDoc) 
      {
        var textNode = xmlDoc.getElementsByTagName("text")[0];
        if (textNode) 
        {
          var id = textNode.getElementsByTagName("id")[0].textContent;
          var value = textNode.getElementsByTagName("value")[0].textContent;
          document.getElementById(id).textContent = value;
        }
      }
    }
  };
  xhr.open("GET", "currentTime.cgx", true);
  xhr.send();
}

function updateDateField() 
{
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() 
  {
    if (xhr.readyState == 4 && xhr.status == 200) 
    {
      var xmlDoc = xhr.responseXML;
      if (xmlDoc) 
      {
        var textNode = xmlDoc.getElementsByTagName("text")[0];
        if (textNode) 
        {
          var id = textNode.getElementsByTagName("id")[0].textContent;
          var value = textNode.getElementsByTagName("value")[0].textContent;
          document.getElementById(id).textContent = value;
        }
      }
    }
  };
  xhr.open("GET", "currentDate.cgx", true);
  xhr.send();
}

function periodicUpdateDate() 
{
  updateTimeField();
  updateDateField();
  setTimeout(periodicUpdateDate, 1000);
}

window.onload = function() 
{
  periodicUpdateDate();
  toggleIncrement(); // Ensure correct initial state
};

let matchTime = 15;
function changeMatchTime(delta) {
  matchTime = Math.max(5, Math.min(90, matchTime + delta));
  document.getElementById('matchTime').textContent = matchTime + ":00";
  document.getElementById('matchTimeInput').value = matchTime;
}

let incrementTime = 2;
function changeIncrement(delta) {
  if (!document.getElementById('incrementEnabled').checked) return;
  incrementTime = Math.max(2, Math.min(30, incrementTime + delta));
  document.getElementById('incrementTime').textContent = "+" + incrementTime;
  document.getElementById('incrementTimeInput').value = incrementTime;
}

function submitFormAjax() {
  // Get player name inputs
  var p1 = document.getElementById('player1Name');
  var p2 = document.getElementById('player2Name');
  var valid = true;

  // Reset previous highlights
  p1.style.backgroundColor = '';
  p2.style.backgroundColor = '';

  // Validate player names
  if (!p1.value.trim()) {
    p1.style.backgroundColor = '#ffcccc'; // light red
    valid = false;
  }
  if (!p2.value.trim()) {
    p2.style.backgroundColor = '#ffcccc';
    valid = false;
  }

  if (!valid) return; // Don't send if invalid

  // Set ayuda value explicitly
  var ayudaCheckbox = document.getElementById('ayuda');
  if (ayudaCheckbox) {
    ayudaCheckbox.value = ayudaCheckbox.checked ? "true" : "false";
    ayudaCheckbox.disabled = false; // ensure it's sent
  }

  // Serialize and send form data via AJAX
  var form = document.forms['cgi'];
  var data = [];
  for (var i = 0; i < form.elements.length; i++) {
    var el = form.elements[i];
    if (el.name && !el.disabled) {
      data.push(encodeURIComponent(el.name) + '=' + encodeURIComponent(el.value));
    }
  }
  var params = data.join('&');

  var xhr = new XMLHttpRequest();
  xhr.open("POST", "leaderboard.cgi", true);
  xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
  xhr.send(params);
}

function toggleIncrement() {
  var enabled = document.getElementById('incrementEnabled').checked;
  var span = document.getElementById('incrementTime');
  var incDown = document.getElementById('incDown');
  var incUp = document.getElementById('incUp');
  var hiddenInput = document.getElementById('incrementTimeInput');

  if (enabled) {
    span.style.filter = '';
    span.style.backgroundColor = '#f9f9f9'; // original color
    span.style.color = ''; // reset to default
    incDown.disabled = false;
    incUp.disabled = false;
    hiddenInput.disabled = false;
  } else {
    // Match disabled button color (adjust if your CSS is different)
    span.style.filter = '';
    span.style.backgroundColor = '#e9ecef'; // typical disabled button bg
    span.style.color = '#6c757d';           // typical disabled button text
    incDown.disabled = true;
    incUp.disabled = true;
    hiddenInput.disabled = true;
  }
}
