#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>
#include <Wire.h>

//touch sensor
int prevt = 0;
int t = 0;
int threshold = 3;
int cstay = 0;

//mp3
SoftwareSerial mySerial(2, 3);
int flag = 0;
int randnum;

//onoff 
#define LED_PIN 5
#define SW_PIN 6 
int swVal = 0;
int swCount = 0;
boolean swON = false;

//mode_change
#define MODE_PIN 11
int mode = 0;
int modeVal = 0;
int modeCount = 0;

//9jiku
#define Addr_Accl 0x19
float xAccl = 0.00;
float yAccl = 0.00;
float yAcclold = 0.00;
float zAccl = 0.00;

void setup(){
  Serial.begin(9600);
  
  //touch
  pinMode(8,OUTPUT);
  pinMode(9,INPUT);

  //mp3
  mySerial.begin(9600);
  mp3_set_serial(mySerial);
  mp3_set_volume(30);

  //9jiku
  Wire.begin();
  BMX055_Init();
  delay(300);  

  //onoff
  pinMode(LED_PIN, OUTPUT);
  pinMode(SW_PIN, INPUT);

  //mode_change
  pinMode(MODE_PIN, INPUT);

  randnum = (int)random(0, 10);
}


void loop(){
  //onoff
  swVal = digitalRead(SW_PIN);
  modeVal = digitalRead(MODE_PIN);

  if(modeVal == HIGH){
    modeCount++;
  }else{
    modeCount = 0;
  }

  if(modeCount == 10){
    mode = (mode+1)%2; //0:ダンベル 1:腕立て
    mp3_play(100+mode); // "だんべる！"or"うでたて！"
    flag = 0;
  }

  if(swVal == HIGH){
    swCount++;
  }else{
    swCount = 0;
  }
 
  if(swCount == 10){
    swON = !swON;
    if(swON){
     mp3_play(0);//はじめ！
    }else{   
     mp3_play(255);//おしまい！
     flag = 0;
    }
  }

  if(mode==0 && swON){
    BMX055_Accl();
    if(yAccl > 8 && yAcclold < 8){
      //mp3
      flag++;
      if(flag%10 != 0){
         mp3_play(flag%10); //1,2,3...
      }else{
         mp3_play(flag/10*10); //10, 20, 30...
         randnum = (int)random(0, 10);
      }
      delay(1000);
      if(flag%10 == randnum){
        int r = (int)random(0, 3);
        mp3_play(252+r); //がんばれ！
      }
    }
  }

  if(mode==1 && swON){
    t = 0;
    // パルスの立ち上げ
    digitalWrite(8, HIGH);
    // 立ち上がりまでの時間計測
    
    while (digitalRead(9)!=HIGH) {
      t++;
    }

    // 放電するまで待つ
    digitalWrite(8, LOW);  
    delay(1);
    // ローパスフィルタ
    //t = 0.8 * prevt + 0.2 * t;
    //prevt = t;
    if(t > threshold){
      cstay++;
      Serial.print(t);
    }
    
    if(cstay > 3){
      cstay = 0;
      flag++;
      //mp3
      if(flag%10 != 0){
         mp3_play(flag%10);
      }else{
         mp3_play(flag/10*10);
         randnum = (int)random(0, 10);
      }
      delay(1000);
      if(flag%10 == randnum){
        int r = (int)random(0, 3);
        Serial.println(r);
        mp3_play(252+r); //がんばれ！
      }
    }
  }
  delay(1);
}

//加速度センサ用
void BMX055_Init()
{
  Wire.beginTransmission(Addr_Accl);
  Wire.write(0x0F); // Select PMU_Range register
  Wire.write(0x03);   // Range = +/- 2g
  Wire.endTransmission();
  delay(100);

  Wire.beginTransmission(Addr_Accl);
  Wire.write(0x10);  // Select PMU_BW register
  Wire.write(0x08);  // Bandwidth = 7.81 Hz
  Wire.endTransmission();
  delay(100);
}

void BMX055_Accl()
{
  int data[6];
  for (int i = 0; i < 6; i++)
  {
    Wire.beginTransmission(Addr_Accl);
    Wire.write((2 + i));// Select data register
    Wire.endTransmission();
    Wire.requestFrom(Addr_Accl, 1);// Request 1 byte of data
    // Read 6 bytes of data
    // xAccl lsb, xAccl msb, yAccl lsb, yAccl msb, zAccl lsb, zAccl msb
    if (Wire.available() == 1)
      data[i] = Wire.read();
  }
  // Convert the data to 12-bits
  xAccl = ((data[1] * 256) + (data[0] & 0xF0)) / 16;
  if (xAccl > 2047)  xAccl -= 4096;
  yAcclold = yAccl;
  yAccl = ((data[3] * 256) + (data[2] & 0xF0)) / 16;
  if (yAccl > 2047)  yAccl -= 4096;
  zAccl = ((data[5] * 256) + (data[4] & 0xF0)) / 16;
  if (zAccl > 2047)  zAccl -= 4096;
  xAccl = xAccl * 0.0098; // range +-2g
  yAccl = yAccl * 0.0098; // range +-2g
  zAccl = zAccl * 0.0098; // range +-2g
}
