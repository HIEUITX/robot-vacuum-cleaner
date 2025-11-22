#include <Arduino.h>

// ==================== 1. KHAI BÁO CHÂN ====================
const int CHAN_ENA = 5;
const int CHAN_IN1 = 8;
const int CHAN_IN2 = 7;
const int CHAN_ENB = 6;
const int CHAN_IN3 = 9;
const int CHAN_IN4 = 10;

const int CHAN_ENC  = 11;
const int CHAN_INC1 = 12;
const int CHAN_INC2 = 13;

const int CHAN_TRIG = A0;
const int CHAN_ECHO = A1;

// Cảm biến chống rơi (IR)
const int IR_DUOI = A2;
const int IR_TREN = A3;
const int IR_TRAI = A4;
const int IR_PHAI = A5;

// Nút bật/tắt
const int NUT_BAT_TAT = 2;

// ==================== 2. HẰNG SỐ ====================
#define TOC_DO_MAX     180
#define TOC_DO_QUAT    210
#define NGUONG_VAT_CAN 10      // cm

// Thời gian các hành động (ms)
#define THOI_GIAN_LUI       450
#define THOI_GIAN_RE        600
// #define THOI_GIAN_TIEN_SAU_RE 800 // KHÔNG CẦN THIẾT
#define THOI_GIAN_CHO_SAU_HANH_DONG 100
#define THOI_GIAN_DOC_CAM_BIEN 100   // Tần suất đọc cảm biến (ms)

// Chống dội nút
#define THOI_GIAN_CHONG_DOI_NUT 50

// ==================== 3. BIẾN TRẠNG THÁI ====================
bool robotDangChay = false;
bool trangThaiNutCu = HIGH;
unsigned long thoiGianNhanNutCu = 0;

// Biến trạng thái tránh vật cản
enum TrangThaiTranh { TIEN, LUI, RE_TRAI, RE_PHAI }; // Đã loại bỏ CHO_SAU_RE
TrangThaiTranh trangThaiTranh = TIEN;

bool daReTrai = false;
bool daRePhai = false;

// Biến thời gian non-blocking
unsigned long tgBatDauHanhDong = 0;
unsigned long tgDocCamBienTruoc = 0;
unsigned long tgChongRoiTruoc = 0;

// ==================== 4. ĐIỀU KHIỂN ĐỘNG CƠ ====================
void dieuKhienMotBanhXe(int huong, int tocDo, int chanDau1, int chanDau2, int chanPWM) {
  tocDo = constrain(tocDo, 0, 255);
  analogWrite(chanPWM, (huong != 0) ? tocDo : 0);

  if (huong > 0) {
    digitalWrite(chanDau1, HIGH);  digitalWrite(chanDau2, LOW);
  } else if (huong < 0) {
    digitalWrite(chanDau1, LOW);   digitalWrite(chanDau2, HIGH);
  } else {
    digitalWrite(chanDau1, LOW);   digitalWrite(chanDau2, LOW);
  }
}

void datTocDoCuaCacBanhXe(int huongTrai, int huongPhai, int tocDo) {
  dieuKhienMotBanhXe(huongTrai, tocDo, CHAN_IN1, CHAN_IN2, CHAN_ENA);
  dieuKhienMotBanhXe(huongPhai, tocDo, CHAN_IN3, CHAN_IN4, CHAN_ENB);
}

void dungLai()  { datTocDoCuaCacBanhXe(0, 0, 0); }
void diThang()   { datTocDoCuaCacBanhXe(1, 1, TOC_DO_MAX); }
void diLui()     { datTocDoCuaCacBanhXe(-1, -1, TOC_DO_MAX); }
void reTrai()    { datTocDoCuaCacBanhXe(-1, 1, TOC_DO_MAX); }
void rePhai()    { datTocDoCuaCacBanhXe(1, -1, TOC_DO_MAX); }

void datTocDoQuat(int tocDo) {
  digitalWrite(CHAN_INC1, HIGH);
  digitalWrite(CHAN_INC2, LOW);
  analogWrite(CHAN_ENC, constrain(tocDo, 0, 255));
}

// ==================== 5. SIÊU ÂM ====================
long docKhoangCachCm() {
  digitalWrite(CHAN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(CHAN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(CHAN_TRIG, LOW);

  long t = pulseIn(CHAN_ECHO, HIGH, 30000);
  if (t == 0) return 999;
  return t * 0.034 / 2;
}

// ==================== 6. CHỐNG RƠI ====================
bool kiemTraChongRoi() {
  if (millis() - tgChongRoiTruoc < 100) return false; // Kiểm tra mỗi 100ms
  tgChongRoiTruoc = millis();

  int tren = digitalRead(IR_TREN);
  int duoi = digitalRead(IR_DUOI);
  int trai = digitalRead(IR_TRAI);
  int phai = digitalRead(IR_PHAI);

  bool roiTren  = (tren == HIGH);
  bool roiDuoi  = (duoi == HIGH);
  bool roiTrai  = (trai == HIGH);
  bool roiPhai  = (phai == HIGH);

  if (!roiTren && !roiDuoi && !roiTrai && !roiPhai) return false;

  Serial.print("NGUY CƠ RƠI - T="); Serial.print(roiTren);
  Serial.print(" D="); Serial.print(roiDuoi);
  Serial.print(" Tr="); Serial.print(roiTrai);
  Serial.print(" Ph="); Serial.println(roiPhai);

  dungLai();
  delay(50); // nhỏ thôi, vẫn cần dừng chút để ổn định

  if (roiTren) { diLui(); delay(400); reTrai(); delay(500); }
  else if (roiDuoi) { diThang(); delay(200); }
  else if (roiTrai) { rePhai(); delay(300); }
  else if (roiPhai) { reTrai(); delay(300); }

  dungLai();
  trangThaiTranh = TIEN; // Reset trạng thái tránh vật cản
  daReTrai = daRePhai = false; // Reset biến rẽ
  return true;
}

// ==================== 7. NÚT BẬT/TẮT ====================
void xuLyNutBatTat() {
  bool nutHienTai = digitalRead(NUT_BAT_TAT);

  if (trangThaiNutCu == HIGH && nutHienTai == LOW &&
      (millis() - thoiGianNhanNutCu > THOI_GIAN_CHONG_DOI_NUT)) {

    robotDangChay = !robotDangChay;

    if (robotDangChay) {
      Serial.println("ROBOT BẬT - Bắt đầu chạy!");
      datTocDoQuat(TOC_DO_QUAT);
      trangThaiTranh = TIEN;
      daReTrai = daRePhai = false;
    } else {
      Serial.println("ROBOT TẮT");
      dungLai();
      analogWrite(CHAN_ENC, 0);
    }
    thoiGianNhanNutCu = millis();
  }
  trangThaiNutCu = nutHienTai;
}

// ==================== 8. SETUP ====================
void setup() {
  Serial.begin(9600);

  // Động cơ
  pinMode(CHAN_IN1, OUTPUT); pinMode(CHAN_IN2, OUTPUT);
  pinMode(CHAN_IN3, OUTPUT); pinMode(CHAN_IN4, OUTPUT);
  pinMode(CHAN_ENA, OUTPUT);  pinMode(CHAN_ENB, OUTPUT);
  pinMode(CHAN_ENC, OUTPUT);  pinMode(CHAN_INC1, OUTPUT); pinMode(CHAN_INC2, OUTPUT);

  // Cảm biến
  pinMode(CHAN_TRIG, OUTPUT);
  pinMode(CHAN_ECHO, INPUT);
  pinMode(IR_TREN, INPUT); pinMode(IR_DUOI, INPUT);
  pinMode(IR_TRAI, INPUT); pinMode(IR_PHAI, INPUT);

  pinMode(NUT_BAT_TAT, INPUT_PULLUP);

  dungLai();
  analogWrite(CHAN_ENC, 0);

  Serial.println("=== ROBOT HÚT BỤI - Nhấn nút D2 để BẬT/TẮT ===");
}

// ==================== 9. LOOP CHÍNH (NON-BLOCKING) ====================
void loop() {
  unsigned long now = millis();

  xuLyNutBatTat();           // Luôn kiểm tra nút
  if (!robotDangChay) {
    delay(10);
    return;
  }

  // Kiểm tra chống rơi (không block)
  kiemTraChongRoi();

  // === TRẠNG THÁI MÁY TRÁNH VẬT CẢN (state machine) ===
  switch (trangThaiTranh) {
    case TIEN:
      diThang();
      if (now - tgDocCamBienTruoc >= THOI_GIAN_DOC_CAM_BIEN) {
        tgDocCamBienTruoc = now;
        long kc = docKhoangCachCm();
        Serial.print("Khoảng cách: "); Serial.print(kc); Serial.println(" cm");

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
        if (!daReTrai && !daRePhai) {
          trangThaiTranh = RE_TRAI;
          daReTrai = true;
          Serial.println("Rẽ trái");
        }
        else if (daReTrai && !daRePhai) {
          trangThaiTranh = RE_PHAI;
          daRePhai = true;
          Serial.println("Rẽ phải");
        }
        else {
          trangThaiTranh = RE_TRAI; 
          daReTrai = daRePhai = false;
          Serial.println("Quay 180 độ");
        }
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
                                 
  
  delay(10);
}