#include <Servo.h>
#include <Encoder.h>
#include "Arduino.h"
#include "Messages.h"
#include "LCD.h"
#include "LineFollower.h"
#include "RBE2001.h"


/**
 * Note: Use a jumper wire to decide which quardrant to start in.
 *       Quadrant I: Where the field computer is located.
 *       Storage: Inbetween I and II
 *       Supply: Inbetween III and IV
 *       ____________________
 *      |         |         |
 *      |    I    |    II   |
 *      |_________|_________|
 *      |         |         |
 *      |    IV   |   III   |
 *      |_________|_________|
 *      
 */


// initialize the LCD
LCD lcd;

Messages msg;
unsigned long timeForHeartbeat;

// left side
int leftFrontLineTrackerPin = 25;
int leftHorizontalLineTrackerPin = 24;
int leftBackLineTrackerPin = 23;
int leftBackMostLineTrackerPin = 22;

// right side
int rightFrontLineTrackerPin = 27;
int rightHorizontalLineTrackerPin = 5;
int rightBackLineTrackerPin = 26;
int rightBackMostLineTrackerPin = 4;

int frontLimitSwitchPin = 29;

int upperArmLimitSwitchPin = 6;  // NEW !!!
int lowerArmLimitSwitchPin = 28;  // NEW !!!

int oldDegree; //new

// NEW !!!
// Interrupts
int leftEncoderPin1 = 2; // All four encoder wires (left and right) are in interrupts
int leftEncoderPin2 = 3;
int rightEncoderPin1 = 18; // NEW !!!
int rightEncoderPin2 = 19; // NEW !!!
int wristPotentiometerPin = 20; // NEW!!!

// Field Elements
bool ReactorTubeA = false;          // AKA 
bool ReactorTubeB = false;          // AKA 

bool SpentRodStorageArea1 = false;  // AKA 
bool SpentRodStorageArea2 = false;  // AKA 
bool SpentRodStorageArea3 = false;  // AKA 
bool SpentRodStorageArea4 = false;  // AKA 

bool NewRodStorageArea1 = false;    // AKA 
bool NewRodStorageArea2 = false;    // AKA 
bool NewRodStorageArea3 = false;    // AKA 
bool NewRodStorageArea4 = false;    // AKA 


/**
   This "starter" program reads and debug-prints messages from the field. It also sends
   heartbeat messages every second letting the field know that the robot is still running.

   You can use this as the basis for your project, or copy the elements from here into your
   project. If you do that, be sure that you include the Messages and BTComms classes (.cpp
   and .h files) into your new project.
*/

Servo rightMotor;
Servo leftMotor;

// NEW !!!
Servo armMotor;
Servo wristMotor;
Servo clawMotor;


// Initialize encoder
Encoder leftWheelEncoder(leftEncoderPin1, leftEncoderPin2); // NEW !!! (changed)
Encoder rightWheelEncoder(rightEncoderPin1, rightEncoderPin2); // NEW !!!

// Keep track of current action
Action action;

/**
   Initialize the messages class and the debug serial port
*/
void setup() {
  Serial.begin(115200);
  Serial.println("Starting");
  msg.setup();
  timeForHeartbeat = millis() + 1000;
  // Initialize LCD screen with 2 rows and 16 columns
  lcd = LCD();

  // Left Side
  pinMode(leftFrontLineTrackerPin, INPUT);
  pinMode(leftHorizontalLineTrackerPin, INPUT);
  pinMode(leftBackLineTrackerPin, INPUT);
  pinMode(leftBackMostLineTrackerPin, INPUT);

  // Right side
  pinMode(rightFrontLineTrackerPin, INPUT);
  pinMode(rightHorizontalLineTrackerPin, INPUT);
  pinMode(rightBackLineTrackerPin, INPUT);
  pinMode(rightBackMostLineTrackerPin, INPUT);
  // Front
  pinMode(frontLimitSwitchPin, INPUT_PULLUP);
  // Arm
  pinMode(upperArmLimitSwitchPin, INPUT_PULLUP);
  pinMode(lowerArmLimitSwitchPin, INPUT_PULLUP);

  // Initialize motors
  leftMotor.attach(11);
  rightMotor.attach(10);
  armMotor.attach(9);
  wristMotor.attach(8);
  clawMotor.attach(7);

  

  // Update our Field Element variables with the status of the field
  fieldStatus();

//  action = followLineBackwardAction; // first action
}

/**
   The loop method runs continuously, so call the Messages read method to check for input
   on the bluetooth serial port and parse any complete messages. Then send the heartbeat if
   it's time to do that.

   For the final project, one good way of implementing it is to use a state machine with high
   level tasks as states. The state will tell what you should be doing each time the loop
   function is called.

   Refer to the state machine lecture or look at the BTComms class for an example on how to
   implement state machines.
*/
void loop() {
  messageUpdate();
  heartbeatUpdate();
//  robotMain();
  LEDs();
  testLineFollow();

  
}

void testLineFollow() {
  /**
   * Wait for button, raise arm, line follow to a storage container, line follow to a supply container,
   * line follow to the previous reactor, lower arm.
  **/
  // wait for buttom
  while (digitalRead(frontLimitSwitchPin) == HIGH) {
    Serial.println("Waiting for front limit switch to be pressed...");
  }
  
  // raise arm
  while (digitalRead(upperArmLimitSwitchPin) == HIGH) {
    Serial.println("Raise Arm");
    armMotor.write(80);
  }
  
  // stop arm
  Serial.println("Stop Arm!");
  armMotor.write(90);
  
  // line follow to first line cross (HIGH = Black line)
  while (digitalRead(leftBackMostLineTrackerPin) == LOW && digitalRead(rightBackMostLineTrackerPin) == LOW) {
    if (digitalRead(leftBackMostLineTrackerPin) == LOW && digitalRead(rightBackMostLineTrackerPin) == LOW) {
      leftMotor.write(75);
      rightMotor.write(75);
    } else if (digitalRead(leftBackMostLineTrackerPin) == HIGH && digitalRead(rightBackMostLineTrackerPin) == LOW) {
      leftMotor.write(80);
      rightMotor.write(75);
    } else if (digitalRead(leftBackMostLineTrackerPin) == LOW && digitalRead(rightBackMostLineTrackerPin) == HIGH) {
      leftMotor.write(75);
      rightMotor.write(80);
    }
  }

  // line-up robot
  leftMotor.write(90);
  rightMotor.write(90);
  delay(1000);
  while ((leftWheelEncoder.read() * -1) != (rightWheelEncoder.read() * -1)) {
    while ((leftWheelEncoder.read() * -1) > (rightWheelEncoder.read() * -1)) {
      leftMotor.write(90);
      rightMotor.write(80);
    }
  }
  // rotate robot 90 degrees

  // line follow to container

  // pause

  // robot back up

  // robot rotate 180 degrees

  // line follow to new rod

  // robot back up

  // robot turn 90 degrees

  // line follow back to reactor
  
  
} // testLineFollow()

void LEDs() {
  if (digitalRead(leftFrontLineTrackerPin) == HIGH) {
    digitalWrite(46, HIGH);
  } else {
    digitalWrite(46, LOW);
  }

  if (digitalRead(leftHorizontalLineTrackerPin) == HIGH) {
    digitalWrite(48, HIGH);
  } else {
    digitalWrite(48, LOW);
  }

  if (digitalRead(leftBackLineTrackerPin) == HIGH) {
    digitalWrite(50, HIGH);
  } else {
    digitalWrite(50, LOW);
  }

  if (digitalRead(leftBackMostLineTrackerPin) == HIGH) {
    digitalWrite(52, HIGH);
  } else {
    digitalWrite(52, LOW);
  }

  if (digitalRead(rightFrontLineTrackerPin) == HIGH) {
    digitalWrite(47, HIGH);
  } else {
    digitalWrite(47, LOW);
  }

  if (digitalRead(rightHorizontalLineTrackerPin) == HIGH) {
    digitalWrite(49, HIGH);
  } else {
    digitalWrite(49, LOW);
  }

  if (digitalRead(rightBackLineTrackerPin) == HIGH) {
    digitalWrite(51, HIGH);
  } else {
    digitalWrite(51, LOW);
  }

  if (digitalRead(rightBackMostLineTrackerPin) == HIGH) {
    digitalWrite(53, HIGH);
  } else {
    digitalWrite(53, LOW);
  }
}

//void robotMain() {
//  switch (action) {
//    case followLineBackwardAction:
//      followLine(backwardDirection);
//      if (isFollowLineFinished()) {
//        Serial.println("Backward");
//        delay(1000);
//        oldDegree = leftWheelEncoder.read(); // new
//        action = rotateNintyDegreesAction;
//      }
//      break;
//    case followLineForwardAction:
//      Serial.println("Forward");
//      followLine(forwardDirection);
//      break;
//    case rotateNintyDegreesAction:
//      Serial.println("Rotate");
//      turnNintyDegrees();
//      break;
//  }
//}

// Update our Field Element variables with the status of the field
void fieldStatus() {
  // Field Elements
  bool ReactorTubeA = false;          // AKA 
  bool ReactorTubeB = false;          // AKA 

  bool SpentRodStorageArea1 = false;  // AKA 
  bool SpentRodStorageArea2 = false;  // AKA 
  bool SpentRodStorageArea3 = false;  // AKA 
  bool SpentRodStorageArea4 = false;  // AKA 

  bool NewRodStorageArea1 = false;    // AKA 
  bool NewRodStorageArea2 = false;    // AKA 
  bool NewRodStorageArea3 = false;    // AKA 
  bool NewRodStorageArea4 = false;    // AKA 

  Serial.print("ReactorTubeA: ");
  Serial.print(ReactorTubeA);
  Serial.print(" ReactorTubeB: ");
  Serial.println(ReactorTubeB);
  
  Serial.print("SpentRodStorageArea1: ");
  Serial.print(SpentRodStorageArea1);
  Serial.print(" SpentRodStorageArea2: ");
  Serial.print(SpentRodStorageArea2);
  Serial.print(" SpentRodStorageArea3: ");
  Serial.print(SpentRodStorageArea3);
  Serial.print(" SpentRodStorageArea4: ");
  Serial.println(SpentRodStorageArea4);
  
  Serial.print("NewRodStorageArea1: ");
  Serial.print(NewRodStorageArea1);
  Serial.print(" NewRodStorageArea2: ");
  Serial.print(NewRodStorageArea2);
  Serial.print(" NewRodStorageArea3: ");
  Serial.print(NewRodStorageArea3);
  Serial.print(" NewRodStorageArea4: ");
  Serial.println(NewRodStorageArea4);
  
}


void messageUpdate() {
  if (msg.read()) {
    switch (msg.getMessageType()) {
      case Messages::kStopMovement:
        // Received "Stop" message
        lcd.print(1, "Stop");
        break;
      case Messages::kResumeMovement:
        // Received "Resume" message
        lcd.print(1, "Resume");
        break;
    }
  }
}

void heartbeatUpdate() {
  if (millis() > timeForHeartbeat) {
    timeForHeartbeat = millis() + 1000;
    msg.sendHeartbeat();
  }
}

//void followLine(Direction direction) {
//  int left = digitalRead(leftLineTrackerPin);
//  int center = digitalRead(centerLineTrackerPin);
//  int right = digitalRead(rightLineTrackerPin);
//
//  if (left == HIGH && right == LOW) {
//    //    if (direction == backwardDirection) {
//    leftMotor.write(105);
//    rightMotor.write(85);
//    //    } else {
//    //      leftMotor.write(75);
//    //      rightMotor.write(90);
//    //    }
//
//  } else if (left == LOW && right == HIGH) {
//    //     if (direction == backwardDirection) {
//    leftMotor.write(85);
//    rightMotor.write(105);
//    //     } else {
//    //      leftMotor.write(90);
//    //    rightMotor.write(75);
//    //     }
//  } if ((left == HIGH && right == HIGH) || center == HIGH) {
//    if (direction == backwardDirection) {
//      leftMotor.write(75);
//      rightMotor.write(75);
//    } else {
//      leftMotor.write(105);
//      rightMotor.write(105);
//    }
//  }
//}

void turnNintyDegrees() {
  int newDegree = leftWheelEncoder.read();  
  int differenceDegree = oldDegree - newDegree;
  if (abs(differenceDegree) < 200) { //246
    Serial.print(oldDegree);
    Serial.print(" ");
    Serial.print(leftWheelEncoder.read());
    Serial.print(" ");
    Serial.println(differenceDegree);
    leftMotor.write(80);
    rightMotor.write(110);
  } else {
    Serial.println("STOP!");
    leftMotor.write(90);
    rightMotor.write(90);
    delay(1000);
    action = followLineForwardAction;
  }
}

//bool isFollowLineFinished() {
//  return digitalRead(horizontalLineTrackerPin) == HIGH;
//}

//void raiseArm() {
//  armMotor.write
//}
//
//void lowerArm() {
//  
//}
//
//void stopArm() {
//  if (
//}


void testArm() {  // test arm motor with height limit switches (no fail safe)
  bool upperSwitch = digitalRead(upperArmLimitSwitchPin);
  bool lowerSwitch = digitalRead(lowerArmLimitSwitchPin);
  
  // set the arm down if arm is floating between limit switches
  if (lowerSwitch == HIGH && upperSwitch == HIGH) {
    while (lowerSwitch == HIGH && upperSwitch == HIGH) {
      armMotor.write(100); // lower arm slowly
      Serial.println("LOWER");
    }
  }
  
  delay(2000);
  
  // raise and lower arm once
  if (lowerSwitch == LOW) {
    armMotor.write(90); // stop motor
    Serial.println("STOP!");
    delay(2000);  // preference delay inbetween transisitions
    while (upperSwitch == HIGH) {
      armMotor.write(80); // raise arm slowly
      Serial.println("RAISE");
    }
    if (upperSwitch == LOW) {
      armMotor.write(90); 
      Serial.println("STOP!");
      delay(2000);
      while (lowerSwitch == HIGH) {
        armMotor.write(100);  // lower arm slowly
        Serial.println("LOWER");
      } //while
    } //if
  } //if
} //testArm()

