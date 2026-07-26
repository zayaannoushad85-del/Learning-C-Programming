#include <IRremote.h>

#define IN1 5
#define IN2 4
#define IN3 3
#define IN4 2
#define ENA1 10
#define ENA2 11
int Sinput = 9;
const int Buz = 7;
#define GND1 6
#define trig 12
#define echo 13
#define VCC3 8
#define GND3 A3 
#define GND2 A1
#define VCC2 A0
#define VCC1 A2


char data;
float duration;
float distance;
  
IRrecv IR (A4);
decode_results result;
          
void Right(){
digitalWrite(IN2,HIGH);
digitalWrite(IN3,HIGH);
analogWrite(ENA1,125);
analogWrite(ENA2, 225);
}
void Left(){
digitalWrite(IN1,HIGH);
digitalWrite(IN4,HIGH);
analogWrite(ENA1,125);
analogWrite(ENA2,225);

}

	
void For(){
  digitalWrite(IN2,HIGH);
digitalWrite(IN4,HIGH);
analogWrite(ENA1,255);
analogWrite(ENA2,255);


}
void Back() {
  // put your main code here, to run repeatedly


digitalWrite(IN1,HIGH);
digitalWrite(IN3,HIGH);
analogWrite(ENA1,255);
analogWrite(ENA2,255);


}
void Ri(){
digitalWrite(IN2,HIGH);
digitalWrite(IN3,HIGH);
analogWrite(ENA1,255);
analogWrite(ENA2,255);

}
void Le(){
digitalWrite(IN1,HIGH);
digitalWrite(IN4,HIGH);
analogWrite(ENA1,255);
analogWrite(ENA2,255);

}
void Stop(){
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,LOW);
}
void Backward() {
  // put your main code here, to run repeatedly


digitalWrite(IN1,HIGH);
digitalWrite(IN3,HIGH);
analogWrite(ENA1,123);
analogWrite(ENA2,228);

}
void Buzz() {
   
 
   tone(Buz, 1000);
   delay(1000);
   noTone(Buz);
    delay(500);
    
}
void Forward() {
  // put your main code here, to run repeatedly

digitalWrite(IN2,HIGH);
digitalWrite(IN4,HIGH);
analogWrite(ENA1,123);
analogWrite(ENA2,228);

}
void getdistance() {
  digitalWrite(trig, HIGH);
  delay(10);
  digitalWrite(trig, LOW);
  duration=pulseIn(echo, HIGH);
  distance=(duration/2) * 0.034;

}
void IRdata() {
 
}
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);

  pinMode(VCC3, OUTPUT);
    pinMode(VCC1, OUTPUT);
  digitalWrite(VCC3, HIGH);
  digitalWrite(VCC2, HIGH);
digitalWrite(VCC1, HIGH);
pinMode(GND1,OUTPUT);
digitalWrite(GND1, LOW);
digitalWrite(GND2, LOW);
pinMode(Sinput, INPUT);
pinMode(GND3, OUTPUT);
pinMode(GND2, OUTPUT);
pinMode(trig, OUTPUT);
pinMode(echo, INPUT);
pinMode(Buz, OUTPUT);
pinMode(VCC2, OUTPUT);
digitalWrite(GND3, LOW);
Serial.println(digitalRead(Sinput));
pinMode(ENA1, OUTPUT);
pinMode(ENA2, OUTPUT);
IR.enableIRIn();

}

void loop() {
  getdistance();



  // put your main code here, to run repeatedly:
if(Serial.available() > 0 && digitalRead(Sinput) == 0){ 
    data = Serial.read(); 
    Stop();
     //initialize with motors stoped
    //Change pin mode only if new command is different from previous.   
    Serial.println(data);
    switch(data){
    case 'f':  
      Forward();
      Serial.println("Forward");
      break;
    case 'b':  
       Backward();
        Serial.println("Backward");
      break;
    case 'S':
    Stop();
    break;
    case 'r':
    Ri();
    break;
    case 'l':
    Le();
    break;
    case 'F':  
      For();
      Serial.println("Forward");
      break;
    case 'B':  
       Back();
        Serial.println("Backward");
      break;
    case 'R':
    Ri();
    break;
    case 'L':
    Le();
    break;
  
    }
  }else if(digitalRead(Sinput) == 1){
    
    delay(1500);
   if(digitalRead(Sinput) == 1){
     Stop();
     Buzz();
   }
}
 if(distance < 16){
  Stop();
  Serial.println(distance);
  delay(100);
  switch(data){
    case 'F':  
      For();
      Serial.println("Forward");
      break;
    case 'b':  
       Backward();
        Serial.println("Backward");
      break;
    case 'S':
    Stop();
    break;
    case 'r':
    Ri();
    break;
    case 'l':
    Le();
  }delay(50);
  
}

      
}
