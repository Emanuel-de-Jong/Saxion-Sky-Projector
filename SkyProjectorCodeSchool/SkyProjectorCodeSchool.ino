#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwmDriver = Adafruit_PWMServoDriver();

// LEDS
// PIN 0-2 = no label
// PIN 3-5 = label three
// PIN 6-8 = label six

// MOTORS
// PIN 9 = label three
// PIN 10 = label six
// PIN 11 = no label

const int SWITCH_PIN = 12;
const int LED_POT_PIN = A1;
const int MOTOR_POT_PIN = A0;

const int MAX_PWM = 4095;
const int BASE_MAX_ALLOWED_PWM = 200;

int maxAllowedPwm = BASE_MAX_ALLOWED_PWM;

const unsigned long TIME_BEFORE_SWITCH_REGISTER = 200;

bool isOn = false;
unsigned long switchTime = 0;

const int LED_MIN_PWM = 200;
const int LED_BASE_STEPS = 10;
const float LED_SPEED_MIN = 0.7;
const float LED_SPEED_MAX = 1.3;
const int LED_SPEED_CHANGE_TIME_MIN = 1000;
const int LED_SPEED_CHANGE_TIME_MAX = 5000;
const float LED_FADE_DEPTH_SPEEDS[] = {1.5, 1.0, 0.5};

// 0=All on, 1=Fade, 2=Same color toggle, 3=Random color toggle
int ledMode = 0;
float ledSpeedMod = 1;

int ledPwms[3][3];
int ledDirections[3][3];
float ledSpeeds[3][3];
int ledSpeedTimesBeforeChange[3][3];
int ledSpeedChangeTimes[3][3];

// 0=Red, 1=Green, 2=Blue
int currentColor = 2;
int lastColorChange = millis();

const int MOTOR_MIN_PWM = 800;
const float MOTOR_DEPTH_SPEEDS[] = {1.0, 0.6, 0.8};

// 0=Off, 1=Default, 2=Depth
int motorMode = 0;
int motorSpeedMod = 1;

void setup() {
  randomSeed(analogRead(A5));
  Serial.begin(9600);

  pinMode(SWITCH_PIN, INPUT_PULLUP);

  pwmDriver.begin();
  pwmDriver.setPWMFreq(500);

  // Led fade init
  for (int led = 0; led < 3; led++) {
    for (int color = 0; color < 3; color++) {
      ledPwms[led][color] = random(LED_MIN_PWM, maxAllowedPwm+1);
      ledDirections[led][color] = random(2) == 1 ? 1 : -1;
      ledSpeeds[led][color] = randomFloat(LED_SPEED_MIN, LED_SPEED_MAX+1);
      ledSpeedTimesBeforeChange[led][color] = random(LED_SPEED_CHANGE_TIME_MIN, LED_SPEED_CHANGE_TIME_MAX+1);
      ledSpeedChangeTimes[led][color] = millis();
    }
  }
}

float randomFloat(float min, float max) {
  float resolution = 100.0;
  int min_int = min * resolution;
  int max_int = max * resolution;
  return random(min_int, max_int) / resolution;
}

void loop() {
  if (!checkIsOn()) {
    return;
  }

  checkLedPot();
  loopLeds();

  checkMotorPot();
  loopMotors();

  // Booting fade in
  if (maxAllowedPwm != MAX_PWM) {
    maxAllowedPwm += 100;
    if (maxAllowedPwm > MAX_PWM) {
      maxAllowedPwm = MAX_PWM;
    }
  }

  delay(50);
}

bool checkIsOn() {
  // Switch is normally open so 0 means pressed.
  bool isSwitch = digitalRead(SWITCH_PIN) == 0;
  if (isSwitch && !isOn || !isSwitch && isOn) {
    if (switchTime == 0) {
      switchTime = millis();
    } else if (millis() - switchTime >= TIME_BEFORE_SWITCH_REGISTER) {
      switchTime = 0;
      if (isOn) {
        turnOff();
      } else {
        turnOn();
      }
    }
  }

  return isOn;
}

void turnOn() {
  isOn = true;
}

void turnOff() {
  clearLeds();
  clearMotors();

  isOn = false;
  maxAllowedPwm = BASE_MAX_ALLOWED_PWM;
}

void clearLeds() {
  for (int led = 0; led < 9; led++) {
    pwmDriver.setPWM(led, 0, 0);
  }
}

void clearMotors() {
  for (int motor = 9; motor < 12; motor++) {
    pwmDriver.setPWM(motor, 0, 0);
  }
}

void checkLedPot() {
  int ledPotVal = analogRead(LED_POT_PIN);
  int rangeStart = 0;
  int rangeEnd = 0;
  if (ledPotVal == 0) {
    ledMode = 0;
  } else if (ledPotVal >= 1 && ledPotVal <= 500) {
    ledMode = 1;
    rangeStart = 1;
    rangeEnd = 500;
  } else if (ledPotVal >= 501 && ledPotVal <= 750) {
    ledMode = 2;
    rangeStart = 501;
    rangeEnd = 750;
  } else if (ledPotVal >= 751 && ledPotVal <= 1023) {
    ledMode = 3;
    rangeStart = 751;
    rangeEnd = 1023;
  }

  ledSpeedMod = map(ledPotVal, rangeStart, rangeEnd, 100, 1000) / 100.0;
}

void loopLeds() {
  if (ledMode == 0) {
    for (int led = 0; led < 9; led++) {
      pwmDriver.setPWM(led, 0, maxAllowedPwm);
    }
  } else if (ledMode == 1) {
    for (int led = 0; led < 3; led++) {
      for (int color = 0; color < 3; color++) {
        int pwm = calcLedFadePwm(led, color);
        if (pwm > maxAllowedPwm) {
          ledDirections[led][color] = -1;
          pwm = maxAllowedPwm;
        } else if (pwm < LED_MIN_PWM) {
          ledDirections[led][color] = 1;
          pwm = LED_MIN_PWM;
        }

        int ledDriverIndex = led * 3 + color;
        pwmDriver.setPWM(ledDriverIndex, 0, pwm);
        ledPwms[led][color] = pwm;

        if (millis() - ledSpeedChangeTimes[led][color] >= ledSpeedTimesBeforeChange[led][color]) {
          ledSpeedChangeTimes[led][color] = millis();
          ledSpeedTimesBeforeChange[led][color] = random(LED_SPEED_CHANGE_TIME_MIN, LED_SPEED_CHANGE_TIME_MAX+1);
          ledSpeeds[led][color] = randomFloat(LED_SPEED_MIN, LED_SPEED_MAX+1);
        }
      }
    }
  } else if (ledMode == 2) {
    if (millis() - lastColorChange >= (11 - ledSpeedMod) * 100) {
      lastColorChange = millis();

      currentColor++;
      if (currentColor == 3) {
        currentColor = 0;
      }

      clearLeds();
      for (int led = 0 + currentColor; led < 9; led += 3) {
        pwmDriver.setPWM(led, 0, maxAllowedPwm);
      }
    }
  } else if (ledMode == 3) {
    if (millis() - lastColorChange >= (11 - ledSpeedMod) * 100) {
      lastColorChange = millis();

      clearLeds();
      for (int led = 0; led < 9; led++) {
        if (random(2) == 1) {
          pwmDriver.setPWM(led, 0, maxAllowedPwm);
        }
      }
    }
  }
}

int calcLedFadePwm(int led, int color) {
  float pwmChangeAmount = LED_BASE_STEPS * ledSpeeds[led][color] * ledSpeedMod;

  if (motorMode == 2) {
    pwmChangeAmount *= LED_FADE_DEPTH_SPEEDS[led];
  }

  pwmChangeAmount *= ledDirections[led][color];

  return ledPwms[led][color] + pwmChangeAmount;
}

void checkMotorPot() {
  int motorPotVal = analogRead(MOTOR_POT_PIN);
  int rangeStart = 0;
  int rangeEnd = 0;
  if (motorPotVal == 1023) {
    motorMode = 0;
  } else if (motorPotVal <= 1022 && motorPotVal >= 500) {
    motorMode = 1;
    rangeStart = 500;
    rangeEnd = 1022;
  } else if (motorPotVal <= 499 && motorPotVal >= 0) {
    motorMode = 2;
    rangeStart = 0;
    rangeEnd = 499;
  }

  motorSpeedMod = 11.0 - map(motorPotVal, rangeStart, rangeEnd, 100, 1000) / 100.0;
}

void loopMotors() {
  if (motorMode == 0) {
    clearMotors();
  } else {
    for (int motor = 0; motor < 3; motor++) {
      float normalizedSpeed = motorSpeedMod / 10.0;
      if (motorMode == 2) {
        normalizedSpeed *= MOTOR_DEPTH_SPEEDS[motor];
      }

      int maxPwm = max(MOTOR_MIN_PWM, maxAllowedPwm);

      int pwm = exponential(normalizedSpeed, MOTOR_MIN_PWM, maxPwm);
      if (pwm >= 3475) {
        pwm = maxPwm;
      }

      pwmDriver.setPWM(motor + 9, 0, pwm);
    }
  }
}

float exponential(float norm, float min, float max) {
  return min * pow(max / min, norm);
}
