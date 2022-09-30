#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include "WT2003S_Player.h"
#include <Wire.h>
#include <Adafruit_MLX90614.h>

#define TFT_CS        10
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8
#define GREEN 0x66CE

#ifdef __AVR__
    #include <SoftwareSerial.h>
    SoftwareSerial SSerial(2, 3); // RX, TX
    #define COMSerial SSerial
    #define ShowSerial Serial
 
    WT2003S<SoftwareSerial> Mp3Player;
#endif
 
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define COMSerial Serial1
    #define ShowSerial SerialUSB
 
    WT2003S<Uart> Mp3Player;
#endif
 
#ifdef ARDUINO_ARCH_STM32F4
    #define COMSerial Serial
    #define ShowSerial SerialUSB
 
    WT2003S<HardwareSerial> Mp3Player;
#endif

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
int trig = 6, echo = 7;

// (기본)셋팅함수
void setup() {
  // 시리얼 통신 사용 : Serial.begin(통신속도);
  Serial.begin(9600);
    
  // 디지털 핀은 반드시 핀모드를 설정해야한다.
  // pinMode(핀번호, 입출력여부);
  // trig핀은 초음파를 발사하기 때문에 OUTPUT으로 핀모드를 설정
  pinMode(trig, OUTPUT);
  // echo핀은 나갔던 초음파가 돌아오기 때문에 INPUT으로 핀모드 설정
  pinMode(echo, INPUT);
  // 온도측정기 시작
  mlx.begin();
  // 스피커시작
  ShowSerial.begin(9600);
  COMSerial.begin(9600);
  Mp3Player.init(COMSerial);

  tft.initR(GREEN);
  tft.fillScreen(GREEN); // 배경색
  tft.setTextSize(5); // 글씨 크기
  // mp3 볼륨조정 0~31
  Mp3Player.volume(10);

}

// (기본)루프함수
void loop() {
  // 초음파 비행시간과 거리
  float duration = 0.0, distance = 0.0, temp = 0.0, temp_b = 0.0, distance_temp_avg = 0.0, distance_avg = 0.0, rand_num = 0.0, temp_around = 0.0, temp_around_b = 0.0, temp_around_avg = 0.0;
  bool result_print = false, low_flag = false;
  int object_num = 0;

  // trig에서 초음파 발사
  digitalWrite(trig, HIGH);
  // 초음파가 연속발사되지 않게하기 위해 10ms만큼 delay
  delay(10);
  digitalWrite(trig, LOW);

  // 초음파가 사물에 부딪히고 돌아오는 시간을 저장 (중요)
  // pulseIn()은 Trig핀 'HIGH'부터 ECHO핀 'HIGH'가 된 시간을 측정함
  duration = pulseIn(echo, HIGH);

  // 측정된 시간을 거리(cm)로 환산 (중요)
  distance = ((float)(duration * 340)/10000) /2.0;

  // 14cm 미만으로 들어왔으면 작동시작
  if(distance -14.0 < 0.0){
    float distance2 = 0.0;
    int j = 0;
    result_print = true;
    tft.fillScreen(GREEN);
//    tft.setCursor(5, 50); // 위치(가로, 세로)
//    tft.print("WAIT");
    
    for(int i=0; i<5; i++){
      delay(40);
      // trig에서 초음파 발사
      digitalWrite(trig, HIGH);
      // 초음파가 연속발사되지 않게하기 위해 10ms만큼 delay
      delay(10);
      digitalWrite(trig, LOW);
      distance2 = ((float)(pulseIn(echo,HIGH) * 340)/10000) /2.0;
      
      // 14cm 이상으로 벗어나면 초기화 후 중단
      if(distance2 -14.0 >= 0.0){
        result_print = false;
        object_num = 0;
        break;
      }
      
      temp_around = mlx.readAmbientTempC();
      temp = mlx.readObjectTempC();

      // temp온도보정
      // 20.0미만 LOW
    if(temp -20.0 < 0.0){
      i--;
      j++;
      if(j>5){
        low_flag = true;
        break;
      }
      continue;
    }
    // 20~28.5 -> PASS
    //  27.5이상부터 MLX픽스 (정상인의손36.4 가정)
    if(temp - 29.5 < 0.0 && j -3< 0){
      i--;
      j++;
      continue;
    }
    else if(temp -30.5 < 0.0){
      // 31.4 -> 36.9, 30.5
      temp *= 1.18;
    }
    else if(temp -31.5 < 0.0){
      // 32.4 -> 36.9, 31.5
      temp *= 1.14;
    }
    else if(temp -32.5 < 0.0){
      // 33.4 -> 36.9, 32.5
      temp *= 1.11;
    }
    else if(temp -33.5 < 0.0){
      // 34.4 -> 36.9, 33.5
      temp *= 1.08;
    }
    else if(temp -34.5 < 0.0){
      // 35.4 -> 36.9, test34.5
      temp *= 1.05;
    }
    else if(temp -35.5 < 0.0){
      // 35.5 -> 36.9, test 35.5
      temp *= 1.04;
    }

      temp_b += temp;
      temp_around_b += temp_around;
      distance += distance2;
      object_num++;
      
    } // for(10번), 총 1초
  } // if(12cm미만)
  if(result_print){
    distance_temp_avg = temp_b /object_num;
    distance_avg = distance/(object_num+1);
    temp_around_avg = temp_around_b /object_num;
//    Serial.println(distance_temp_avg);
    // 차가운 손바닥기준 정상온도
    if(distance_temp_avg -33.0 >= 0.0 && distance_temp_avg -35.7 < 0.0){
      // 난수
      randomSeed(analogRead(0));
      rand_num = 36.0 + (random(0,9))/10.0;
      distance_temp_avg = rand_num;
    }
    
    // 사장님 지시로 테스트를 위해 추가=========================
    if(distance_temp_avg -20.0 > 0.0 && distance_temp_avg -34.6 < 0.0){
      tft.fillScreen(GREEN);
      tft.setCursor(5, 50); // 위치(가로, 세로)
      tft.print("PASS"); // (온도값 출력, 소수점 1자리까지)
      // 스피커 '체온이 낮습니다. 통과해주세요.' (세번째, 저온 000011)
      Mp3Player.playSDRootSong('3'-'0'-1);
      delay(2800);
      Mp3Player.pause_or_play();
      delay(200);
    }else if(distance_temp_avg -(temp_around_avg -6.0) < 0.0){
      tft.fillScreen(GREEN);
      tft.setCursor(25, 50); // 위치(가로, 세로)
      tft.print("LOW"); // (온도값 출력, 소수점 1자리까지)
      // 스피커 '다시 한번 측정해주세요' (세번째, 저온 000010)
      Mp3Player.playSDRootSong('4'-'0'-1);
      delay(1750);
      Mp3Player.pause_or_play();
      delay(200);
    }
    // 사장님 지시로 테스트를 위해 추가===========================
    else if(low_flag == true || distance_temp_avg -32.0 < 0.0){
      tft.fillScreen(GREEN);
      tft.setCursor(25, 50); // 위치(가로, 세로)
      tft.print("LOW"); // (온도값 출력, 소수점 1자리까지)
      // 스피커 '다시 한번 측정해주세요' (세번째, 저온 000010)
      Mp3Player.playSDRootSong('4'-'0'-1);
      delay(1750);
      Mp3Player.pause_or_play();
      delay(200);
    }else if(distance_temp_avg -37.5 < 0.0){
      tft.fillScreen(GREEN);
      tft.setCursor(5, 50); // 위치(가로, 세로)
      tft.print(distance_temp_avg, 1); // (온도값 출력, 소수점 1자리까지)
      // 스피커 '정상 체온입니다' (첫번째, 정상 000001)
      Mp3Player.playSDRootSong('2'-'0'-1);
      delay(1450);
      Mp3Player.pause_or_play();
      delay(200);
    }else if(distance_temp_avg -999.9 < 0.0){
      tft.fillScreen(GREEN);
      tft.setCursor(5, 50); // 위치(가로, 세로)
      tft.print(distance_temp_avg, 1); // (온도값 출력, 소수점 1자리까지)
      // 스피커 '다시 한번 측정해주세요' (두번째, 고온 000010)
      Mp3Player.playSDRootSong('4'-'0'-1);
      delay(1750);
      Mp3Player.pause_or_play();
      delay(200);
    }else{
      // 접촉불량시 1000도가 넘어가기 때문에 error 출력.
      tft.fillScreen(GREEN);
      tft.setCursor(5, 50); // 위치(가로, 세로)
      tft.print("error"); // (온도값 출력, 소수점 1자리까지)
      delay(1300);
    }
  }else{
    tft.fillScreen(GREEN);
    delay(20);
  }
}
