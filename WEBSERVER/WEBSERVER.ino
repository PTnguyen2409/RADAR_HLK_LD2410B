#include <WiFi.h>
#include <WebServer.h>
#include <index_html.h>

// Wi-Fi credentials
const char *ssid = "ABC123";
const char *password = "ngaymoitotlanh";

// Web server trên cổng 80
WebServer server(80);

// Các giá trị dữ liệu cảm biến
float Value1 = 0;
float Value2 = 0;

// ---------------------------------------------------------------------------------------------------- Setup
void setup() {
  Serial.begin(115200);

  // Kết nối Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Cấu hình route cho server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleData);

  // Bắt đầu server
  server.begin();
  Serial.println("HTTP server started.");
}

// ---------------------------------------------------------------------------------------------------- Handle Root (Serve HTML)
void handleRoot() {
  server.send_P(200, "text/html", index_html);  // Gửi trang HTML từ PROGMEM
}

// ---------------------------------------------------------------------------------------------------- Handle Data (Send JSON)
void handleData() {
  String json = "{";
  json += "\"row1\":" + String(Value1, 2) + ",";
  json += "\"row2\":" + String(Value2, 2);
  json += "}";

  server.send(200, "application/json", json);
}

// ---------------------------------------------------------------------------------------------------- Loop
void loop() {
  // Tạo dữ liệu ngẫu nhiên cho các cảm biến
  Value1 = random(0, 100);
  Value2 = random(0, 100);

  // In dữ liệu lên Serial Monitor để theo dõi
  Serial.print("Sensor 1: ");
  Serial.print(Value1);
  Serial.print(", Sensor 2: ");
  Serial.println(Value2);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Xử lý request từ client
  server.handleClient();

  delay(1000);
}
