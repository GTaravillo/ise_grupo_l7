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

const STATE_KEY = 'gameButtonState';
const States = {
  INIT:      'init',
  STARTED:   'started',
  SUSPENDED: 'suspended',
  RESIGNED:  'resigned'
};
const btn = {
  iniciar:   'btnIniciar',
  pausar:    'btnPausar',
  suspender: 'btnSuspender',
  rendirse:  'btnRendirse'
};
// map each state to disabled=true/false for each button
const stateConfig = {
  [States.INIT]:      { iniciar:false, pausar:true,  suspender:true,  rendirse:true  },
  [States.STARTED]:   { iniciar:true,  pausar:false, suspender:false, rendirse:false },
  [States.SUSPENDED]: { iniciar:false, pausar:false, suspender:true,  rendirse:false },
  [States.RESIGNED]:  { iniciar:false, pausar:true,  suspender:true,  rendirse:true  },
};

function applyState(s) {
  const cfg = stateConfig[s] || stateConfig[States.INIT];
  Object.entries(cfg).forEach(([key, isDisabled]) => {
    const b = document.getElementById(btn[key]);
    if (b) b.disabled = isDisabled;
  });
}
function saveState(s)   { localStorage.setItem(STATE_KEY, s); }
function loadState()    { return localStorage.getItem(STATE_KEY) || States.INIT; }

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

  applyState(States.STARTED);
  saveState(States.STARTED);

  return submitFormAjax();
}

// Boton pausar
function pausar() {
  buttonStates = {
    btnIniciar:   0,
    btnPausar:    1,
    btnSuspender: 0,
    btnRendirse:  0
  };
}

// Boton suspender
function suspender() {
  buttonStates = {
    btnIniciar:   0,
    btnPausar:    0,
    btnSuspender: 1,
    btnRendirse:  0
  };

  applyState(States.SUSPENDED);
  saveState(States.SUSPENDED);
}


// Boton rendirse
function rendirse() {
  buttonStates = {
    btnIniciar:   0,
    btnPausar:    0,
    btnSuspender: 0,
    btnRendirse:  1
  };

  applyState(States.RESIGNED);
  saveState(States.RESIGNED);
}

window.addEventListener('DOMContentLoaded', () => {
  applyState(loadState());
});