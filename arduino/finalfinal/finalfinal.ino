#include <Servo.h>
#include <Adafruit_FXOS8700.h>


Servo myServo; // create a servo object
Servo smallServo;

int potPin = A4; // analog pin for potentiometer
int potValue;    // variable to store the potentiometer value

int pos = 0;


Adafruit_FXOS8700 accelmag = Adafruit_FXOS8700(0x8700A, 0x8700B);

  
int enA = 9;
int in1 = 8;
int in2 = 7;

float roll;
float pitch;



void setup() {
 
  Serial.begin(9600); // initialize serial communication
  myServo.attach(6);  // attach the servo to pin 6
  smallServo.attach(4);

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

  /* smallServo.write(0);
   delay(200);
   smallServo.write(180);
    */
  potValue = analogRead(potPin);
  Serial.print("Pot value: ");
  Serial.print("\t");
  Serial.print(potValue);
  Serial.print("\n");
 
 
  if (potValue >= 640 && potValue <= 725) {
    dontAim();
  }

  if (potValue <= 640) {
    aimRight();
  }

  if (potValue >= 725) {
    aimLeft();
  }


  sensors_event_t aevent, mevent;
  accelmag.getEvent(&aevent, &mevent);

  roll = atan(aevent.acceleration.y / sqrt(pow(aevent.acceleration.x, 2) + pow(aevent.acceleration.z, 2))) * 180 /PI;
  pitch = 180 * atan2(aevent.acceleration.x, sqrt(aevent.acceleration.y*aevent.acceleration.y + aevent.acceleration.z*aevent.acceleration.z))/PI;
Serial.println("pitch: ");
Serial.print("\t");
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

 delay(400);

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
  smallServo.write(155); // sets the servo position to 155 degrees
  delay(500);  // waits for half a second
  smallServo.write(0); // move the servo to 0 degrees
}
