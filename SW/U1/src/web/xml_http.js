// === UI ELEMENT IDS AND CLASSES USED IN THIS FILE ===
const UI_IDS = {
  dateOut:                "dateOut",
  timeOut:                "timeOut",
  consumoActual:          "consumoActual",
  player1Name:            "player1Name",
  player2Name:            "player2Name",
  matchTime:              "matchTime",
  matchTimeInput:         "matchTimeInput",
  incrementTime:          "incrementTime",
  incrementTimeInput:     "incrementTimeInput",
  incrementEnabled:       "incrementEnabled",
  incrementEnabledHidden: "incrementEnabledHidden",
  incDown:                "incDown",
  incUp:                  "incUp",
  ayuda:                  "ayuda",
  ayudaHidden:            "ayudaHidden",
  btnIniciar:             "btnIniciar",
  btnPausar:              "btnPausar",
  btnSuspender:           "btnSuspender",
  btnRendirse:            "btnRendirse"
};

const WEBPAGE_NAME = {
  nuevaPartida: "nuevaPartida"
};

const ENDPOINTS = {
  horaActual:    "currentTime.cgx",
  fechaActual:   "currentDate.cgx",
  consumoActual: "currentConsumo.cgx"
};

// Move elements into their table cells
function moveToCell(cellId, elementId) {
  var cell = document.getElementById(cellId);
  var el = document.getElementById(elementId);
  if (cell && el) cell.appendChild(el);
}

// Form validation and AJAX submission
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

  var ayudaCheckbox = document.getElementById(UI_IDS.ayuda);
  if (ayudaCheckbox) {
    ayudaCheckbox.value = ayudaCheckbox.checked ? "true" : "false";
    ayudaCheckbox.disabled = false;
  }

  var form = document.forms['nuevaPartida'];
  var data = [];
  for (var i = 0; i < form.elements.length; i++) {
    var el = form.elements[i];
    if (el.name && !el.disabled) {
      data.push(encodeURIComponent(el.name) + '=' + encodeURIComponent(el.value));
    }
  }
  var params = data.join('&');

  var xhr = new XMLHttpRequest();
  xhr.open("POST", "nuevaPartida.cgi", true);
  xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
  xhr.send(params);

  showGameButtons();
  return false; // Always prevent default form submission
}

// Increment logic
let matchTime = 15;
function changeMatchTime(delta) {
  matchTime = Math.max(5, Math.min(90, matchTime + delta));
  document.getElementById(UI_IDS.matchTime).textContent = matchTime + ":00";
  document.getElementById(UI_IDS.matchTimeInput).value = matchTime;
}

let incrementTime = 2;
function changeIncrement(delta) {
  if (!document.getElementById(UI_IDS.incrementEnabled).checked) return;
  incrementTime = Math.max(2, Math.min(30, incrementTime + delta));
  document.getElementById(UI_IDS.incrementTime).textContent = "+" + incrementTime;
  document.getElementById(UI_IDS.incrementTimeInput).value = incrementTime;
}

function toggleIncrement() {
  var enabled = document.getElementById(UI_IDS.incrementEnabled).checked;
  var span = document.getElementById(UI_IDS.incrementTime);
  var incDown = document.getElementById(UI_IDS.incDown);
  var incUp = document.getElementById(UI_IDS.incUp);
  var hiddenInput = document.getElementById(UI_IDS.incrementTimeInput);
  if (enabled) {
    span.style.filter = '';
    span.style.backgroundColor = '#f9f9f9';
    span.style.color = '';
    incDown.disabled = false;
    incUp.disabled = false;
    hiddenInput.disabled = false;
  } else {
    span.style.filter = '';
    span.style.backgroundColor = '#e9ecef';
    span.style.color = '#6c757d';
    incDown.disabled = true;
    incUp.disabled = true;
    hiddenInput.disabled = true;
  }
}

// Periodic update of time, date, and consumoActual
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

function periodicUpdateFields() {
  updateTimeField();
  updateDateField();
  updateConsumoField();
  setTimeout(periodicUpdateFields, 1000);
}

// On page load, assemble modular layout and start periodic updates
window.onload = function() {
  // moveToCell(UI_IDS.cellPlayer1, UI_IDS.player1Name);
  // moveToCell(UI_IDS.cellPlayer2, UI_IDS.player2Name);
  // moveToCell(UI_IDS.cellAyuda, UI_IDS.ayuda);
  // moveToCell(UI_IDS.cellStartBtn, UI_IDS.startBtn);
  // moveToCell(UI_IDS.cellStartBtn, UI_IDS.pauseBtn);
  // moveToCell(UI_IDS.cellStartBtn, UI_IDS.stopBtn);
  // Only call showGameButtons() after starting a game

  updateIncrementHidden();
  updateAyudaHidden();
  periodicUpdateFields();
};

function pauseGame() {
  // Add your pause logic here
}

function stopGame() {
  // Add your stop logic here
}

function suspendGame() {
  // Your suspend logic here
  alert("Partida suspendida");
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
  return valid; // If false, form will not submit
}

function updateIncrementHidden() {
  document.getElementById(UI_IDS.incrementEnabledHidden).value =
    document.getElementById(UI_IDS.incrementEnabled).checked ? 'true' : 'false';
}
function updateAyudaHidden() {
  document.getElementById(UI_IDS.ayudaHidden).value =
    document.getElementById(UI_IDS.ayuda).checked ? 'true' : 'false';
}