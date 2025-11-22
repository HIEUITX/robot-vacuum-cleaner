# ROBOT HÚT BỤI THÔNG MINH(SMART ROBOT-VACUUM CLEAN)

DaiNam University Logo AIoTLab Logo

Hệ thống quản lý bãi đỗ xe tích hợp nhận diện biển số xe tự động, với giao diện web hiện đại để quản lý bãi đỗ một cách dễ dàng và hiệu quả. Dữ liệu điểm danh được lưu trữ trong lite SQL

🌟 Sơ Đồ Kết Nối Mạch


🌟 WEB QUẢN LÝ (Minh họa)


🌟 Tính Năng
✅ Nhận diện biển số xe tự động với EasyOCR
✅ Quản lý 6 vị trí đỗ xe
✅ Giao diện web hiện đại & dễ sử dụng
✅ Lưu trữ lịch sử đỗ xe
✅ Thống kê số lượng xe đang đỗ và chỗ trống
✅ Hiển thị thời gian vào/ra của xe

📌 Yêu Cầu Hệ Thống
🖥️ Phần Mềm
Python 3.7+
Các thư viện Python cần thiết:
pip install Flask EasyOCR OpenCV-Python NumPy Pillow
Flask==2.3.3
Pillow==10.2.0
numpy==1.26.4
opencv-python==4.9.0.80
torch==2.2.0
torchvision==0.17.0
easyocr==1.7.1 
🚀 Hướng Dẫn Cài Đặt & Chạy Hệ Thống
1️⃣ Cài Đặt Môi Trường
Đảm bảo bạn đã cài đặt Python 3.7+. Nếu chưa, hãy tải về từ Python.org.
Cài đặt các thư viện cần thiết bằng lệnh:
pip install Flask EasyOCR OpenCV-Python NumPy Pillow
2️⃣ Thư viện và khai báo phần cứng
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
👉 Dùng 3 thư viện:

Servo.h – điều khiển servo barie

LiquidCrystal_I2C – điều khiển màn LCD 16x2 qua I2C

Wire.h – giao tiếp I2C

3️⃣ Khai báo chân cảm biến
#define SENSOR_ENTRY 2
#define SENSOR_EXIT  3
#define SLOT1 4
#define SLOT2 5
#define SLOT3 6
#define SLOT4 7
#define SERVO_ENTRY_PIN 9
#define SERVO_EXIT_PIN 10
| Chân | Chức năng                   |
| ---- | --------------------------- |
| 2    | Cảm biến vào (entry sensor) |
| 3    | Cảm biến ra (exit sensor)   |
| 4–7  | Cảm biến chỗ đỗ S1–S4       |
| 9    | Servo cổng vào              |
| 10   | Servo cổng ra               |
4️⃣ Servo điều khiển barie
void openGate(Servo &gate) {
  gate.write(0); // mở
}
void closeGate(Servo &gate) {
  gate.write(90); // đóng
}
Mở: quay 0°

Đóng: quay 90°

Có delay để gate thực sự chuyển động

In LCD để báo trạng thái

Chú ý: servo vào và ra sử dụng cùng một hàm, truyền bằng tham chiếu Servo &gate.
5️⃣ Đọc trạng thái slot + cập nhật LCD
slotState[i] == LOW ? "X" : "O";
Low = có xe -> X Hight = trống xe -> O

S1:X S2:O
S3:O S4:X
6️⃣ Hàm đếm số bãi trống
if (slotState[i] == HIGH) freeCount++;
Hight = trống -> thêm xe vào

7️⃣ Gửi log ra Serial Monitor
Serial.print("[S1:X] ");
In đầy đủ trạng thái của 4 slot và số chỗ trống

Slots: [S1:X] [S2:O] [S3:O] [S4:X] | Free: 2
### 8️⃣  Setup() 
```pythonpinMode(..., INPUT_PULLUP);
Các cảm biến đều được bật PULLUP → bình thường HIGH → kích hoạt LOW.

servoEntry.attach(9);
servoExit.attach(10);
closeGate(servoEntry);
closeGate(servoExit);
→ Barie vào & ra đều đóng khi khởi động.

lcd.init();
lcd.backlight();
updateLCD();
Hiển thị trạng thái slot ngay từ đầu.

⚠️ Lưu Ý
ESP32-CAM không cắm như sơ đồ mạch, mà cắm thẳng vào laptop.
Code Arduino đã được cập nhật và sửa lỗi, không chạy theo video sẽ gặp lỗi.
🎯 Mục Tiêu
Tự động hóa quy trình quản lý bãi đỗ xe bằng công nghệ nhận diện biển số xe.
Nâng cao hiệu suất và giảm thiểu lỗi thủ công trong quản lý xe ra vào.
Dễ dàng tích hợp với hệ thống IoT để giám sát từ xa.
🚀 Hãy triển khai ngay và trải nghiệm sự tiện lợi! 🚀

📝 Bản quyền
© 2025 Vũ Văn Hậu-Nhóm 2-CNTT_17-01, Khoa Công nghệ Thông tin, Đại học Đại Nam. Mọi quyền được bảo lưu.

Được thực hiện bởi 💻 Nhóm 2-CNTT_17-01 tại Đại học Đại Nam
Email cá nhân : vuvanhau10082005@gmail.com
