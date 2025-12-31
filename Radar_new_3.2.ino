#define BLYNK_TEMPLATE_ID "TMPL6L3ua4elI"
#define BLYNK_TEMPLATE_NAME "RADAR HLK LD2410"
#define BLYNK_AUTH_TOKEN "9RNthDNHOUThlgEJajVh8GrrkPSphQuC"

#include <stdio.h>
#include <math.h>
#include <string>

#include <ld2410.h>
ld2410 radar;

#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <BlynkSimpleEsp32.h>

#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_TX_PIN 32
#define RADAR_RX_PIN 33

// Các giá trị cho Led RGB
#define ANALOG_PIN 34  // Chân ADC để đo *volt
#define red 22
#define green 23
float volt = 0.00;
int led = 2;

uint32_t lastReading = 0;
int Matrix[10][100];
float Avg[10];           // cột trung bình (float)
int ColsSetup = 10;     //Thiết lập số cột cho Matrix



// Wi-Fi credentials
const char *ssid = "ABC123";
const char *password = "ngaymoitotlanh";
//Laptop
// const char *ssid = "abc123";
// const char *password = "ngaymoitotlanh";
// const char *ssid = "418-H2";
// const char *password = "helloworld123";

const char* scriptUrl = "https://script.google.com/macros/s/AKfycbzWlDg9-wnMKEU5n03AP5EZGxzWvIn0S1HnFC-AeFwiHhv2pMObE_Cv9OGvLY9BhsY4yg/exec";

void setup(void) {
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);

  MONITOR_SERIAL.begin(115200);
  RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_TX_PIN, RADAR_RX_PIN);
  // Cú pháp (baudrate, config, RX_pin, TX_pin)

  delay(3000);
  radar.begin(RADAR_SERIAL);
  MONITOR_SERIAL.print(F("\nConnect LD2410 radar TX to GPIO:"));
  MONITOR_SERIAL.println(RADAR_RX_PIN);
  MONITOR_SERIAL.print(F("Connect LD2410 radar RX to GPIO:"));
  MONITOR_SERIAL.println(RADAR_TX_PIN);
  MONITOR_SERIAL.println();

  // Kết nối Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    VoltToLed(&volt, &led);
    delay(500);
    digitalWrite(green,LOW);
    digitalWrite(red,LOW);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Blynk begin
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
}

void loop() {
  static int col = 0;
  radar.read();
  if (radar.isConnected() && millis() - lastReading > 100){ //chuẩn là 100 tương đương 0.1s
   lastReading = millis();
   VoltToLed(&volt, &led);
   RecordData(Matrix, col);
   col++;
   }
   if(col==ColsSetup){
    ShowMatrix(Matrix, 0, 6, 0, 9);

    for(int i=0; i < 7; i++){
         Avg[i] = Mean(i, Matrix);
    }
    
    
    // Các giá trị dữ liệu cảm biến
    char pose[100] = " ";
    float distance = -1.00;
    int zone = -1;
    int detected = 1;

    if( Avg[0] == 0 && Avg[1] == 0 && Avg[4] == 0) detected = 0;

    if(detected == 0){
      strcpy(pose, "Không phát hiện người");
    }
    else{
       zone = Zone(Avg[2], Avg[5], &distance);
      Conclude(zone, Avg[3], Avg[6], pose);
    }

    Serial.print(" | zone: ");
    Serial.print(zone);
    Serial.print(" | pose: ");
    Serial.print(pose);
    Serial.print(" | distance: ");
    Serial.println(distance);

//   // ---------------- Gửi dữ liệu lên Google Sheets ----------------
//   if (WiFi.status() == WL_CONNECTED) {


//       HTTPClient http;
//       http.begin(scriptUrl);  // URL của Google Apps Script
//       http.addHeader("Content-Type", "application/json");


//     // Tạo chuỗi JSON dữ liệu
//     String json = "{";
//     json += "\"presenceDetected\":\"" + String(Avg[0],2) + "\",";
//     json += "\"stationaryTargetDetected\":\"" + String(Avg[1],2) + "\",";
//     json += "\"stationaryTargetDistance\":\"" + String(Avg[2],2) + "\",";
//     json += "\"stationaryTargetEnergy\":\"" + String(Avg[3],2) + "\",";
//     json += "\"movingTargetDetected\":\"" + String(Avg[4],2) + "\",";
//     json += "\"movingTargetDistance\":\"" + String(Avg[5],2) + "\",";
//     json += "\"movingTargetEnergy\":\"" + String(Avg[6],2) + "\",";
//     json += "\"zone\":\"" + String(zone) + "\",";
//     json += "\"distance\":\"" + String(distance) + "\",";
//     json += "\"pose\":\"" + String(pose) + "\"";
//     json += "}";
//     int httpResponseCode = http.POST(json);
    
//     if (httpResponseCode > 0) {
//       Serial.print("Data sent to Google Sheets, code: ");
//       Serial.println(httpResponseCode);
//     } else {
//       Serial.print("Error sending data: ");
//       Serial.println(httpResponseCode);
//     }
//     http.end();
// } else {
//     Serial.println("WiFi disconnected!");
//     }

      // ---------------- Gửi dữ liệu lên Blynk ----------------
      Blynk.virtualWrite(V0, String(pose));
      Blynk.virtualWrite(V1, String(distance, 2));
      Blynk.run();


    MONITOR_SERIAL.println();

    col = 0;
    }
}

// ---------------------------------------------------------------------------------------------------- LedRGB
float VoltToLed(float *volt, int *led) {
  *volt = ((analogRead(ANALOG_PIN)/4095.0)*4.0*3.3) + 0.7;  // Đọc giá trị ADC (0 - 4095) ~ (0 - 3.3V)
  int zone;

  if(*volt > 7.5)zone = 5;
  else if(*volt > 7 && *volt <= 7.5)zone = 4;
  else if(*volt > 6 && *volt <= 7)zone = 3;
  else if(*volt > 5.5 && *volt <= 6)zone = 2;
  else if(*volt > 5 && *volt <= 5.5)zone = 1;
  else zone = 0;

  switch(zone){
  case 5:
      digitalWrite(red,LOW);
      digitalWrite(green,HIGH);
      *led = 2 ;
      break;
  case 4:
      if (*led == 2){
        digitalWrite(red,LOW);
        digitalWrite(green,HIGH);
        }
      else{
        digitalWrite(green,HIGH);
        digitalWrite(red,HIGH);
        }
      break;
  case 3:
      digitalWrite(green,HIGH);
      digitalWrite(red,HIGH);
      *led = 1 ;
      break;
  case 2:
      if (*led == 1){
        digitalWrite(green,HIGH);
        digitalWrite(red,HIGH);
        }
      else{
        digitalWrite(green,LOW);
        digitalWrite(red,HIGH);
        }
      break;
  case 1:
      digitalWrite(green,LOW);
      digitalWrite(red,HIGH);
      *led = 0 ;
      break;
  case 0:
      while (*volt <= 4){
      digitalWrite(green,LOW);
      digitalWrite(red,LOW);
      }   
      }
  return *volt;
}
//----------------------------------------------------------------------------------------------------Matrix with 4 initial parameters
void RecordData(int Matrix[][100], int col){

         Matrix[0][col] = radar.presenceDetected();
         Matrix[1][col] = radar.stationaryTargetDetected();
         Matrix[2][col] = radar.stationaryTargetDistance();
         Matrix[3][col] = radar.stationaryTargetEnergy(); 
         Matrix[4][col] = radar.movingTargetDetected();
         Matrix[5][col] = radar.movingTargetDistance(); 
         Matrix[6][col] = radar.movingTargetEnergy();

}
//----------------------------------------------------------------------------------------------------ShowMatrix
void ShowMatrix(int Matrix[][100], int BeginRow, int EndRow, int BeginCol, int EndCol) {
  String space = "";
  space += "Số lần đo            : 1   2   3   4   5   6   7   8   9   10 \n";

  for (int j = BeginRow; j <= EndRow; j++) {
    if (j == 0) space += "Phát hiện người      : ";
    else if (j == 1) space += "Phát hiện tĩnh       : ";
    else if (j == 2) space += "Khoảng cách tĩnh     : ";
    else if (j == 3) space += "Năng lượng tĩnh      : ";
    else if (j == 4) space += "Phát hiện động       : ";
    else if (j == 5) space += "Khoảng cách động     : ";
    else if (j == 6) space += "Năng lượng động      : ";

    for (int i = BeginCol; i <= EndCol; i++) {
      space += String(Matrix[j][i]);
      if (Matrix[j][i] <= 9) space += "  |";
      else if (Matrix[j][i] <= 99) space += " |";
      else space += "|";
    }
    space += "\n";
  }
  Serial.print(space);
}
//---------------------------------------------------------------------------------------------------- Tính tổng
float Sum(int row, int Matrix[][100]){
  float sum=0;
  int col=0;
  
  while(col<ColsSetup){
    sum += Matrix[row][col];
    col++;
  }
  return sum;
}
//---------------------------------------------------------------------------------------------------- Tính Trung bình
float Mean(int row, int Matrix[][100]) {
  return Sum(row, Matrix) / ColsSetup;
}
//---------------------------------------------------------------------------------------------------- Tính Khoảng cách 
float Distance2D(float x1, float y1, float x2, float y2) {
    return sqrt(pow((x1-x2),2) + pow((y1-y2),2));
}
float Distance3D(float x1, float y1, float x2, float y2, float z1, float z2) {
    return sqrt(pow((x1-x2),2) + pow((y1-y2),2) + pow((z1-z2),2));
}
//----------------------------------------------------------------------------------------------------Dự đoán
int Zone(float StationaryDistance, float MovingDistance, float *distance) {
  float ZoneMean[8][2] = {
  {55.2, 76.2},    // 0: 0-0.75
  {144.4, 160.6},    // 1: 0.75-1.5
  {220.1, 245.3},    // 2: 1.5-2.25
  {290.2, 313.4},    // 3: 2.25-3
  {362.6, 382.2},    // 4: 3-3.75
  {430.5, 444.2},    // 5: 3.75-4.5
  {506.5, 521.2},    // 6: 4.5-5.25
  {560.8, 567.5},    // 7: 5.25-6
  };
  int euclid = 1000;
  float euclid_new = 0;
  int zone = 0;
  for (int i = 0; i < 8; i++){
      euclid_new = Distance2D(MovingDistance, StationaryDistance, ZoneMean[i][0], ZoneMean[i][1]);
      if ( euclid > euclid_new){
        euclid = euclid_new;
        zone = i;
      }
  }
  *distance = (StationaryDistance + MovingDistance)/2;
  return zone;
}
void Conclude(int zone,float StationaryEnergy,float MovingEnergy, char pose[100]){
  
    float euclid = 1000;
    float euclid_new = 0;
    int pose_human= 0;
   
    if(zone==0){

      float PoseMean[3][2] = {
          {45.0, 97.9},    // 0: Tĩnh
          {60.1, 100.0},    // 1: Vận động nhẹ
          {95.4, 99.9}    // 2: Vận động mạnh
        };
       
        for (int i = 0; i < 3; i++){
          euclid_new = Distance2D(MovingEnergy, StationaryEnergy, PoseMean[i][0], PoseMean[i][1]);
          if ( euclid > euclid_new){
            euclid = euclid_new;
            pose_human= i;
          }
        }

      if(pose_human== 0) strcpy(pose, "Tĩnh");
      if(pose_human== 1) strcpy(pose, "Vận động nhẹ");
      if(pose_human== 2) strcpy(pose, "Vận động mạnh");

    }
    else if(zone==1){

      float PoseMean[3][2] = {
          {17.8, 85.6},    // 0: Tĩnh
          {33.4, 99.6},    // 1: Vận động nhẹ
          {78.6, 100.0}    // 2: Vận động mạnh
        };

        for (int i = 0; i < 3; i++){
          euclid_new = Distance2D(MovingEnergy, StationaryEnergy, PoseMean[i][0], PoseMean[i][1]);
          if ( euclid > euclid_new){
            euclid = euclid_new;
            pose_human= i;
          }
        }

      if(pose_human== 0) strcpy(pose, "Tĩnh");
      if(pose_human== 1) strcpy(pose, "Vận động nhẹ");
      if(pose_human== 2) strcpy(pose, "Vận động mạnh");

    }
    else if(zone==2){

      float PoseMean[3][2] = {
        {7.3, 76.3},    // 0: Tĩnh
        {20.3, 99.5},    // 1: Vận động nhẹ
        {50.4, 100.0}    // 2: Vận động mạnh
      };

        for (int i = 0; i < 3; i++){
          euclid_new = Distance2D(MovingEnergy, StationaryEnergy, PoseMean[i][0], PoseMean[i][1]);
          if ( euclid > euclid_new){
            euclid = euclid_new;
            pose_human= i;
          }
        }

      if(pose_human== 0) strcpy(pose, "Tĩnh");
      if(pose_human== 1) strcpy(pose, "Vận động nhẹ");
      if(pose_human== 2) strcpy(pose, "Vận động mạnh");

    }
    else if(zone==3){

      float PoseMean[3][2] = {
          {5.0, 65.2},    // 0: Tĩnh
          {11.9, 97.6},    // 1: Vận động nhẹ
          {31.4, 100.0}    // 2: Vận động mạnh
        };

        for (int i = 0; i < 3; i++){
          euclid_new = Distance2D(MovingEnergy, StationaryEnergy, PoseMean[i][0], PoseMean[i][1]);
          if ( euclid > euclid_new){
            euclid = euclid_new;
            pose_human= i;
          }
        }

      if(pose_human== 0) strcpy(pose, "Tĩnh");
      if(pose_human== 1) strcpy(pose, "Vận động nhẹ");
      if(pose_human== 2) strcpy(pose, "Vận động mạnh");

      
    }
    else if(zone==4){

      float PoseMean[3][2] = {
          {3.1, 69.1},    // 0: Tĩnh
          {4.1, 87.3},    // 1: Vận động nhẹ
          {27.0, 99.9}    // 2: Vận động mạnh
          
      };

      

        for (int i = 0; i < 3; i++){
          euclid_new = Distance2D(MovingEnergy, StationaryEnergy, PoseMean[i][0], PoseMean[i][1]);
          if ( euclid > euclid_new){
            euclid = euclid_new;
            pose_human= i;
          }
        }

      if(pose_human== 0) strcpy(pose, "Tĩnh");
      if(pose_human== 1) strcpy(pose, "Vận động nhẹ");
      if(pose_human== 2) strcpy(pose, "Vận động mạnh");

    }
    else if(zone==5){

      float PoseMean[3][2] = {
          {1.2, 54.5},    // 0: Tĩnh
          {5.2, 83.3},    // 1: Vận động nhẹ
          {23.3, 94.7}    // 2: Vận động mạnh
          
      };

       for (int i = 0; i < 3; i++){
          euclid_new = Distance2D(MovingEnergy, StationaryEnergy, PoseMean[i][0], PoseMean[i][1]);
          if ( euclid > euclid_new){
            euclid = euclid_new;
            pose_human= i;
          }
        }

      if(pose_human== 0) strcpy(pose, "Tĩnh");
      if(pose_human== 1) strcpy(pose, "Vận động nhẹ");
      if(pose_human== 2) strcpy(pose, "Vận động mạnh");
    }
    else if(zone==6){
      float PoseMean[3][2] = {
          {1.0, 45.3},    // 0: Tĩnh
          {2.4, 63.7},    // 1: Vận động nhẹ
          {14.1, 58.8}    // 2: Vận động mạnh
         
      };

        for (int i = 0; i < 3; i++){
          euclid_new = Distance2D(MovingEnergy, StationaryEnergy, PoseMean[i][0], PoseMean[i][1]);
          if ( euclid > euclid_new){
            euclid = euclid_new;
            pose_human= i;
          }
        }

      if(pose_human== 0) strcpy(pose, "Tĩnh");
      if(pose_human== 1) strcpy(pose, "Vận động nhẹ");
      if(pose_human== 2) strcpy(pose, "Vận động mạnh");
    }
    else if(zone==7){
      float PoseMean[3][2] = {
          {0.6, 37.9},    // 0: Tĩnh
          {0.2, 45.2},    // 1: Vận động nhẹ
          {6.3, 37.2}    // 2: Vận động mạnh
          
      };

     
        for (int i = 0; i < 3; i++){
          euclid_new = Distance2D(MovingEnergy, StationaryEnergy, PoseMean[i][0], PoseMean[i][1]);
          if ( euclid > euclid_new){
            euclid = euclid_new;
            pose_human= i;
          }
        }

      if(pose_human== 0) strcpy(pose, "Tĩnh");
      if(pose_human== 1) strcpy(pose, "Vận động nhẹ");
      if(pose_human== 2) strcpy(pose, "Vận động mạnh");
    }
}
