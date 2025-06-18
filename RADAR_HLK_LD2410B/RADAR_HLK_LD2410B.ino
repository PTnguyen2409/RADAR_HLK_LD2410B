#include <ld2410.h>
#include <stdio.h>
#include <math.h>


#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_TX_PIN 32
#define RADAR_RX_PIN 33

// Các giá trị dữ liệu cảm biến
char room[100] = " ";
char target[100] = " ";
float distance = 0.00;

ld2410 radar;
uint32_t lastReading = 0;
bool radarConnected = false;

int MatrixData[4][1000];
int ColsSetup = 1000;     //Setup number columns of Matrixvalue <=1000 
int col = 0;

float MatrixValue[4][4];
//----------------------------------------------------------------------------------------------------Set up

void setup(void){
  MONITOR_SERIAL.begin(115200);
  RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_TX_PIN, RADAR_RX_PIN);
  if (radar.begin(RADAR_SERIAL))
  {
    // Radar initialized successfully
  }
  else
  {
    // Radar not connected
  }
}

//----------------------------------------------------------------------------------------------------Matrix with 4 initial parameters

void DataMatrix(int Matrix[20][1000], int col){
  if (radar.presenceDetected()){
    int stationaryDistance = radar.stationaryTargetDistance(); 
    byte stationaryEnergy = radar.stationaryTargetEnergy(); 
    int movingDistance = radar.movingTargetDistance(); 
    byte movingEnergy = radar.movingTargetEnergy();
    
      if (radar.stationaryTargetDetected() & radar.movingTargetDetected()){
         Matrix[0][col] = stationaryDistance; 
         Matrix[1][col] = stationaryEnergy; 
         Matrix[2][col] = movingDistance; 
         Matrix[3][col] = movingEnergy;
      }else if (radar.stationaryTargetDetected()==1 & radar.movingTargetDetected()==0){
         Matrix[0][col] = stationaryDistance; 
         Matrix[1][col] = stationaryEnergy; 
         Matrix[2][col] = 0; 
         Matrix[3][col] = 0;
      }
    }else{
         Matrix[0][col]= 0;
         Matrix[1][col]= 0;
         Matrix[2][col]= 0;
         Matrix[3][col]= 0;
    }     
}

//----------------------------------------------------------------------------------------------------

void ShowMatrixData(int Matrix[20][1000], int BeginRow, int EndRow){
  String space = ""; //initialize empty string
   for(int j=BeginRow;j<EndRow+1;j++){
    for(int i=0;i<ColsSetup+1;i++){
      space += String(Matrix[j][i]); // Add matrix values ​​to string 
      if (Matrix[j][i] <= 9){ 
        space += "  |"; 
      }else if (9 < Matrix[j][i] && Matrix[j][i] <= 99){ 
        space += " |"; 
      }else{ 
        space += "|";
      } 
      }
      space += "\n"; // Print string to Serial Monitor
      } 
      Serial.print(space);
      }

//----------------------------------------------------------------------------------------------------

void ShowMatrixValue(float Matrix[4][4]){
  String space = ""; //initialize empty string
   for(int j=0;j<4;j++){
    for(int i=0;i<4;i++){
      space += String(Matrix[j][i]); // Add matrix values ​​to string 
      if (Matrix[j][i] < 10){ 
        space += "  |"; 
      }else if (10 <= Matrix[j][i] && Matrix[j][i] < 100){ 
        space += " |"; 
      }else if (Matrix[j][i] >=100){ 
        space += "|";
      }else{
        space += "   |";
      }
      }
      space += "\n"; // Print string to Serial Monitor
      } 
      Serial.print(space);
      }

//----------------------------------------------------------------------------------------------------

void ChangePosition(int *a, int *b){
  int bufferchange;
  bufferchange=*a;
  *a=*b;
  *b=bufferchange;
}

//----------------------------------------------------------------------------------------------------

float Median(int row, int Matrix[][1000]) { 
  int temp[ColsSetup]; 
  
  for (int i = 0; i < ColsSetup; i++){
    temp[i] = Matrix[row][i];
    }
    
  for (int i = 0; i < ColsSetup - 1; i++){
    for (int j = i + 1; j < ColsSetup; j++){ 
      if (temp[i] > temp[j]){ 
        ChangePosition(&temp[i], &temp[j]);
        } 
        } 
        }
        
  if (ColsSetup % 2 == 0){ 
    return (temp[ColsSetup/2 - 1] + temp[ColsSetup/2]) / 2.0;
    }else{ 
      return temp[ColsSetup/2]; 
    } 
    }

//----------------------------------------------------------------------------------------------------

float Sum(int row, int Matrix[][1000]){
  float sum=0;
  int col=0;
  
  while(col<ColsSetup){
    sum += Matrix[row][col];
    col++;
  }
  return sum;
}

//----------------------------------------------------------------------------------------------------

float Variance(int row, int Matrix[][1000], float average){
  float sum=0;
  
  for(int i=0;i<ColsSetup;i++){
    sum += pow(Matrix[row][i]-average,2);
  }
  return sum/ColsSetup;
}

//----------------------------------------------------------------------------------------------------

void Conclude(float Matrix[4][4], float *distance, char room[100], char target[100]) {

    float SDistance;
      if (Matrix[0][2] < 20) {
        SDistance = Matrix[0][0];
      } else {
        SDistance = Matrix[0][1];
      }

    float SEnergy;
      if (Matrix[1][2] < 20) {
        SEnergy = Matrix[1][0];
      }else{
        SEnergy = Matrix[1][1];
      }
    
   float MDistance;
      if (Matrix[2][2] < 20) {
        MDistance = Matrix[2][0];
      } else {
        MDistance = Matrix[2][1];
      }
    
   float MEnergy;
      if (Matrix[3][2] < 20) {
        MEnergy = Matrix[3][0];
      }else{
        MEnergy = Matrix[3][1];
      }
      
    
  if ( SEnergy < 40 ) {
    strcpy(room, "Phòng trống");
    strcpy(target, "Không phát hiện mục tiêu");
    *distance = 0.00;
  } else {
    strcpy(room, "Phòng có người");
    *distance = SDistance/100;
    
    if(*distance>0 && *distance<=2){
      if(SEnergy <= 100 && MEnergy <= 5){
        strcpy(target, "Ngồi");
      }else if(SEnergy == 100 && MEnergy >= 60 && MDistance > 0 ){
        strcpy(target, "Đứng");
      }else if(SEnergy == 100 && MEnergy > 5 &&  MEnergy <= 60 && target == "Ngồi"){
        strcpy(target, "Ngồi");
      }else if(SEnergy == 100 && MEnergy > 5 && MEnergy <= 60 && target == "Đứng"){
        strcpy(target, "Đứng");
      }
    }else if(*distance>2 && *distance<=3){
      if( SEnergy < 100 ){
        strcpy(target, "Ngồi");
      }else if(SEnergy == 100 && MEnergy >= 20){
        strcpy(target, "Đứng");
      }else if( MEnergy < 20 && target == "Ngồi"){
        strcpy(target, "Ngồi");
      }else if( MEnergy < 20 && target == "Đứng"){
        strcpy(target, "Đứng");
      }
    }else if(*distance>3 && *distance<=4){
      if( SEnergy < 70 ){
        strcpy(target, "Ngồi");
      }else if(SEnergy >=70 ){
        strcpy(target, "Đứng");
      }
    }else if(*distance>4 && *distance<=5){
      strcpy(target, "Chưa xác định");
    }else if(*distance>5){
      strcpy(target, "Chưa xác định");
    }

  }
  Serial.print("Hàng 1: ");
  Serial.println(SDistance);
  Serial.print("Hàng 2: ");
  Serial.println(SEnergy);
  Serial.print("Hàng 3: ");
  Serial.println(MDistance);
  Serial.print("Hàng 4: ");
  Serial.println(MEnergy);
}

//----------------------------------------------------------------------------------------------------

void loop(){
  radar.read();
  if (radar.isConnected() && millis() - lastReading > 1){
   lastReading = millis();
   DataMatrix(MatrixData, col);
   col++;
   }if(col==ColsSetup){
    ShowMatrixData( MatrixData, 0, 3); //Show MatrixData
    col=0;
    
    for(byte row=0; row<4; row++){
      float average = Sum(row, MatrixData)/ColsSetup;
      float median = Median(row, MatrixData);
      float SD = sqrt(Variance(row, MatrixData, average));
      float CV = SD*100/average;

      MatrixValue[row][0] = average;
      MatrixValue[row][1] = median;
      MatrixValue[row][2] = SD;
      MatrixValue[row][3] = CV;
    }


   
//  ShowMatrixValue( MatrixValue);
    Conclude(MatrixValue, &distance, room, target);
//  In dữ liệu lên Serial Monitor để theo dõi
    Serial.print("room: ");
    Serial.print(room);
    Serial.print(" | target: ");
    Serial.print(target);
    Serial.print(" | distance: ");
    Serial.println(distance);
    Serial.println();
    
//    String parameter = "";
//    parameter += "Trung bình: "; 
//    parameter += String(average, 2);
//    parameter += "\n";
//    
//    parameter += "Trung vị: ";
//    parameter += String(median, 2);
//    parameter += "\n";
//    
//    parameter += "Độ lệch chuẩn: ";
//    parameter += String(SD, 2);
//    parameter += "\n";
//    
//    parameter += "Hệ số biến thiên: ";
//    parameter += String(CV, 2);
//    parameter += "\n";
//    Serial.println(parameter);
    }
    }
