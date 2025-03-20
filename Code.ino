#include <Servo.h>

// Motor driver L298N pins
#define enA 6
#define in1 3
#define in2 7
#define in3 2
#define in4 4
#define enB 5

// Ultrasonic sensor pins
#define frontTrig 12
#define frontEcho 11
#define leftTrig 13
#define leftEcho 10
#define rightTrig 8
#define rightEcho 9

// Fire sensor pins
#define fireRight A0
#define fireFront A1
#define fireLeft A2

// Other components
#define waterPump A5
#define servoPin A4
#define buzzer A3
int threshold = 100;
Servo nozzle;  // Servo motor for water spray

// Function to measure distance using ultrasonic sensor
long getDistance(int trig, int echo) {
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
    long duration = pulseIn(echo, HIGH);
    return duration * 0.034 / 2;  // Convert to cm
}

// Function to move forward
void moveForward() {
    analogWrite(enA, 85); // Motor speed
    analogWrite(enB, 85);
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
}

// Function to move backward
void moveBackward() {
    analogWrite(enA, 80);
    analogWrite(enB, 80);
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
}

// Function to turn right
void turnRight() {
    analogWrite(enA, 80);
    analogWrite(enB, 80);
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
}

// Function to turn left
void turnLeft() {
    analogWrite(enA, 85);
    analogWrite(enB, 85);
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
}

// Function to stop movement
void stopMoving() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
}

// Function to extinguish fire
void extinguishFire() {
    stopMoving();
    digitalWrite(waterPump, HIGH);
    while(1){
    int pos;
    Serial.print("Fire Right: ");
    Serial.print(analogRead(fireRight));
    Serial.print("  Fire Front: ");
    Serial.print(analogRead(fireFront));
    Serial.print("  Fire Left: ");
    Serial.println(analogRead(fireLeft));
    for (pos = 0; pos <= 70; pos += 1) { // goes from 0 degrees to 150 degrees
    // in steps of 1 degree
    nozzle.write(pos);              // tell servo to go to position in variable 'pos'
    delay(10);                       // waits 10 ms for the servo to reach the position
  }
  for (pos = 70; pos >= 0; pos -= 1) { // goes from 150 degrees to 0 degrees
    nozzle.write(pos);              // tell servo to go to position in variable 'pos'
    delay(10);                       // waits 10 ms for the servo to reach the position
  }
  int fireR = analogRead(fireRight);
  int fireF = analogRead(fireFront);
  int fireL = analogRead(fireLeft);
  Serial.print(fireF < 200);
  Serial.print(fireL < 200);
  Serial.print(fireR < 200);
  if (fireF < 200 || fireR < 200 || fireL < 200) {continue;} else break;
  }
}

void setup() {
    // Motor driver setup
    pinMode(enA, OUTPUT);\


    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
    pinMode(enB, OUTPUT);

    // Ultrasonic sensor setup
    pinMode(frontTrig, OUTPUT);
    pinMode(frontEcho, INPUT);
    pinMode(leftTrig, OUTPUT);
    pinMode(leftEcho, INPUT);
    pinMode(rightTrig, OUTPUT);
    pinMode(rightEcho, INPUT);

    // Fire sensor setup
    pinMode(fireRight, INPUT);
    pinMode(fireFront, INPUT);
    pinMode(fireLeft, INPUT);

    // Other components setup
    pinMode(buzzer, OUTPUT);
    pinMode(waterPump, OUTPUT);
    nozzle.attach(servoPin);
    nozzle.write(35); // Default position
    digitalWrite(waterPump, LOW);
    digitalWrite(buzzer, LOW);
    Serial.begin(9600);
}

void loop() {
    long frontDist = getDistance(frontTrig, frontEcho);
    long leftDist = getDistance(leftTrig, leftEcho);
    long rightDist = getDistance(rightTrig, rightEcho);

    int fireR = analogRead(fireRight);
    int fireF = analogRead(fireFront);
    int fireL = analogRead(fireLeft);

    // Check for fire
    if (fireF < 400 || fireR < 400 || fireL < 400) {
        Serial.println("Fire Detected!");
        moveForward();
        if (fireF < 40 || fireR < 40 || fireL < 40) {
            digitalWrite(buzzer, HIGH); // Buzzer ON
            delay(500);
            digitalWrite(buzzer, LOW); // Buzzer OFF
            moveBackward();
            delay(500);
            extinguishFire();
            nozzle.write(35);
            digitalWrite(waterPump, LOW);
            return; // Resume navigation after extinguishing
        }
    } else {
        // Obstacle Avoidance Logic
        if (frontDist < 20) {  // If an object is too close in front
            stopMoving();
            moveBackward();  // Move back briefly
            delay(200);  // Reduced delay (was 500ms)
            stopMoving();

            if (leftDist > rightDist) {
                Serial.println("Turning Left");
                turnLeft();
                delay(800);
            } else {
                Serial.println("Turning Right");
                turnRight();
                delay(800);
            }
        } else if (leftDist < 20) {  // Avoid left obstacle
            Serial.println("Obstacle on Left, Turning Right");
            turnRight();
            delay(800);
        } else if (rightDist < 20) {  // Avoid right obstacle
            Serial.println("Obstacle on Right, Turning Left");
            turnLeft();
            delay(800);
        } else {
            Serial.println("Moving Forward");
            moveForward();
        }
    }

    delay(100);
}
