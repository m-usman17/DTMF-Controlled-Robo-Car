#include <HardwareSerial.h>

HardwareSerial sim800l(2);  // Use UART2 for SIM800L

// Define L298N control pins for Motor A (Left)
const int ENA = 26;  // Enable pin for Motor A
const int IN1 = 25;  // Control pin 1 for Motor A
const int IN2 = 23;  // Control pin 2 for Motor A

// Define L298N control pins for Motor B (Right)
const int ENB = 33;  // Enable pin for Motor B
const int IN3 = 18;  // Control pin 1 for Motor B
const int IN4 = 15;  // Control pin 2 for Motor B

bool callInProgress = false;

void setup() {
  Serial.begin(115200);
  sim800l.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17

  // Set motor control pins as outputs
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Initialize motor control pins
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 200);  // Set initial speed for Motor A (0-255)
  analogWrite(ENB, 200);  // Set initial speed for Motor B (0-255)

  Serial.println("ESP32 GSM DTMF Dual Motor Control Initialized");
  
  // Initialize SIM800L
  initializeSIM800L();
}

void loop() {
  if (callInProgress) {
    processCall();
  } else {
    checkIncomingCall();
  }
}

void initializeSIM800L() {
  sendATCommand("AT", "OK");
  sendATCommand("AT+CLIP=1", "OK");  // Enable Caller ID
  sendATCommand("AT+DDET=1", "OK");  // Enable DTMF detection
}

void processCall() {
  while (sim800l.available()) {
    String response = sim800l.readStringUntil('\n');
    Serial.println("SIM800L: " + response);

    // Check for DTMF tones
    if (response.indexOf("+DTMF:") != -1) {
      char dtmfChar = response.charAt(response.indexOf(':') + 2);
      Serial.println("DTMF Detected: " + String(dtmfChar));
      controlMotors(dtmfChar);
    }

    // Check for call end signal
    if (response.indexOf("NO CARRIER") != -1) {
      callInProgress = false;
      stopMotors();
      Serial.println("Call ended.");
    }
  }
}

void checkIncomingCall() {
  if (sim800l.available()) {
    String response = sim800l.readStringUntil('\n');
    Serial.println("SIM800L: " + response);
    
    if (response.indexOf("RING") != -1) {
      Serial.println("Incoming call detected.");
      answerCall();
      callInProgress = true;
    }
  }
}

void answerCall() {
  sendATCommand("ATA", "OK");
}

void controlMotors(char command) {
  switch (command) {
    case '2': // Move forward
      moveForward();
      break;
    case '8': // Move backward
      moveBackward();
      break;
    case '4': // Turn left
      turnLeft();
      break;
    case '6': // Turn right
      turnRight();
      break;
    case '5': // Stop
      stopMotors();
      break;
    default:
      Serial.println("Invalid command received.");
      stopMotors();
      break;
  }
}

void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  Serial.println("Moving forward");
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  Serial.println("Moving backward");
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  Serial.println("Turning left");
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  Serial.println("Turning right");
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  Serial.println("Motors stopped");
}

void sendATCommand(const char* command, const char* expectedResponse) {
  Serial.print("Sending: ");
  Serial.println(command);
  
  sim800l.println(command);

  unsigned long startTime = millis();
  bool responseReceived = false;

  while (millis() - startTime < 5000) {  // Timeout after 5 seconds
    if (sim800l.available()) {
      String response = sim800l.readStringUntil('\n');
      Serial.println("Response: " + response);
      if (response.indexOf(expectedResponse) != -1) {
        responseReceived = true;
        break;  // Expected response received
      }
    }
  }
  
  if (!responseReceived) {
    Serial.println("Command timed out.");
  }
}
