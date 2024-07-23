#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "Infinix S4";
const char* password = "9876543210";

WiFiUDP udp;
unsigned int localUdpPort = 12345;
char incomingPacket[255];

Servo shoulderServo;
Servo elbowServo;
Servo gripperServo;
Servo gripperUpDownServo; // New servo for gripper up and down

#define SHOULDER_PIN 13
#define ELBOW_PIN 12
#define GRIPPER_PIN 32
#define GRIPPER_UP_DOWN_PIN 27 // Pin for gripper up and down servo

#define SHOULDER_MIN 0
#define SHOULDER_MAX 180
#define ELBOW_MIN 0
#define ELBOW_MAX 180

#define GRIPPER_OPEN 180
#define GRIPPER_CLOSE 80

#define GRIPPER_UP 100 // Position for gripper up
#define GRIPPER_DOWN 150 // Position for gripper down

bool actionCompleted = false;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  
  udp.begin(localUdpPort);
  
  shoulderServo.attach(SHOULDER_PIN);
  elbowServo.attach(ELBOW_PIN);
  gripperServo.attach(GRIPPER_PIN);
  gripperUpDownServo.attach(GRIPPER_UP_DOWN_PIN); // Attach the new servo
  
  // Initialize position
  initializeArm();
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }
    
    char* class_name = strtok(incomingPacket, ",");
    char* x_str = strtok(NULL, ",");
    char* y_str = strtok(NULL, ",");
    char* confidence_str = strtok(NULL, ","); // Parse the confidence value
    
    if (class_name && x_str && y_str && confidence_str) {
      Serial.print("Class: ");
      Serial.println(class_name);
      Serial.print("x: ");
      Serial.println(x_str);
      Serial.print("y: ");
      Serial.println(y_str);
      Serial.print("Confidence: ");
      Serial.println(confidence_str); // Print the confidence value
      
      if (strcmp(class_name, "Damaged") == 0 && !actionCompleted) {
        pickAndDropTomato();
        actionCompleted = true; // Mark the action as completed
      }
    }
  } else {
    actionCompleted = false; // Reset the actionCompleted flag if no packet is received
  }
}

void moveServo(Servo &servo, int start, int end, int stepDelay = 10) {
  if (start < end) {
    for (int pos = start; pos <= end; pos++) {
      servo.write(pos);
      delay(stepDelay);
    }
  } else {
    for (int pos = start; pos >= end; pos--) {
      servo.write(pos);
      delay(stepDelay);
    }
  }
}

void pickAndDropTomato() {
  // Move gripper up
  moveServo(gripperUpDownServo, GRIPPER_DOWN, GRIPPER_UP);
  delay(1000);

  // Move elbow to 100
  moveServo(elbowServo, elbowServo.read(), 120);
  delay(1000);

  // Move shoulder to 130
  moveServo(shoulderServo, shoulderServo.read(), 130);
  delay(1000);
  
  // Close gripper
  moveServo(gripperServo, gripperServo.read(), GRIPPER_CLOSE);
  delay(1000);

  // Move shoulder back to 100
  moveServo(shoulderServo, shoulderServo.read(), 90);
  delay(1000);

  // Move elbow back to 80
  moveServo(elbowServo, elbowServo.read(), 80);
  delay(1000);

  // Open gripper
  moveServo(gripperServo, gripperServo.read(), GRIPPER_OPEN);
  delay(1000);

  // Move gripper down
  moveServo(gripperUpDownServo, gripperUpDownServo.read(), GRIPPER_DOWN);
  delay(1000);
  
  // Return to initial positions
  initializeArm();
}

void initializeArm() {
  gripperServo.write(GRIPPER_OPEN);
  gripperUpDownServo.write(GRIPPER_DOWN);
  shoulderServo.write(100);
  elbowServo.write(80);
}
