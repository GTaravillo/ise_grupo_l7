// === UI ELEMENT IDS AND CLASSES USED IN THIS FILE ===
const UI_IDS = {
  dateOut:       "dateOut",
  timeOut:       "timeOut",
  consumoActual: "consumoActual",
  player1Name:   "player1Name",
  player2Name:   "player2Name",
  tiempoBlancas: "tiempoBlancas",
  tiempoNegras:  "tiempoNegras",
  turno:         "turno",
  btnReanudar:   "btnReanudar"
};

const ENDPOINTS = {
  horaActual:    "currentTime.cgx",
  fechaActual:   "currentDate.cgx",
  consumoActual: "currentConsumo.cgx"
};

const WEBPAGE_NAME = "retomarPartida.cgi";
const FORM_NAME    = "retomarPartida";

function submitFormAjax() {
  // Only send the button state, since all fields are read-only
  var data = [];
  data.push("btnReanudar=1");
  var params = data.join('&');
  var xhr = new XMLHttpRequest();
  xhr.open("POST", WEBPAGE_NAME, true);
  xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
  xhr.send(params);
  return false; // Prevent default form submission
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

// Boton iniciar
function reanudar(event) {
  document.getElementById(UI_IDS.btnReanudar).disabled = true;

  buttonStates = {
    btnReanudar:   1,
  };

  return submitFormAjax();
}
