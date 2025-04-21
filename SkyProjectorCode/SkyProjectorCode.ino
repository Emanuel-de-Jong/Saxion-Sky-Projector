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

const int POT_MAX = 1023;
const int PWM_MAX = 4095;

const int BTN_PIN = 11;
const int LED_POT_PIN = A1;
const int MOTOR_POT_PIN = A0;

const unsigned long BTN_VALIDATION_COUNT_THRESHOLD = 3;
const unsigned long BTN_SHORT_THRESHOLD = 500;
const int COLOR_MODE_COUNT = 9;
const int COLOR_SCOPE_COUNT = 2;
const bool[][9] COLOR_MODES = {
  {1,1,1, 1,1,1, 1,1,1}, // rgb
  {1,0,0, 1,0,0, 1,0,0}, // r
  {0,1,0, 0,1,0, 0,1,0}, // g
  {0,0,1, 0,0,1, 0,0,1}, // b
  {1,1,0, 1,1,0, 1,1,0}, // rg
  {1,0,1, 1,0,1, 1,0,1}, // rb
  {0,1,1, 0,1,1, 0,1,1}, // gb
  {1,0,0, 0,1,0, 0,0,1}, // r, g, b
  {1,1,0, 1,0,1, 0,1,1} // rg, rb, gb
};

bool isColorModeChanged = true;
int colorMode = 0;
// 0=Per color, 1=Per led
int colorScope = 0;
int btnTime = 0;
int btnValidationCount = 0;

const int LED_MIN_PWM = 200;
const int LED_BASE_STEPS = 5;
const float LED_SPEED_MIN = 0.7;
const float LED_SPEED_MAX = 1.3;
const int LED_SPEED_CHANGE_TIME_MIN = 1000;
const int LED_SPEED_CHANGE_TIME_MAX = 5000;
const float LED_FADE_DEPTH_SPEEDS[] = {1.3, 1.0, 0.7};

// 0=Static, 1=Fade, 2=Toggle
int ledMode = 0;
float ledSpeedMod = 1;

int ledPwms[3][3];
int ledDirections[3][3];
float ledSpeeds[3][3];
int ledSpeedTimesBeforeChange[3][3];
int ledSpeedChangeTimes[3][3];

int lastColorChange = millis();
bool lastColorCombination[9];

const int MOTOR_MIN_PWM = 800;
const float MOTOR_DEPTH_SPEEDS[] = {1.0, 0.8, 0.9};

// 0=Off, 1=Global, 2=Depth
int motorMode = 0;
int motorSpeedMod = 1;

void setup() {
  randomSeed(analogRead(A5));
  Serial.begin(9600);

  pinMode(BTN_PIN, INPUT_PULLUP);

  pwmDriver.begin();
  pwmDriver.setPWMFreq(500);
}

float randomFloat(float min, float max) {
  float resolution = 100.0;
  int min_int = min * resolution;
  int max_int = max * resolution;
  return random(min_int, max_int) / resolution;
}

void loop() {
  checkMode();

  checkLedPot();
  checkMotorPot();

  loopLeds();
  loopMotors();

  isColorModeChanged = false;

  delay(25);
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

void checkMode() {
  bool isBtnPressed = digitalRead(BTN_PIN) == LOW;

  if (isBtnPressed) {
    btnValidationCount++;
  }

  if (isBtnPressed && btnTime == 0) {
    btnTime = millis();
  } else if (!isBtnPressed && btnTime != 0) {
    if (btnValidationCount >= 3) {
      int btnDuration = millis() - btnTime;
      if (btnDuration <= BTN_SHORT_THRESHOLD) {
        colorMode++;
        if (colorMode == COLOR_MODE_COUNT) {
          colorMode = 0;
        }

        isColorModeChanged = true;
      } else {
        colorScope++;
        if (colorScope == COLOR_SCOPE_COUNT) {
          colorScope = 0;
        }
      }
    }

    btnTime = 0;
    btnValidationCount = 0;
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
  } else if (ledPotVal >= 501 && ledPotVal <= POT_MAX) {
    ledMode = 2;
    rangeStart = 501;
    rangeEnd = POT_MAX;
  }

  ledSpeedMod = map(ledPotVal, rangeStart, rangeEnd, 1, 1000) / 10.0;
}

void checkMotorPot() {
  int motorPotVal = POT_MAX - analogRead(MOTOR_POT_PIN);
  int rangeStart = 0;
  int rangeEnd = 0;
  if (motorPotVal == 0) {
    motorMode = 0;
  } else if (motorPotVal >= 1 && motorPotVal <= 500) {
    motorMode = 1;
    rangeStart = 1;
    rangeEnd = 500;
  } else if (motorPotVal >= 501 && motorPotVal <= POT_MAX) {
    motorMode = 2;
    rangeStart = 501;
    rangeEnd = POT_MAX;
  }

  motorSpeedMod = map(motorPotVal, rangeStart, rangeEnd, 1, 1000) / 10.0;
}

void loopLeds() {
  if (ledMode == 0) {
    for (int led = 0; led < 9; led++) {
      pwmDriver.setPWM(led, 0, PWM_MAX);
    }
  } else if (ledMode == 1) {
    if (isColorModeChanged) {
      clearLeds();

      for (int led = 0; led < 3; led++) {
        for (int color = 0; color < 3; color++) {
          ledPwms[led][color] = PWM_MAX;
        }
      }
    }

    int speedMod = exponential(ledSpeedMod, 1, 100);

    for (int led = 0; led < 3; led++) {
      for (int color = 0; color < 3; color++) {
        if (colorScope == 1 && color > 0) {
          continue;
        }

        int ledDriverIndex = led * 3 + color;
        if (!COLOR_MODES[colorMode][ledDriverIndex]) {
          continue;
        }

        setLedFadePwm(led, color, speedMod);

        pwmDriver.setPWM(ledDriverIndex, 0, pwm);
        if (colorScope == 1) {
          pwmDriver.setPWM(ledDriverIndex + 1, 0, pwm);
          pwmDriver.setPWM(ledDriverIndex + 2, 0, pwm);
        }
      }
    }
  } else if (ledMode == 2) {
    int speedMod = exponential(ledSpeedMod, 1, 100);
    int timeBetweenColorChange = 2000 + 20 - map(speedMod, 1, 100, 20, 2000)
    if (millis() - lastColorChange >= timeBetweenColorChange) {
      lastColorChange = millis();

      do {
        bool colorCombination[9] = {};
        for (int color = 0; color < 9; color++) {
          if (random(2) == 1) {
            colorCombination[color] = true;
          } else {
            colorCombination[color] = false;
          }
        }

        // Prevent darkness
        colorCombination[random(9)] = true;
      } while (compareColorCombinations(colorCombination, lastColorCombination));

      clearLeds();
      for (int color = 0; color < 9; color++) {
        if (colorCombination[color]) {
          pwmDriver.setPWM(color, 0, pwm);
        }
      }

      lastColorCombination = colorCombination;
    }
  }
}

float exponential(float value, float min, float max, float curve = 2.0) {
  float norm = (value - min) / (max - min);
  norm = constrain(norm, 0.0, 1.0);
  float curved = pow(norm, curve);
  return min + curved * (max - min);
}

bool compareColorCombinations(bool[9] combination1, bool[9] combination2) {
  if (combination1 == nullptr || combination2 == nullptr) {
    return combination1 == nullptr && combination2 == nullptr;
  }

  bool isIdentical = true;
  for (int color = 0; color < 9; color++) {
    if (combination1[color] != combination2[color]) {
      isIdentical = false;
      break;
    }
  }

  return isIdentical;
}

int setLedFadePwm(int led, int color, float speedMod) {
  float pwmChangeAmount = LED_BASE_STEPS * ledSpeeds[led][color] * (speedMod / 10) * ledDirections[led][color];
  if (motorMode == 2) {
    pwmChangeAmount *= LED_FADE_DEPTH_SPEEDS[led];
  }

  int pwm = ledPwms[led][color] + pwmChangeAmount;
  if (pwm > PWM_MAX) {
    ledDirections[led][color] = -1;
    pwm = PWM_MAX;
  } else if (pwm < LED_MIN_PWM) {
    ledDirections[led][color] = 1;
    pwm = LED_MIN_PWM;
  }

  ledPwms[led][color] = pwm;

  if (millis() - ledSpeedChangeTimes[led][color] >= ledSpeedTimesBeforeChange[led][color]) {
    ledSpeedChangeTimes[led][color] = millis();
    ledSpeedTimesBeforeChange[led][color] = random(LED_SPEED_CHANGE_TIME_MIN, LED_SPEED_CHANGE_TIME_MAX+1);
    ledSpeeds[led][color] = randomFloat(LED_SPEED_MIN, LED_SPEED_MAX+1);
  }
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

      int maxPwm = max(MOTOR_MIN_PWM, PWM_MAX);

      int pwm = exponential(normalizedSpeed, MOTOR_MIN_PWM, maxPwm);
      if (pwm >= 3475) {
        pwm = maxPwm;
      }

      pwmDriver.setPWM(motor + 9, 0, pwm);
    }
  }
}
