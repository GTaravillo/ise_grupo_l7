// === UI ELEMENT IDS AND CLASSES USED IN THIS FILE ===
const UI_IDS = {
  dateOut:        "dateOut",
  timeOut:        "timeOut",
  consumoActual:  "consumoActual",
  player1Name:    "player1Name",
  player2Name:    "player2Name",
  matchTime:      "matchTime",
  matchTimeInput: "matchTimeInput",
  btnIniciar:     "btnIniciar",
  btnPausar:      "btnPausar",
  btnSuspender:   "btnSuspender",
  btnRendirse:    "btnRendirse"
};

const WEBPAGE_NAME = "nuevaPartida.cgi";
const FORM_NAME    = "nuevaPartida";

const ENDPOINTS = {
  horaActual:    "currentTime.cgx",
  fechaActual:   "currentDate.cgx",
  consumoActual: "currentConsumo.cgx"
};

let buttonStates = {
  btnIniciar:   0,
  btnPausar:    0,
  btnSuspender: 0,
  btnRendirse:  0
};

function submitFormAjax() {
  var p1 = document.getElementsByName(UI_IDS.player1Name)[0];
  var p2 = document.getElementsByName(UI_IDS.player2Name)[0];
  var valid = true;
  p1.style.backgroundColor = '';
  p2.style.backgroundColor = '';
  if (!p1.value.trim()) {
    p1.style.backgroundColor = '#ffcccc';
    valid = false;
  }
  if (!p2.value.trim()) {
    p2.style.backgroundColor = '#ffcccc';
    valid = false;
  }
  if (!valid) return false; // Prevent form submission

  var form = document.forms[FORM_NAME];
  var data = [];
  for (var i = 0; i < form.elements.length; i++) {
    var el = form.elements[i];
    if (el.name && !el.disabled) {
      data.push(encodeURIComponent(el.name) + '=' + encodeURIComponent(el.value));
    }
  }

  for (const key in buttonStates) {
    data.push(encodeURIComponent(key) + '=' + encodeURIComponent(buttonStates[key]));
  }

  var params = data.join('&');

  var xhr = new XMLHttpRequest();
  xhr.open("POST", WEBPAGE_NAME, true);
  xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
  xhr.send(params);

  return false;
}

// Actualización topbar al cargar página
window.onload = function() {
  periodicUpdateFields();
};

function periodicUpdateFields() {
  updateTimeField();
  updateDateField();
  updateConsumoField();
  setTimeout(periodicUpdateFields, 1000);
}

function updateDateField() {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (xhr.readyState == 4 && xhr.status == 200) {
      var xmlDoc = xhr.responseXML;
      if (xmlDoc) {
        var textNode = xmlDoc.getElementsByTagName("text")[0];
        if (textNode) {
          var id = textNode.getElementsByTagName("id")[0].textContent;
          var value = textNode.getElementsByTagName("value")[0].textContent;
          document.getElementById(id).textContent = value;
          document.getElementById(UI_IDS.dateOut).textContent = value;
        }
      }
    }
  };
  xhr.open("GET", ENDPOINTS.fechaActual, true);
  xhr.send();
}

function updateTimeField() {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (xhr.readyState == 4 && xhr.status == 200) {
      var xmlDoc = xhr.responseXML;
      if (xmlDoc) {
        var textNode = xmlDoc.getElementsByTagName("text")[0];
        if (textNode) {
          var id = textNode.getElementsByTagName("id")[0].textContent;
          var value = textNode.getElementsByTagName("value")[0].textContent;
          document.getElementById(id).textContent = value;
        }
      }
    }
  };
  xhr.open("GET", ENDPOINTS.horaActual, true);
  xhr.send();
}

function updateConsumoField() {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (xhr.readyState == 4 && xhr.status == 200) {
      var xmlDoc = xhr.responseXML;
      if (xmlDoc) {
        var textNode = xmlDoc.getElementsByTagName("text")[0];
        if (textNode) {
          var id = textNode.getElementsByTagName("id")[0].textContent;
          var value = textNode.getElementsByTagName("value")[0].textContent;
          document.getElementById(id).textContent = value;
        }
      }
    }
  };
  xhr.open("GET", ENDPOINTS.consumoActual, true);
  xhr.send();
}

function validatePlayers() {
  var p1 = document.getElementsByName(UI_IDS.player1Name)[0];
  var p2 = document.getElementsByName(UI_IDS.player2Name)[0];
  var valid = true;
  p1.style.backgroundColor = '';
  p2.style.backgroundColor = '';
  if (!p1.value.trim()) {
    p1.style.backgroundColor = '#ffcccc';
    valid = false;
  }
  if (!p2.value.trim()) {
    p2.style.backgroundColor = '#ffcccc';
    valid = false;
  }
  return valid;
}

// Incrementos/decrementos tiempo partida
let matchTime = 15;
function changeMatchTime(delta) {
  matchTime = Math.max(5, Math.min(90, matchTime + delta));
  document.getElementById(UI_IDS.matchTime).textContent = matchTime + ":00";
  document.getElementById(UI_IDS.matchTimeInput).value = matchTime;
}

// Boton iniciar
function iniciar(event) {
  // event.preventDefault();
  if (!validatePlayers()) {
    return false;
  }
  document.getElementById(UI_IDS.btnIniciar).disabled   = true;
  document.getElementById(UI_IDS.btnPausar).disabled    = false;
  document.getElementById(UI_IDS.btnSuspender).disabled = false;
  document.getElementById(UI_IDS.btnRendirse).disabled  = false;

  buttonStates = {
    btnIniciar:   1,
    btnPausar:    0,
    btnSuspender: 0,
    btnRendirse:  0
  };

  return submitFormAjax();
}

// Boton pausar
function pausar() {
  // Add your pause logic here
}

// Boton rendirse
function rendirse() {
  // Add your stop logic here
}

// Boton suspender
function suspender() {
  // Your suspend logic here
  alert("Partida suspendida");
}
