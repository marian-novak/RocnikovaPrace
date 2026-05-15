// Knihovny, které používám nejsou mé, ale ze správce knihoven v Arduine IDE.
// CSS, HTML a JavaScript je poupraven z těchto stránek: https://randomnerdtutorials.com/esp32-esp8266-web-server-outputs-momentary-switch/ a https://stackoverflow.com/questions/42061680/how-to-detect-html-switch-state-with-javascript
// Kód pro používání světelného senzoru je z https://github.com/adafruit/Adafruit_TSL2591_Library/blob/master/examples/tsl2591/tsl2591.ino
// Maunál pro používání knihovny AccelStepper: https://hackaday.io/project/183279-accelstepper-the-missing-manual/details
// Také kód pro zapnutí HTTP serveru a GET requesty je z této stránky: https://hackaday.io/project/183279-accelstepper-the-missing-manual/details

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AccelStepper.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_TSL2591.h>

const char* ssid = "TP-Link_EC30";
const char* password = "52210389";

#define pinStep 16
#define pinDir 17
#define pinReset 2
#define pinSleep 19
#define dolniSenzor 4
#define horniSenzor 15
AccelStepper stepper(1, pinStep, pinDir);
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
bool nahoru = false;
bool dolu = false;
bool uplneDolu = false;
bool uplneNahoru = false;
bool svetSenzor = true;
bool svetSenzorOtevrit = false;


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
  <head>
    <title>Ovl&#225d&#225n&#237 Rolet</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Outfit:wght@100..900&display=swap');
    :root {
      color-scheme: light dark;
    }

    body {
      font-family: "Outfit", sans-serif;
      text-align: center;
      margin: 0px auto;
      padding-top: 30px;
 
    }

    .button {
      padding: 10px 20px;
      font-size: 24px;
      text-align: center;
      outline: none;
      font-weight: bold;
      color: #000000;
      background-color: #D5B60A;
      border-radius: 5px;
      border: 3px solid #5f5f5f;
      box-shadow: 0 6px #5f5f5f;
      cursor: pointer;
      -webkit-touch-callout: none;
      -webkit-user-select: none;
      -khtml-user-select: none;
      -moz-user-select: none;
      -ms-user-select: none;
      user-select: none;
      margin: 0px 0px 10px 0px;
      -webkit-tap-highlight-color: rgba(0, 0, 0, 0);
    }

    .text-switch {
      font-size: 18px;
      margin: 10px;
      font-weight: bold;

    }

    .button:hover {
      background-color: light-dark(#ccc, #9f8808);
    }

    .button:active {
      background-color: light-dark(#ccc, #9f8808);
      box-shadow: 0 4px #666;
      transform: translateY(2px);
    }

    .cmn-toggle {
      position: absolute;
      visibility: hidden;
    }

    .cmn-toggle+label {
      display: block;
      position: relative;
      cursor: pointer;
      outline: none;
      user-select: none;
    }

    .switch {
      margin-top: 20px;
      display: grid;
      grid-template-columns: auto auto auto;
      align-items: center;
      justify-content: center;
    }

    input.cmn-toggle-round+label {
      padding: 2px;
      width: 110px;
      height: 60px;
      background-color: #666;
      border-radius: 60px;
    }

    input.cmn-toggle-round+label:before,
    input.cmn-toggle-round+label:after {
      display: block;
      position: absolute;
      top: 1px;
      left: 1px;
      bottom: 1px;
      content: "";
    }

    input.cmn-toggle-round+label:before {
      right: 1px;
      background-color: #D5B60A;
      border-radius: 60px;
      transition: background 0.4s;
    }

    input.cmn-toggle-round+label:after {
      width: 62px;
      background-color: #5f5f5f;
      border-radius: 100%;
      box-shadow: 0 2px 5px rgba(0, 0, 0, 0.3);
      transition: margin 0.4s;
    }

    input.cmn-toggle-round:checked+label:before {
      background-color: light-dark(#ccc, #1f2e45);
    }

    input.cmn-toggle-round:checked+label:after {
      margin-left: calc(100% - 62px);
    }
  </style>
  </head>
  <body>
    <h1>Ovl&#225d&#225n&#237 Rolet</h1>
    <button class="button" onmousedown="toggleCheckbox('nahoru');" ontouchstart="toggleCheckbox('nahoru');" onmouseup="toggleCheckbox('off');" ontouchend="toggleCheckbox('off');">Nahoru</button>
    <button class="button" onmousedown="toggleCheckbox('dolu');" ontouchstart="toggleCheckbox('dolu');" onmouseup="toggleCheckbox('off');" ontouchend="toggleCheckbox('off');">Dolu</button>
    <button class="button" onmousedown="toggleCheckbox('uplnedolu');" ontouchstart="toggleCheckbox('uplnedolu');" onmouseup="toggleCheckbox('off');" ontouchend="toggleCheckbox('off');">&#218pln&#283 Dolu</button>
    <button class="button" onmousedown="toggleCheckbox('uplnenahoru');" ontouchstart="toggleCheckbox('uplnenahoru');" onmouseup="toggleCheckbox('off');" ontouchend="toggleCheckbox('off');">&#218pln&#283 Nahoru</button>
    <div>
        <div class="switch">
            <p class="text-switch">Zapnout sv&#283teln&#253 senzor</p>
            <input id="cmn-toggle-2" class="cmn-toggle cmn-toggle-round" type="checkbox" onchange="toggleCheckbox(this.checked ? 'svetelnySenzorOff' : 'svetelnySenzorOn');">
            <label for="cmn-toggle-2"></label>
            <p class="text-switch">Vypnout sv&#283teln&#253 senzor</p>
        </div>
    </div>
    <div>
        <div class="switch">
            <p class="text-switch">Zav&#345&#237t p&#345i sv&#283tle</p>
            <input id="cmn-toggle-3" class="cmn-toggle cmn-toggle-round" type="checkbox" onchange="toggleCheckbox(this.checked ? 'svetelnySenzorOtevrit' : 'svetelnySenzorZavrit');">
            <label for="cmn-toggle-3"></label>
            <p class="text-switch">Otev&#345&#237t p&#345i sv&#283tle</p>
        </div>
    </div>
   <script>
   function toggleCheckbox(x) {
     var xhr = new XMLHttpRequest();
     xhr.open("GET", "/" + x, true);
     xhr.send();
   }
  </script>
  </body>
</html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  tsl.setGain(TSL2591_GAIN_MED);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  pinMode(dolniSenzor, INPUT);
  pinMode(horniSenzor, INPUT);
  pinMode(pinSleep, OUTPUT);
  pinMode(pinReset, OUTPUT);
  stepper.setMaxSpeed(1000.0);
  stepper.setAcceleration(500.0);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("ESP IP Address: http://");
  Serial.println(WiFi.localIP());
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });

  server.on("/nahoru", HTTP_GET, [](AsyncWebServerRequest *request) {
    nahoru = true;
    request->send(200, "text/plain", "ok");
  });

  server.on("/dolu", HTTP_GET, [](AsyncWebServerRequest *request) {
    dolu = true;
    request->send(200, "text/plain", "ok");
  });

    server.on("/uplnedolu", HTTP_GET, [](AsyncWebServerRequest *request) {
    uplneDolu = true;
    request->send(200, "text/plain", "ok");
  });

    server.on("/uplnenahoru", HTTP_GET, [](AsyncWebServerRequest *request) {
    uplneNahoru = true;
    request->send(200, "text/plain", "ok");
  });

    server.on("/svetelnySenzorOn", HTTP_GET, [](AsyncWebServerRequest *request) {
    svetSenzor = true;
    request->send(200, "text/plain", "ok");
  });

    server.on("/svetelnySenzorOff", HTTP_GET, [](AsyncWebServerRequest *request) {
    svetSenzor = false;
    request->send(200, "text/plain", "ok");
  });

    server.on("/svetelnySenzorZavrit", HTTP_GET, [](AsyncWebServerRequest *request) {
    svetSenzorOtevrit = false;
    request->send(200, "text/plain", "ok");
  });

    server.on("/svetelnySenzorOtevrit", HTTP_GET, [](AsyncWebServerRequest *request) {
    svetSenzorOtevrit = true;
    request->send(200, "text/plain", "ok");
  });
    server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "ok");
    dolu = false;
    nahoru = false;
  });
  
  server.onNotFound(notFound);
  server.begin();

}

void loop() {
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  uint32_t svetlo = tsl.calculateLux(full, ir);
  digitalWrite(pinReset, HIGH);
  digitalWrite(pinSleep, LOW);
  delay(1);
  while ((uplneNahoru == true) && (digitalRead(horniSenzor) == 1))
  {
    digitalWrite(pinSleep, HIGH);
    delay(1);
    stepper.move(20);
    stepper.runToPosition();
  }
  uplneNahoru = false;
  while ((uplneDolu == true) && (digitalRead(dolniSenzor) == 1))
  {
    digitalWrite(pinSleep, HIGH);
    delay(1);
    stepper.move(-20);
    stepper.runToPosition();
  }
  uplneDolu = false;
  if ((svetSenzor == true) && (svetSenzorOtevrit == false) && (svetlo > 500))
  {
    digitalWrite(pinSleep, HIGH);
    delay(1);
    while (digitalRead(dolniSenzor) == 1)
    {
      stepper.move(-20);
      stepper.runToPosition();
    }
  }
  if ((svetSenzor == true) && (svetSenzorOtevrit == true) && (svetlo > 500))
  {
    digitalWrite(pinSleep, HIGH);
    delay(1);
    while (digitalRead(dolniSenzor) == 1)
    {
      stepper.move(-20);
      stepper.runToPosition();
    }
    stepper.move(200);
    stepper.runToPosition();
  }
  if (nahoru == true && (digitalRead(horniSenzor) == 1)) {
    digitalWrite(pinSleep, HIGH);
    delay(1);
    dolu = false;
    stepper.move(20);
    stepper.runToPosition();
  }
  if (dolu == true && (digitalRead(dolniSenzor) == 1)) {
    digitalWrite(pinSleep, HIGH);
    delay(1);
    nahoru = false;
    stepper.move(-20);
    stepper.runToPosition();
  }

}
}
