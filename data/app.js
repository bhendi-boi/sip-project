const gateway = `ws://${window.location.hostname}/ws`;
let websocket;
let isFanOn = true;
// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

window.addEventListener("load", onLoad);

function onLoad(event) {
  initWebSocket();
  updateStyles();
}

// ----------------------------------------------------------------------------
// WebSocket handling
// ----------------------------------------------------------------------------

function initWebSocket() {
  console.log("Trying to open a WebSocket connection...");
  websocket = new WebSocket(gateway);
  websocket.onopen = onOpen;
  websocket.onclose = onClose;
  websocket.onmessage = onMessage;
}

function onMessage(event) {
  console.log(`Received a notification from ${event.origin}`);
  console.log(event);
  isFanOn = event.data === "true" ? true : false;
  updateStyles();
}

function onOpen(event) {
  console.log("Connection opened");
}

function onClose(event) {
  console.log("Connection closed");
  setTimeout(initWebSocket, 2000);
}

function sendToggleInfoToESP32() {
  websocket.send("toggle");
}

// custom code

const input = document.getElementById("toggle");
const label = document.getElementById("toggle-label");
const slider = document.querySelector(".slider");
slider.addEventListener("click", handleChange);
input.addEventListener("change", handleChange);

function handleChange() {
  isFanOn = !isFanOn;
  console.log(isFanOn);
  sendToggleInfoToESP32();
  updateStyles();
}

function updateStyles() {
  input.setAttribute("data-checked", isFanOn);
  slider.setAttribute("data-checked", isFanOn);
  isFanOn ? (label.innerHTML = "Turn Off") : (label.innerHTML = "Turn On");
}
