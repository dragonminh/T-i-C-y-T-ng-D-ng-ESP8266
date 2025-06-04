#define BLYNK_TEMPLATE_ID "TMPL6M_0Pi1RK"
#define BLYNK_TEMPLATE_NAME "terarium"
#define BLYNK_AUTH_TOKEN "wqb-nwetZSBIra8jSJFDhfDlsRvBNzJC"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
// Cấu hình chân
#define DHT_PIN   D4          // GPIO2
#define LED_PIN   D5          // GPIO14 (PWM)
#define PS_PIN    D6          // GPIO12
#define FAN_PIN   D7          // GPIO13
#define SOIL_PIN  A0          // Cảm biến độ ẩm đất (analog)
#define DHTTYPE   DHT11
DHT dht(DHT_PIN, DHTTYPE);

BlynkTimer timer;

// ====== WIFI CONFIG ======
char ssid[] = "Dang Thi Huyen";
char pass[] = "05003859718";

// ====== TRẠNG THÁI & BIẾN ẢO ======
bool POWER = false;
int V4_value = 0;  // Quạt
int V6_value = 0;  // Phun sương
int V7_value = 0;  // Độ sáng đèn

// ====== HÀM ĐIỀU KHIỂN ======
void Fan(int state) {
  digitalWrite(FAN_PIN, state != 0 ? HIGH : LOW);
}

void PhunSuong(int state) {
  digitalWrite(PS_PIN, state != 0 ? HIGH : LOW);
}

void Led(int brightness) {
  analogWrite(LED_PIN, brightness);
}

// ====== BLYNK WRITE XỬ LÝ TỪ APP ======
BLYNK_WRITE(V0) { // Bật/tắt toàn hệ thống
  POWER = param.asInt();
  Led(POWER ? V7_value : 0);
  PhunSuong(POWER ? V6_value : 0);
  Fan(POWER ? V4_value : 0);

  if (!POWER) {
    Blynk.virtualWrite(V1, 0);  // Nhiệt độ
    Blynk.virtualWrite(V2, 0);  // Độ ẩm không khí
    Blynk.virtualWrite(V3, 0);  // Độ ẩm đất
    Blynk.virtualWrite(V4, 0);  // Quạt
    Blynk.virtualWrite(V6, 0);  // Phun sương
    Blynk.virtualWrite(V7, 0);  // Đèn
  }
}

BLYNK_WRITE(V4) { // Điều khiển quạt thủ công
  if (POWER) {
    V4_value = param.asInt();
    Fan(V4_value);
  }
}

BLYNK_WRITE(V6) { // Điều khiển phun sương thủ công
  if (POWER) {
    V6_value = param.asInt();
    PhunSuong(V6_value);
  }
}

BLYNK_WRITE(V7) { // Điều chỉnh độ sáng đèn thủ công
  if (POWER) {
    V7_value = param.asInt();
    Led(V7_value);
  }
}

// ====== ĐỌC CẢM BIẾN VÀ GỬI DỮ LIỆU ======
void readSensors() {
  if (POWER) {
    float temp = dht.readTemperature();
    float humi = dht.readHumidity();
    int soilRaw = analogRead(SOIL_PIN);
    float soilMoisture = (soilRaw / 1023.0) * 100;

    // Gửi dữ liệu cảm biến lên Blynk
    if (!isnan(temp))  Blynk.virtualWrite(V1, temp);         // V1: nhiệt độ
    if (!isnan(humi))  Blynk.virtualWrite(V2, humi);         // V2: độ ẩm không khí
                        Blynk.virtualWrite(V3, soilMoisture); // V3: độ ẩm đất

    // ====== ĐIỀU KHIỂN TỰ ĐỘNG ======

    // Quạt tự động nếu nhiệt độ > 30
    if (temp > 30) {
      Fan(1);
      Blynk.virtualWrite(V4, 1);
    } else {
      Fan(V4_value); // Giữ trạng thái thủ công
    }

    // Phun sương tự động nếu độ ẩm không khí < 50%
    if (humi < 50) {
      PhunSuong(1);
      Blynk.virtualWrite(V6, 1);
    } else {
PhunSuong(V6_value); // Giữ trạng thái thủ công
    }

    // Tự động điều chỉnh độ sáng đèn: đất càng khô đèn càng mờ
    int brightness = map(soilRaw, 1023, 0, 0, 1023);
    Led(brightness);
    Blynk.virtualWrite(V7, brightness);
  }
}

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  dht.begin();

  pinMode(SOIL_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PS_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

  timer.setInterval(5000L, readSensors); // Đọc dữ liệu mỗi 5 giây
}

// ====== LOOP ======
void loop() {
  Blynk.run();
  timer.run();
}