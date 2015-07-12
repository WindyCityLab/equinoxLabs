#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"
#include "TypesAndDeclarations.h"

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); // Initialize the Shield
Adafruit_DCMotor *motorFL = AFMS.getMotor(1);
Adafruit_DCMotor *motorFR = AFMS.getMotor(4);
Adafruit_DCMotor *motorBL = AFMS.getMotor(2);
Adafruit_DCMotor *motorBR = AFMS.getMotor(3);

void setMotor(Adafruit_DCMotor *motor, bool On, String motorName)
{
  Serial.print(motorName); Serial.print(" ");
  if (On) {
    Serial.println("ON");
    motor->run(FORWARD);
    motor->setSpeed(DEFAULT_MOTOR_SPEED);
  }
  else {
    Serial.println("OFF");
    motor->run(BACKWARD);
    motor->setSpeed(DEFAULT_MOTOR_SPEED);
  }
}
void setMotors(uint8_t toSpeed)
{
  switch (toSpeed) {
    case BOTH_MOTORS : {
      setMotor(motorFL,true, "FL");
      setMotor(motorFR,true, "FR");
      setMotor(motorBL,true, "BL");
      setMotor(motorBR,true, "BR");
    }
    break;
    case RIGHT_MOTOR_ONLY : {
      setMotor(motorFL,false, "FL");
      setMotor(motorFR,true, "FR");
      setMotor(motorBL,false, "BL");
      setMotor(motorBR,true, "BR");
    }
    break;
    case LEFT_MOTOR_ONLY : {
      setMotor(motorFL,true, "FL");
      setMotor(motorFR,false, "FR");
      setMotor(motorBL,true, "BL");
      setMotor(motorBR,false, "BR");
    }
    break;
  }
}

PossibleInputs currentRelationshipToLine() {
  if ((analogRead(A0) < CENTER_LEFT_SENSOR) && (analogRead(A1) < CENTER_RIGHT_SENSOR))
  {
    return ON_LINE;
  }
  if ((analogRead(A0) < CENTER_LEFT_SENSOR) && (analogRead(A1) > CENTER_RIGHT_SENSOR))
  {
    return OFF_TO_THE_RIGHT;
  }
  if ((analogRead(A0) > CENTER_LEFT_SENSOR) && (analogRead(A1) < CENTER_RIGHT_SENSOR))
  {
    return OFF_TO_THE_LEFT;
  }
  return LOST;
}

uint8_t currentState = CENTER;

void setup() {
  Serial.begin(115200);
  AFMS.begin();
  motorFL->run(FORWARD);
  motorFR->run(FORWARD);
  motorBL->run(FORWARD);
  motorBR->run(FORWARD);
}

void loop() {
  while (1) {
    setMotors(fsm[currentState].output);
    delay(fsm[currentState].timeDelay);
    currentState = fsm[currentState].nextState[currentRelationshipToLine()];

    Serial.print("Current state: ");
    Serial.print(currentState);
    Serial.print("  Input -> ");
    Serial.println(currentRelationshipToLine());
  }
}
