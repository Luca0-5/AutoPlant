#include <Servo.h>
#include <Adafruit_FXOS8700.h>


Servo myServo; // create a servo object
Servo smallServo;

int potPin = A4; // analog pin for potentiometer
int potValue;    // variable to store the potentiometer value



Adafruit_FXOS8700 accelmag = Adafruit_FXOS8700(0x8700A, 0x8700B);


int enA = 9;
int in1 = 8;
int in2 = 7;

float roll;
float pitch;



void setup() {
 
  Serial.begin(9600); // initialize serial communication
  myServo.attach(6);  // attach the servo to pin 9
  smallServo.attach(5);

  while (!Serial) {
    delay(1);
  }

  if (!accelmag.begin(0x1E)) {
    Serial.println("Ooops, no FXOS8700 detected ... Check your wiring!");
    while (1);
  }


  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

   
}

void loop() {


  potValue = analogRead(potPin);
  Serial.println("\n");
  Serial.println(potValue);
 
 
  if (potValue >= 30 && potValue <= 900) {
    dontAim();
  }

  if (potValue <= 30) {
    aimRight();
  }

  if (potValue >= 900) {
    aimLeft();
  }


  sensors_event_t aevent, mevent;
  accelmag.getEvent(&aevent, &mevent);

  roll = atan(aevent.acceleration.y / sqrt(pow(aevent.acceleration.x, 2) + pow(aevent.acceleration.z, 2))) * 180 /PI;
  pitch = 180 * atan2(aevent.acceleration.x, sqrt(aevent.acceleration.y*aevent.acceleration.y + aevent.acceleration.z*aevent.acceleration.z))/PI;
Serial.println("pitch: ");
Serial.print(pitch);
 
   if(roll < -30){
    moveLeft();
  }
  else if(roll > 30){
    moveRight();
  }
  else{
    dontMove();
  }


   if(pitch < -60){
    servofire();
  }

 delay(1000);

}


void aimLeft() {
  myServo.write(92);
}

void aimRight() {
  myServo.write(96);
}

void dontAim() {
 myServo.write(94);
}

void moveLeft(){
  analogWrite(enA, 255);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
}

void moveRight(){
  analogWrite(enA, 255);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
}

void dontMove(){
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
}

void servofire(){
  smallServo.write(0); // sets the servo position to 0 degrees
   // waits for a second
  smallServo.write(180); // move the servo to 180 degrees
  // waits for a second
}
