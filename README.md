# ROBOT HÚT BỤI THÔNG MINH(SMART ROBOT-VACUUM CLEAN)

🌟 Tính Năng
✅ hút rác, bụi bẩn 
✅ tự động di chuyển khắp nhà
✅ có hệ thống chống rơi
✅ có hệ thống tránh vật cản
✅ có cục bin để sạc khi hết bin

📌 Yêu Cầu Hệ Thống
🖥️ Phần Mềm
ARDUINO IDE

🚀 Hướng Dẫn Cài Đặt & Chạy Hệ Thống
1️⃣Thư viện và khai báo phần cứng
#include <Arduino.h>

3️⃣ Khai báo chân cảm biến
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
const int IR_DUOI = A2;
const int IR_TREN = A3;
const int IR_TRAI = A4;
const int IR_PHAI = A5;
const int NUT_BAT_TAT = 2;

| Chân | Chức năng                       |
| ---- | --------------------------------|
| 2    | nút bấm tắt/bật                 |
| 5,6  | điều chỉnh tốc độ động cơ       |
| 7-10 | điều khiển 2 bánh xe            |
| 11   | điều khiển tốc độ bánh xe       |
| 12-13| điều khiểu chiều quay bánh xe   |
| A0-A1| điều chỉnh cảm biến siêu âm     |
| A2-A6| phát hiện rơi 4 hướng           |
4️⃣ Servo kiểm tra
bool kiemTraChongRoi() {
  if (millis() - tgChongRoiTruoc < 100) return false; // Kiểm tra mỗi 100ms
  tgChongRoiTruoc = millis();
void closeGate(Servo &gate) {
  gate.write(90); // đóng
}

5️⃣ nút bật tắt
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

🎯 Mục Tiêu
Nâng cao khả năng hút và tránh rơi.
Dễ dàng tích hợp với hệ thống IoT để giám sát từ xa.
🚀 Hãy triển khai ngay và trải nghiệm sự tiện lợi! 🚀

📝 Bản quyền
© 2025 Nguyễn Văn Hiếu-Nhóm 10-CNTT_17-01, Khoa Công nghệ Thông tin, Đại học Đại Nam. Mọi quyền được bảo lưu.

Được thực hiện bởi 💻 Nhóm 10-CNTT_17-01 tại Đại học Đại Nam
Email cá nhân : hieucon396@gmail.com

