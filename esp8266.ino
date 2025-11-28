// ==================== ESP8266 ROBOT HÚT KHÍ – KHÔNG CÒN LỖI FUNCTION ====================

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

ThreeWire myWire(D6, D7, D8);
RtcDS1302<ThreeWire> Rtc(myWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);

ESP8266WebServer server(80);

const char* ssid = "RobotHutBui";
const char* password = "12345678";

// BIẾN
String nutDangGiu = "";
int tocDoHienTai = 180;
int tocDoQuatHienTai = 210;
unsigned long lastSend = 0;
const unsigned long interval = 25;
String cheDoHienThi = "TAT";
int phutChay = 0;
unsigned long thoiGianBatDau = 0;
bool dangDemNguoc = false;

void gui(const String& cmd) {
  Serial.println(cmd);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Robot Hút Khí Gas</title>
<link rel="icon" href="data:image/svg+xml,<svg xmlns=%22http://www.w3.org/2000/svg%22 viewBox=%220 0 100 100%22><text y=%22.9em%22 font-size=%2290%22>🤖</text></svg>">
<style>
  /* --- Variables & Theming (Optimized for dark mode contrast) --- */
  :root {
    --bg-start: #1e3c72; /* Deep Blue */
    --bg-end: #2a5298;   /* Medium Blue */
    --card-bg: rgba(255, 255, 255, 0.15); /* Slightly less transparent */
    --border-color: rgba(255, 255, 255, 0.3);
    --text-color: #ffffff;
    --primary-color: #00eaff; /* Brighter Cyan */
    --danger-color: #ff4d4d;
    --success-color: #69ff4d;
    --shadow-light: rgba(0, 0, 0, 0.2);
    --shadow-dark: rgba(0, 0, 0, 0.5);
    --glass-blur: 18px; /* Increased blur */
  }
  @media (prefers-color-scheme: light) {
    :root {
      --bg-start: #f5f7fa;
      --bg-end: #c3cfe2;
      --card-bg: rgba(255, 255, 255, 0.9);
      --border-color: rgba(0, 0, 0, 0.15);
      --text-color: #1a1a2e;
      --primary-color: #007bff;
      --shadow-light: rgba(0, 0, 0, 0.1);
      --shadow-dark: rgba(0, 0, 0, 0.3);
    }
  }

  /* --- Global Styles --- */
  * { margin:0; padding:0; box-sizing:border-box; }
  body {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    background: linear-gradient(135deg, var(--bg-start) 0%, var(--bg-end) 100%);
    color: var(--text-color);
    min-height: 100vh;
    padding: 25px 15px;
    background-attachment: fixed;
    transition: background 0.5s ease;
  }
  .container {
    max-width: 450px; 
    margin: 0 auto;
  }
  h1 {
    font-size: 34px;
    text-align: center;
    margin-bottom: 5px;
    text-shadow: 0 4px 12px var(--shadow-dark);
    letter-spacing: 1.5px;
    font-weight: 800;
  }
  .subtitle {
    text-align: center;
    opacity: 0.9;
    font-size: 15px;
    margin-bottom: 35px;
    font-style: italic;
  }

  /* --- Glass Card Style --- */
  .card {
    background: var(--card-bg);
    backdrop-filter: blur(var(--glass-blur));
    -webkit-backdrop-filter: blur(var(--glass-blur));
    border-radius: 25px; /* More rounded */
    padding: 25px;
    margin: 20px 0;
    border: 1px solid var(--border-color);
    box-shadow: 0 15px 40px var(--shadow-dark);
    transition: all 0.3s ease;
  }
  .card:hover {
    transform: translateY(-3px);
    box-shadow: 0 20px 50px var(--shadow-dark);
  }
  .label {
    font-size: 19px;
    font-weight: 700;
    margin-bottom: 18px;
    display: flex;
    justify-content: space-between;
    align-items: center;
    text-shadow: 0 1px 2px var(--shadow-dark);
  }
  .value {
    font-size: 20px;
    font-weight: 800;
    color: var(--primary-color);
    text-shadow: 0 0 8px var(--primary-color);
  }

  /* --- Sliders --- */
  input[type="range"] {
    -webkit-appearance: none;
    width: 100%;
    height: 12px;
    border-radius: 12px;
    background: var(--border-color);
    outline: none;
    margin: 10px 0;
  }
  input[type="range"]::-webkit-slider-thumb {
    -webkit-appearance: none;
    width: 30px;
    height: 30px;
    border-radius: 50%;
    background: var(--primary-color);
    cursor: grab;
    border: 4px solid var(--text-color);
    box-shadow: 0 0 15px var(--primary-color);
    transition: all 0.3s;
  }
  input[type="range"]::-webkit-slider-thumb:active {
    transform: scale(1.1);
    cursor: grabbing;
  }

  /* --- Big Buttons (Mode/Power) --- */
  .btn-group {
    display: flex;
    gap: 12px;
    margin: 20px 0;
  }
  .bigbtn {
    flex: 1;
    padding: 20px;
    font-size: 19px;
    font-weight: 700;
    border: none;
    border-radius: 20px;
    color: white;
    box-shadow: 0 10px 30px var(--shadow-dark);
    transition: transform 0.15s;
    text-shadow: 0 1px 3px var(--shadow-dark);
  }
  .auto { background: linear-gradient(45deg, #1d976c, #93f9b9); }
  .manual { background: linear-gradient(45deg, #f7941d, #f9a84a); }
  .start { background: linear-gradient(45deg, #4d7cff, #8a9eff); }
  .off { background: linear-gradient(45deg, #ff6b6b, #ff8c8c); color: #8d1c1c; }
  .bigbtn:active { transform: translateY(3px); box-shadow: 0 5px 15px var(--shadow-dark); }

  /* --- Directional Pad (Simplified) --- */
  .dir-pad {
    display: grid;
    grid-template-columns: repeat(2, 1fr);
    grid-template-rows: repeat(2, 1fr);
    gap: 15px;
    margin-bottom: 20px;
  }
  .dir-btn {
    padding: 30px 10px;
    font-size: 40px;
    font-weight: 900;
    border: 3px solid var(--border-color);
    border-radius: 20px;
    color: var(--text-color);
    background: var(--card-bg);
    backdrop-filter: blur(var(--glass-blur));
    box-shadow: 0 8px 25px var(--shadow-dark);
    transition: all 0.2s ease;
    user-select: none;
    cursor: pointer;
    text-align: center;
    line-height: 1;
  }
  .dir-btn:active {
    transform: scale(0.95);
    background: var(--primary-color);
    border-color: var(--primary-color);
    box-shadow: 0 0 30px var(--primary-color);
  }

  /* Specific Button Layout */
  .fwd { grid-column: 1 / 3; } /* Nút Tiến (F) chiếm toàn bộ chiều ngang */
  .bwd { grid-column: 1 / 3; } /* Nút Lùi (B) chiếm toàn bộ chiều ngang */
  .left { grid-row: 2; grid-column: 1; }
  .right { grid-row: 2; grid-column: 2; }
  
  /* --- Status Bar --- */
  .status {
    text-align: center;
    padding: 18px;
    font-size: 16px;
    opacity: 0.9;
    font-weight: 600;
    margin-top: 30px;
    background: var(--card-bg);
    backdrop-filter: blur(var(--glass-blur));
    border-radius: 20px;
    border: 1px solid var(--border-color);
    box-shadow: 0 5px 15px var(--shadow-dark);
  }
</style>
</head>
<body>

<div class="container">
  <h1>🤖 Robot Hút Bụi</h1>
  <div class="subtitle">Điều khiển thông minh • An toàn tuyệt đối</div>

  <div class="card">
    <div class="label">Điều khiển Hướng Di chuyển</div>
    <div class="dir-pad">
      <button class="dir-btn fwd" 
              onmousedown="keep('F')" ontouchstart="keep('F')" 
              onmouseup="stop()" ontouchend="stop()">
        <span style="font-size: 20px; display: block; margin-bottom: 5px;">TIẾN</span>▲
      </button>
      
      <button class="dir-btn left" 
              onmousedown="keep('L')" ontouchstart="keep('L')" 
              onmouseup="stop()" ontouchend="stop()">
        ◀<span style="font-size: 20px; display: block; margin-top: 5px;">TRÁI</span>
      </button>
      
      <button class="dir-btn right" 
              onmousedown="keep('R')" ontouchstart="keep('R')" 
              onmouseup="stop()" ontouchend="stop()">
        ▶<span style="font-size: 20px; display: block; margin-top: 5px;">PHẢI</span>
      </button>

      <button class="dir-btn bwd" 
              onmousedown="keep('B')" ontouchstart="keep('B')" 
              onmouseup="stop()" ontouchend="stop()">
        ▼<span style="font-size: 20px; display: block; margin-top: 5px;">LÙI</span>
      </button>
    </div>
  </div>

  <div class="btn-group">
    <button class="bigbtn auto" onclick="once('AUTO')">TỰ ĐỘNG</button>
    <button class="bigbtn manual" onclick="once('MANUAL')">THỦ CÔNG</button>
  </div>

  <div class="btn-group">
    <button class="bigbtn start" onclick="once('START')">BẬT ROBOT</button>
    <button class="bigbtn off" onclick="once('STOPBOT')">TẮT ROBOT</button>
  </div>

  <div class="card">
    <div class="label">Tốc độ quạt <span id="v" class="value">210</span></div>
    <input type="range" min="0" max="255" value="210" 
          oninput="once('FAN:'+this.value); document.getElementById('v').innerHTML=this.value">
  </div>

  <div class="card">
    <div class="label">Tốc độ di chuyển <span id="speedVal" class="value">180</span></div>
    <input type="range" min="0" max="255" value="180" oninput="setSpeed(this.value)">
  </div>

  <div class="card">
    <div class="label">Thời gian chạy <span id="timerVal" class="value">0</span> phút</div>
    <input type="range" min="0" max="60" step="1" value="0" oninput="setTimer(this.value)">
  </div>

  <div class="status">Kết nối ổn định • Sẵn sàng điều khiển</div>
</div>

<script>
  let currentSpeed = 180;

  const vibrate = () => {
    if ('vibrate' in navigator) {
      navigator.vibrate(50);
    }
  };

  const once = (cmd) => {
    vibrate();
    fetch('/cmd?c=' + cmd);
  };
  const stop = () => {
    fetch('/stop');
  };

  const setSpeed = (val) => {
    currentSpeed = val;
    document.getElementById('speedVal').innerHTML = val;
    fetch('/speed?s=' + val);
  };

  const keep = (cmd) => {
    vibrate(); // Rung khi bắt đầu nhấn giữ
    fetch('/keep?c=' + cmd + '&s=' + currentSpeed);
  };

  const setTimer = (val) => {
    document.getElementById('timerVal').innerHTML = val;
    fetch('/timer?t=' + val);
  };

  window.onload = () => {
    // Cập nhật giá trị ban đầu cho các thanh trượt
    document.getElementById('speedVal').innerHTML = document.querySelector('[oninput*="setSpeed"]').value;
    document.getElementById('v').innerHTML = document.querySelector('[oninput*="FAN"]').value;
    document.getElementById('timerVal').innerHTML = document.querySelector('[oninput*="setTimer"]').value;
  };
</script>

</body>
</html>
)rawliteral";
// ==================== XỬ LÝ WEB (GIỮ NGUYÊN) ====================
void handleRoot() { server.send_P(200, "text/html", index_html); }

void handleCmd() {
  if (server.hasArg("c")) {
    String c = server.arg("c");
    gui(c);
    if (c == "START") { cheDoHienThi = "TD"; dangDemNguoc = false; }
    else if (c == "AUTO")   cheDoHienThi = "TD";
    else if (c == "MANUAL") cheDoHienThi = "DK";
    else if (c == "STOPBOT") { cheDoHienThi = "TAT"; dangDemNguoc = false; }
    else if (c.startsWith("FAN:")) {
      tocDoQuatHienTai = c.substring(4).toInt();
      tocDoQuatHienTai = constrain(tocDoQuatHienTai, 0, 255);
    }
  }
  server.send(200);
}

void handleKeep() {
  if (server.hasArg("c")) {
    nutDangGiu = server.arg("c");
    if (server.hasArg("s")) tocDoHienTai = constrain(server.arg("s").toInt(), 0, 255);
  }
  server.send(200);
}

void handleStop() {
  nutDangGiu = "";
  gui("S");
  server.send(200);
}

void handleSpeed() {
  if (server.hasArg("s")) {
    int s = constrain(server.arg("s").toInt(), 0, 255);
    if (s != tocDoHienTai) {
      tocDoHienTai = s;
      gui("SPEED:" + String(s));
    }
  }
  server.send(200);
}

void handleTimer() {
  if (server.hasArg("t")) {
    phutChay = server.arg("t").toInt();
    phutChay = constrain(phutChay, 0, 30);
    if (phutChay > 0) {
      thoiGianBatDau = millis();
      dangDemNguoc = true;
    } else {
      dangDemNguoc = false;
    }
  }
  server.send(200);
}

// ==================== LCD ====================
unsigned long lastLCD = 0;
void capNhatLCD() {
  if (millis() - lastLCD < 500) return;
  lastLCD = millis();

  lcd.setCursor(0, 0);
  lcd.print("MODE:");
  lcd.print(cheDoHienThi);
  lcd.print("     ");

  lcd.setCursor(11, 0);
  if (dangDemNguoc && phutChay > 0) {
    unsigned long daChay = (millis() - thoiGianBatDau) / 1000;
    long conLai = (long)phutChay * 60 - daChay;
    if (conLai <= 0) {
      conLai = 0;
      dangDemNguoc = false;
      gui("STOPBOT");
      cheDoHienThi = "TAT";
    }
    int m = conLai / 60;
    int s = conLai % 60;
    if (m < 10) lcd.print("0");
    lcd.print(m);
    lcd.print(":");
    if (s < 10) lcd.print("0");
    lcd.print(s);
  } else {
    lcd.print("    ");
  }

  lcd.setCursor(0, 1);
  lcd.print("DC:");
  if (tocDoHienTai < 100) lcd.print(" ");
  if (tocDoHienTai < 10)  lcd.print(" ");
  lcd.print(tocDoHienTai);

  lcd.setCursor(8, 1);
  lcd.print("QUAT:");
  if (tocDoQuatHienTai < 100) lcd.print(" ");
  if (tocDoQuatHienTai < 10)  lcd.print(" ");
  lcd.print(tocDoQuatHienTai);
}

// ==================== SETUP & LOOP ====================
void setup() {
  Serial.begin(115200);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  Rtc.Begin();

  // === HIỂN THỊ TÊN WIFI & MẬT KHẨU KHI KHỞI ĐỘNG ===
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WF: ");
  lcd.print(ssid);

  lcd.setCursor(0, 1);
  lcd.print("Pass: ");
  lcd.print(password);
  delay(3000);
  lcd.clear();


  lcd.setCursor(0, 0);
  lcd.print("ROBOT KHOI DONG...");
  delay(1500);
  lcd.clear();

  WiFi.softAP(ssid, password);

  server.on("/",      handleRoot);
  server.on("/cmd",   handleCmd);
  server.on("/keep",  handleKeep);
  server.on("/stop",  handleStop);
  server.on("/speed", handleSpeed);
  server.on("/timer", handleTimer);
  server.begin();
}

void loop() {
  server.handleClient();
  capNhatLCD();

  if (nutDangGiu != "" && millis() - lastSend >= interval) {
    lastSend = millis();
    String cmd = nutDangGiu;
    if (nutDangGiu == "F" || nutDangGiu == "B" || nutDangGiu == "L" || nutDangGiu == "R")
      cmd += ":" + String(tocDoHienTai);
    gui(cmd);
  }
}