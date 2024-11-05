#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head><title>ESP8266 Tracker Control</title></head><body><table><tr><th><button id=pause_btn>&#128308;</button></th><th>Pause</th><th></th></tr><tr><th><button id=hold_btn>&#128308;</button></th><th>Hold</th><th></th></tr><tr><th><button id=cm_btn>&#128308;</button></th><th>Continuous mode</th><th><div>Speed<input type=text id=cm_speed_input><input id=cm_direction type=checkbox></div></th></tr><tr><th><button id=step_btn>&#128308;</button></th><th>Steps mode</th><th><div>Steps<input type=text id=step_input></div><div>Speed<input type=text id=step_speed_input></div></th></tr><tr><th></th><th>Total Position</th><th><span id=position>?</span></th></tr></table></body><script>var gateway=`ws:var ws;function initWebSocket(){ws=new WebSocket(gateway);ws.onopen=onOpen;ws.onclose=onClose;}
function onOpen(event){}
function onClose(event){setTimeout(initWebSocket,2000);}
window.addEventListener('load',onLoad);function onLoad(event){initWebSocket();document.getElementById('cm_btn').addEventListener('click',setModeContinuous);}
function setModeContinuous(){const buffer=new ArrayBuffer(32);const view=new DataView(buffer);view.setInt8(0,"C".charCodeAt(0));view.setInt8(4,document.querySelector('.cm_direction:checked').value?1:0);view.setBigUint64(8,document.getElementById("cm_speed_input").value);ws.send(buffer);}</script>
)rawliteral";
