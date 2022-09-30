// LED 전역변수
int red = 11;
int yellow = 12;
int green = 13;

// 초음파센서 전역변수
int trig = 2;
int echo = 3;

void setup() {
  // 초음파센서 셋팅
  Serial.begin(9600);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  // LED 셋팅
  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);
}

void loop() {
  // 초음파센서 동작
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  int distance = pulseIn(echo, HIGH) * 340 / 2/ 10000;
  Serial.print(distance);
  Serial.println("cm");
  delay(100);

  // LED 동작
  if(distance < 10){
    digitalWrite(red, HIGH);
    digitalWrite(yellow, LOW);
    digitalWrite(green, LOW);
    delay(1000);
  }
  else if(distance < 30){
    digitalWrite(red, HIGH);
    digitalWrite(yellow, LOW);
    digitalWrite(green, LOW);
    delay(1000);
  }
  else if(distance < 100){
    digitalWrite(red, HIGH);
    digitalWrite(yellow, LOW);
    digitalWrite(green, LOW);
    delay(1000);
  }
  else{
    digitalWrite(red, LOW);
    digitalWrite(yellow, LOW);
    digitalWrite(green, LOW);
  }  
}
