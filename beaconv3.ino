#include <WiFi.h>
#include <WebServer.h>
#include "esp_wifi.h"
#include "FS.h"
#include "SPIFFS.h"

uint8_t packet[128] = {
  0x80, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
  0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x64, 0x00,
  0x31, 0x04,
  0x00
};

char ssids[10][32];
bool broadcasting = false;
WebServer server(80);

const char* ap_ssid = "Set_Your_Admin_SSID"; //set your own admin SSID
const char* ap_password = "Set_Your_Admin_Password"; //set your own admin passowrd

// ========== SPIFFS Functions ==========
void loadSSIDsFromSPIFFS() {
  if (!SPIFFS.begin(true)) return;
  File file = SPIFFS.open("/ssids.txt", "r");
  if (!file) return;
  for (int i = 0; i < 10 && file.available(); i++) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() > 0 && line.length() < 32) {
      line.toCharArray(ssids[i], 32);
    }
  }
  file.close();
}

void saveSSIDsToSPIFFS() {
  File file = SPIFFS.open("/ssids.txt", "w");
  if (!file) return;
  for (int i = 0; i < 10; i++) {
    file.println(ssids[i]);
  }
  file.close();
}

// ========== Default SSIDs ==========
void setDefaultSSIDs() {
  strcpy(ssids[0], "Starbucks_5G");
  strcpy(ssids[1], "TrojanHorse_AP");
  strcpy(ssids[2], "TP-Link_5G");
  strcpy(ssids[3], "Airport_5Ghz");
  strcpy(ssids[4], "NETGEAR_EXT");
  strcpy(ssids[5], "FBI_Surveillance_Van");
  strcpy(ssids[6], "DDOS_Lounge");
  strcpy(ssids[7], "Burger_King_4G");
  strcpy(ssids[8], "KFC Free Wifi");
  strcpy(ssids[9], "Your_Mic_Is_Live");
  saveSSIDsToSPIFFS();
}

// ========== HTML PAGE ==========
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>⚡ Beacon Control</title>
  <style>
    body {
      background-color: #0d0d0d;
      font-family: 'Courier New', monospace;
      color: #00ff00;
      text-align: center;
      padding: 20px;
    }
    h1 {
      font-size: 2.5em;
      text-shadow: 0 0 15px #00ff00;
    }
    h2, h3 {
      text-shadow: 0 0 5px #00ff00;
    }
    button, input {
      background-color: black;
      color: #00ff00;
      border: 1px solid #00ff00;
      padding: 10px;
      margin: 5px;
      font-family: monospace;
      width: 200px;
      transition: 0.3s;
    }
    button:hover, input:focus {
      background-color: #003300;
      outline: none;
    }
    #console {
      background: #111;
      color: #00ff00;
      padding: 10px;
      margin-top: 20px;
      height: 200px;
      overflow-y: scroll;
      border: 1px solid #00ff00;
      text-align: left;
      font-size: 0.9em;
    }
    ul {
      list-style: none;
      padding: 0;
    }
    li {
      margin: 3px 0;
    }

    footer {
      margin-top: 30px;
      border-top: 1px solid #00ff00;
      padding-top: 10px;
      color: #00ff00;
      font-size: 0.9em;
    }

    footer a {
      color: #00ff00;
      text-decoration: none;
      border-bottom: 1px dashed #00ff00;
    }

    .disclaimer {
      margin-top: 20px;
      padding: 10px;
      background-color: #330000;
      color: red;
      font-weight: bold;
      animation: flash 1s infinite alternate;
      border: 1px solid red;
    }

    @keyframes flash {
      0% { opacity: 1; }
      100% { opacity: 0.6; }
    }
  </style>
</head>
<body>
  <h1>⚡ ESP32 Beacon Dashboard ⚡</h1>
  <h2>Made by K Rajtilak</h2>

  <button onclick="sendCmd('start')">🚀 Start Broadcast</button>
  <button onclick="sendCmd('stop')">🛑 Stop Broadcast</button>

  <h3>Update SSID</h3>
  <input type="number" id="pos" min="0" max="9" placeholder="Position (0-9)">
  <input type="text" id="name" maxlength="31" placeholder="New SSID">
  <button onclick="setSSID()">✅ Set SSID</button>

  <h3>Current SSIDs</h3>
  <ul id="ssidList"></ul>

  <div id="console"><b>Console:</b><br></div>

  <footer>
    &copy; 2025 <a href="https://github.com/rajtilak-2020" target="_blank">K Rajtilak</a>. All rights reserved.
  </footer>

  <div class="disclaimer">
    ⚠️ Disclaimer: This tool is for **educational and testing purposes only**. Unauthorized use may violate local laws. Use responsibly!
  </div>

  <script>
    function sendCmd(cmd) {
      fetch('/cmd?do=' + cmd)
        .then(response => response.text())
        .then(data => logToConsole(data));
    }

    function setSSID() {
      const pos = document.getElementById('pos').value;
      const name = document.getElementById('name').value;
      fetch(`/set?pos=${pos}&name=${name}`)
        .then(res => res.text())
        .then(data => {
          logToConsole(data);
          loadSSIDs();
        });
    }

    function loadSSIDs() {
      fetch('/list')
        .then(res => res.json())
        .then(data => {
          let html = "";
          for (let i in data) html += `<li>${i}: ${data[i]}</li>`;
          document.getElementById('ssidList').innerHTML = html;
        });
    }

    function logToConsole(text) {
      const log = document.getElementById('console');
      log.innerHTML += text + "<br>";
      log.scrollTop = log.scrollHeight;
    }

    loadSSIDs();
  </script>
</body>
</html>
)rawliteral";

// ========== JSON ==========
String getSSIDJson() {
  String json = "{";
  for (int i = 0; i < 10; i++) {
    json += "\"" + String(i) + "\":\"" + String(ssids[i]) + "\"";
    if (i < 9) json += ",";
  }
  json += "}";
  return json;
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  WiFi.softAP(ap_ssid, ap_password);

  SPIFFS.begin(true);
  loadSSIDsFromSPIFFS();

  if (strlen(ssids[0]) == 0) {
    setDefaultSSIDs();
  }

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_AP);
  esp_wifi_start();

  server.on("/", []() {
    server.send_P(200, "text/html", MAIN_page);
  });

  server.on("/cmd", []() {
    String cmd = server.arg("do");
    if (cmd == "start") {
      broadcasting = true;
      server.send(200, "text/plain", "📡 Broadcasting started.");
    } else if (cmd == "stop") {
      broadcasting = false;
      server.send(200, "text/plain", "❌ Broadcasting stopped.");
    } else {
      server.send(400, "text/plain", "⚠️ Unknown command.");
    }
  });

  server.on("/list", []() {
    server.send(200, "application/json", getSSIDJson());
  });

  server.on("/set", []() {
    if (server.hasArg("pos") && server.hasArg("name")) {
      int pos = server.arg("pos").toInt();
      String name = server.arg("name");
      if (pos >= 0 && pos < 10 && name.length() > 0 && name.length() < 32) {
        name.toCharArray(ssids[pos], 32);
        saveSSIDsToSPIFFS();
        server.send(200, "text/plain", "✅ SSID updated.");
      } else {
        server.send(400, "text/plain", "❌ Invalid position or SSID length.");
      }
    } else {
      server.send(400, "text/plain", "⚠️ Missing parameters.");
    }
  });

  server.begin();
  Serial.println("AP Started. Connect to:");
  Serial.println("SSID: " + String(ap_ssid));
  Serial.println("Password: " + String(ap_password));
  Serial.println("Go to http://192.168.4.1");
}

// ========== LOOP ==========
void loop() {
  server.handleClient();
  if (broadcasting) {
    for (int i = 0; i < 10; i++) {
      for (int j = 10; j <= 15; j++) packet[j] = packet[j + 6] = random(256);
      int len = strlen(ssids[i]);
      packet[37] = len;
      for (int j = 0; j < len; j++) packet[38 + j] = ssids[i][j];
      esp_wifi_80211_tx(WIFI_IF_AP, packet, 38 + len, false);
      delay(1);
    }
  }
}
