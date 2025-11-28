#include <Arduino.h>

// ==================== 1. KHAI BÁO CHÂN ====================
const int CHAN_ENA = 5, CHAN_IN1 = 8, CHAN_IN2 = 7;
const int CHAN_ENB = 6, CHAN_IN3 = 9, CHAN_IN4 = 10;
const int CHAN_ENC = 11, CHAN_INC1 = 12, CHAN_INC2 = 13;

const int CHAN_TRIG = A0, CHAN_ECHO = A1;
const int IR_DUOI = A2, IR_TREN = A3, IR_TRAI = A4, IR_PHAI = A5;
const int NUT_BAT_TAT = 2;

// ==================== 2. HẰNG SỐ ====================
#define TOC_DO_QUAT 200
#define NGUONG_VAT_CAN 10
#define THOI_GIAN_LUI 800
#define THOI_GIAN_RE 1700
#define THOI_GIAN_DOC_CAM_BIEN 100
#define THOI_GIAN_CHONG_DOI_NUT 50

// ==================== 3. BIẾN TRẠNG THÁI ====================
bool robotDangChay = false, trangThaiNutCu = HIGH;
unsigned long thoiGianNhanNutCu = 0;

unsigned long thoiGianDoiChuyenAuto = 0;
bool dangChoChuyenAuto = false;

enum TrangThaiTranh { TIEN, LUI, RE_TRAI, RE_PHAI };
TrangThaiTranh trangThaiTranh = TIEN;

bool daReTrai = false, daRePhai = false;
unsigned long tgBatDauHanhDong = 0, tgDocCamBienTruoc = 0, tgChongRoiTruoc = 0;

bool cheDoManual = false;

// TỐC ĐỘ BÁNH XE ĐƯỢC ĐIỀU CHỈNH TỪ WEB (mặc định 180)
int tocDoDongCo = 150;

String uartBuffer = "";

// ==================== 4. ĐIỀU KHIỂN ĐỘNG CƠ ====================
void dieuKhienMotBanhXe(int huong, int tocDo, int d1, int d2, int pwm) {
  tocDo = constrain(tocDo, 0, 255);
  analogWrite(pwm, (huong != 0) ? tocDo : 0);

  if (huong > 0)      { digitalWrite(d1, HIGH); digitalWrite(d2, LOW);  }
  else if (huong < 0) { digitalWrite(d1, LOW);  digitalWrite(d2, HIGH); }
  else                { digitalWrite(d1, LOW);  digitalWrite(d2, LOW);  }
}

void datTocDoCuaCacBanhXe(int hTrai, int hPhai, int speed) {
  dieuKhienMotBanhXe(hTrai, speed, CHAN_IN1, CHAN_IN2, CHAN_ENA);
  dieuKhienMotBanhXe(hPhai, speed, CHAN_IN3, CHAN_IN4, CHAN_ENB);
}

void dungLai()  { datTocDoCuaCacBanhXe(0, 0, 0); }
void diThang()  { datTocDoCuaCacBanhXe(1, 1, tocDoDongCo); }
void diLui()    { datTocDoCuaCacBanhXe(-1, -1, tocDoDongCo); }
void reTrai()   { datTocDoCuaCacBanhXe(-1, 1, tocDoDongCo); }
void rePhai()   { datTocDoCuaCacBanhXe(1, -1, tocDoDongCo); }

void datTocDoQuat(int v) {
  digitalWrite(CHAN_INC1, HIGH);
  digitalWrite(CHAN_INC2, LOW);
  analogWrite(CHAN_ENC, constrain(v, 0, 255));
}

// ==================== 5. SIÊU ÂM ====================
long docKhoangCachCm() {
  digitalWrite(CHAN_TRIG, LOW); delayMicroseconds(2);
  digitalWrite(CHAN_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(CHAN_TRIG, LOW);
  long t = pulseIn(CHAN_ECHO, HIGH, 30000);
  if (!t) return 999;
  return t * 0.034 / 2;
}

// ==================== 6. CHỐNG RƠI ====================
bool kiemTraChongRoi() {
  if (millis() - tgChongRoiTruoc < 100) return false;
  tgChongRoiTruoc = millis();

  bool rT = digitalRead(IR_TREN);   // Trước
  bool rD = digitalRead(IR_DUOI);   // Sau
  bool rL = digitalRead(IR_TRAI);   // Trái
  bool rR = digitalRead(IR_PHAI);   // Phải

  // Không có cảm biến nào kích hoạt
  if (!rT && !rD && !rL && !rR) return false;

  dungLai(); 
  delay(50);

  // ƯU TIÊN: IR TRƯỚC (nguy hiểm vực phía trước)
  if (rT) {
    diLui(); delay(400);  // Lùi lại để tránh vực

    // Luân phiên rẽ trái/phải như tránh vật cản
    if (!daReTrai && !daRePhai) { 
      reTrai(); delay(1800); 
      daReTrai = true; 
    }
    else if (daReTrai && !daRePhai) { 
      rePhai(); delay(1800); 
      daRePhai = true; 
    }
    else { 
      daReTrai = daRePhai = false; 
    }
  }
  // Các trường hợp khác: giữ nguyên
  else if (rD) { 
    diThang(); delay(200); 
  }
  else if (rL) { 
    rePhai(); delay(1000); 
  }
  else if (rR) { 
    reTrai(); delay(1000); 
  }

  dungLai();
  trangThaiTranh = TIEN;
  daReTrai = daRePhai = false;  // Reset sau khi xử lý xong
  return true;
}

// ==================== 7. NÚT BẬT/TẮT ====================
void xuLyNutBatTat() {
  bool h = digitalRead(NUT_BAT_TAT);
  if (trangThaiNutCu == HIGH && h == LOW &&
      millis() - thoiGianNhanNutCu > THOI_GIAN_CHONG_DOI_NUT) {

    robotDangChay = !robotDangChay;

    if (robotDangChay) {
      datTocDoQuat(TOC_DO_QUAT);
      trangThaiTranh = TIEN;
      daReTrai = daRePhai = false;
      cheDoManual = false;
      dangChoChuyenAuto = false;

      Serial.println("START");
      Serial.println("AUTO");
    } else {
      dungLai();
      datTocDoQuat(0);
      Serial.println("STOPBOT");
    }
    thoiGianNhanNutCu = millis();
  }
  trangThaiNutCu = h;
}

// ==================== 8. XỬ LÝ LỆNH UART ====================
void xuLyLenhUART() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      uartBuffer.trim();
      if (uartBuffer.length() == 0) { uartBuffer = ""; return; }

      // Lệnh bật/tắt
      if (uartBuffer == "START") {
        robotDangChay = true;
        datTocDoQuat(TOC_DO_QUAT);
        trangThaiTranh = TIEN;
        daReTrai = daRePhai = false;
        cheDoManual = false;
        dangChoChuyenAuto = false;
        Serial.println("START");
        Serial.println("AUTO");
      } else if (uartBuffer == "STOPBOT") {
        robotDangChay = false;
        cheDoManual = false;
        dangChoChuyenAuto = false;
        dungLai();
        datTocDoQuat(0);
        Serial.println("STOPBOT");
        uartBuffer = ""; 
        return;
      }

      if (!robotDangChay) { uartBuffer = ""; return; }

      // AUTO / MANUAL
      if (uartBuffer == "AUTO") {
        cheDoManual = false;
        dangChoChuyenAuto = true;
        thoiGianDoiChuyenAuto = millis();
        dungLai();
        Serial.println("AUTO");
      } else if (uartBuffer == "MANUAL") {
        cheDoManual = true;
        dangChoChuyenAuto = false;
        Serial.println("MANUAL");
      }

      // QUẠT
      else if (uartBuffer.startsWith("FAN:")) {
        int val = uartBuffer.substring(4).toInt();
        datTocDoQuat(constrain(val, 0, 255));
      }
      // SPEED
      else if (uartBuffer.startsWith("SPEED:")) {
        tocDoDongCo = uartBuffer.substring(6).toInt();
        tocDoDongCo = constrain(tocDoDongCo, 0, 255);
      }
      // Manual control
      else if (cheDoManual) {
        if (uartBuffer == "S") { dungLai(); }
        else if (uartBuffer == "F" || uartBuffer.startsWith("F:")) {
          if (uartBuffer.indexOf(":") > 0) {
            tocDoDongCo = constrain(uartBuffer.substring(uartBuffer.indexOf(":")+1).toInt(), 0, 255);
          }
          datTocDoCuaCacBanhXe(1, 1, tocDoDongCo);
        }
        else if (uartBuffer == "B" || uartBuffer.startsWith("B:")) {
          if (uartBuffer.indexOf(":") > 0) {
            tocDoDongCo = constrain(uartBuffer.substring(uartBuffer.indexOf(":")+1).toInt(), 0, 255);
          }
          datTocDoCuaCacBanhXe(-1, -1, tocDoDongCo);
        }
        else if (uartBuffer == "L" || uartBuffer.startsWith("L:")) {
          if (uartBuffer.indexOf(":") > 0) {
            tocDoDongCo = constrain(uartBuffer.substring(uartBuffer.indexOf(":")+1).toInt(), 0, 255);
          }
          datTocDoCuaCacBanhXe(-1, 1, tocDoDongCo);
        }
        else if (uartBuffer == "R" || uartBuffer.startsWith("R:")) {
          if (uartBuffer.indexOf(":") > 0) {
            tocDoDongCo = constrain(uartBuffer.substring(uartBuffer.indexOf(":")+1).toInt(), 0, 255);
          }
          datTocDoCuaCacBanhXe(1, -1, tocDoDongCo);
        }
      }

      uartBuffer = "";
    } else {
      uartBuffer += c;
      if (uartBuffer.length() > 100) uartBuffer = "";
    }
  }
}

// ==================== 9. SETUP ====================
void setup() {
  Serial.begin(115200);

  pinMode(CHAN_IN1, OUTPUT); pinMode(CHAN_IN2, OUTPUT);
  pinMode(CHAN_IN3, OUTPUT); pinMode(CHAN_IN4, OUTPUT);
  pinMode(CHAN_ENA, OUTPUT); pinMode(CHAN_ENB, OUTPUT);

  pinMode(CHAN_ENC, OUTPUT); pinMode(CHAN_INC1, OUTPUT); pinMode(CHAN_INC2, OUTPUT);

  pinMode(CHAN_TRIG, OUTPUT); pinMode(CHAN_ECHO, INPUT);

  pinMode(IR_TREN, INPUT); pinMode(IR_DUOI, INPUT);
  pinMode(IR_TRAI, INPUT); pinMode(IR_PHAI, INPUT);

  pinMode(NUT_BAT_TAT, INPUT_PULLUP);

  dungLai();
  analogWrite(CHAN_ENC, 0);
}

// ==================== 10. LOOP ====================
void loop() {
  unsigned long now = millis();

  xuLyNutBatTat();
  xuLyLenhUART();

  if (!robotDangChay) { delay(10); return; }
  if (kiemTraChongRoi()) { delay(5); return; }

  // Đang chờ 1 giây để vào chế độ Auto
  if (dangChoChuyenAuto) {
    if (millis() - thoiGianDoiChuyenAuto >= 1000) {
      dangChoChuyenAuto = false;
      cheDoManual = false;
    } else { dungLai(); delay(10); return; }
  }

  if (cheDoManual) { delay(10); return; }

  // CHẾ ĐỘ AUTO
  switch (trangThaiTranh) {
    case TIEN:
      diThang();
      if (now - tgDocCamBienTruoc >= THOI_GIAN_DOC_CAM_BIEN) {
        tgDocCamBienTruoc = now;
        long kc = docKhoangCachCm();
        if (kc <= NGUONG_VAT_CAN) {
          dungLai();
          trangThaiTranh = LUI;
          tgBatDauHanhDong = now;
        }
      }
      break;

    case LUI:
      diLui();
      if (now - tgBatDauHanhDong >= THOI_GIAN_LUI) {
        dungLai();
        if (!daReTrai && !daRePhai) { trangThaiTranh = RE_TRAI; daReTrai = true; }
        else if (daReTrai && !daRePhai) { trangThaiTranh = RE_PHAI; daRePhai = true; }
        else { daReTrai = daRePhai = false; trangThaiTranh = TIEN; }
        tgBatDauHanhDong = now;
      }
      break;

    case RE_TRAI:
      reTrai();
      if (now - tgBatDauHanhDong >= THOI_GIAN_RE) {
        dungLai();
        trangThaiTranh = TIEN;
        tgBatDauHanhDong = now;
      }
      break;

    case RE_PHAI:
      rePhai();
      if (now - tgBatDauHanhDong >= THOI_GIAN_RE) {
        dungLai();
        trangThaiTranh = TIEN;
        tgBatDauHanhDong = now;
      }
      break;
  }

  delay(5);
}
