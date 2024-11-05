#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266mDNS.h>

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "html.cpp"

#define LED_PIN  2

#define EN_PIN 12
#define DIR_PIN 13
#define STEP_PIN 14

ESP8266WiFiMulti wifiMulti;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

enum Mode {
  Pause,
  Hold,
  Continuous,
  Steps,
};

Mode mode = Mode::Pause;

long total_position = 0;

// Continuous


// Steps
unsigned long steps_remaining = 0;

//

unsigned long speed_hT = 1000;
unsigned long step_micros;

uint8_t dir_state;
uint8_t step_state;

uint8_t half_step() {
  digitalWrite(DIR_PIN, dir_state);
  step_state = step_state ? LOW : HIGH;
  digitalWrite(STEP_PIN, step_state);
  total_position += dir_state ? -1 : 1;
  return step_state;
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_BINARY) {
    switch (data[0])
    {
    case 'C':
      uint8_t dir = data[4];
      unsigned long speed = *((unsigned long*)data[8]);

      dir_state = dir;
      speed_hT = speed / 2;

      mode = Mode::Continuous;
      break;
    case 'S':
      uint8_t dir = data[4];
      unsigned long speed = *((unsigned long*)data[8]);
      unsigned long steps = *((unsigned long*)data[16]);

      dir_state = dir;
      steps_remaining = steps;
      speed_hT = speed / 2;

      mode = Mode::Steps;
      break;
    case 'P':
      mode = Mode::Pause;
      break;
    case 'H':
      mode = Mode::Hold;
      break;
    
    default:
      break;
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void setup() {
  pinMode(LED_PIN,OUTPUT);

  pinMode(EN_PIN,OUTPUT);
  pinMode(DIR_PIN,OUTPUT);
  pinMode(STEP_PIN,OUTPUT);

  step_micros = micros();
  dir_state = HIGH;
  
  digitalWrite(EN_PIN, LOW);

  Serial.begin(9600);
  delay(10);

  Serial.println("Serial set up.");

  wifiMulti.addAP("UwU", "05803494937439686062");

  Serial.println("Connecting ");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(1000);
    Serial.print('.');
  }
  Serial.print("\nConnected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());   
  
  MDNS.begin("esptracker");

  ws.onEvent(onEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", index_html);
    });
  server.onNotFound([](AsyncWebServerRequest *request){
        request->send(404, "text/plain", "Not found");
    });
  server.begin();
  Serial.print("WebServer running!");
}

void loop() {
  unsigned long curr_time = micros();

  switch (mode)
  {
  case Mode::Pause:
    digitalWrite(EN_PIN, HIGH);
    break;
  case Mode::Hold:
    digitalWrite(EN_PIN, LOW);
    break;
  case Mode::Continuous:
    if (curr_time >= step_micros + speed_hT) {
      if (curr_time >= step_micros + 2 * speed_hT) {
        Serial.println("Too Fast!");
        step_micros = curr_time;
      } else {
        step_micros += speed_hT;
      }
      half_step();
    }
    break;
  case Mode::Steps:
    if (curr_time >= step_micros + speed_hT) {
      step_micros = curr_time + speed_hT;
      steps_remaining -= half_step();
      if (steps_remaining == 0) {
        mode = Mode::Hold;
      }
    }
    break;
  
  default:
    break;
  }

  digitalWrite(LED_PIN, dir_state);
}