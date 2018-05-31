#include <dht.h>
#include <LiquidCrystal.h>
#include <Servo.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

Servo myservo1;
Servo myservo2;

int sensorPin = A0;    // select the input pin for the potentiometer
int ledPin = 9;      // select the pin for the LED
int sensorValue = 0;

dht DHT;

#define DHT11_PIN 7

void setup(){   
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(ledPin, OUTPUT);
  myservo1.attach(A1);
  myservo2.attach(A2);
  myservo1.write(140); //op
  myservo2.write(102); //op
  
  delay(1000);
  
  myservo1.detach();
  myservo2.detach();

  //myservo1.write(106); //Ned
  //myservo2.write(132); //Ned
}

void loop()
{
  sensorValue = 0-((analogRead(sensorPin)/2)-140)+255;
  if(sensorValue<30){
        digitalWrite(ledPin, LOW);
  }
  else if(sensorValue>255){
        digitalWrite(ledPin, HIGH);
  }
  else{
    analogWrite(ledPin, sensorValue); 
  }
  
  delay(10);
  int chk = DHT.read11(DHT11_PIN);
  lcd.setCursor(0,0); 
  lcd.print("Temp: ");
  lcd.print(DHT.temperature);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("Humidity: ");
  lcd.print(DHT.humidity);
  lcd.print("%");
  Serial.print(":");
  Serial.print((int)DHT.temperature);
  Serial.print(":");
  Serial.print((int)DHT.humidity);
  delay(4000);
}

