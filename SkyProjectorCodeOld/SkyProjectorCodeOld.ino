#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwmDriver = Adafruit_PWMServoDriver();

const int LED_MIN_PWM = 200;
const int LED_MAX_PWM = 4095;
const int LED_BASE_STEPS = 6;
const int LED_SPEED_VARIANCE_RANGE = 1;
int LED_BASE_SPEEDS[] = {10, 12, 14};

int ledPwms[3][3];
int ledSpeeds[3][3];


void setup() {
  randomSeed(analogRead(A5));

  pwmDriver.begin();
  pwmDriver.setPWMFreq(500);

  // Motors from slow to fast
  pwmDriver.setPWM(0, 0, 4095); // First motor is slow of itself
  pwmDriver.setPWM(1, 0, 3000);
  pwmDriver.setPWM(2, 0, 4095);

  // Starting led pwms
  for (int led = 0; led < 3; led++) {
    for (int color = 0; color < 3; color++) {
      ledPwms[led][color] = random(LED_MIN_PWM, LED_MAX_PWM);
    }
  }

  // Create ledSpeedVariances
  // For example: with LED_SPEED_VARIANCE_RANGE=3 it will be -3, -2, -1, 0, 1, 2, 3
  int ledSpeedVariancesLen = LED_SPEED_VARIANCE_RANGE * 2 + 1;
  int ledSpeedVariances[ledSpeedVariancesLen];
  for (int i = 0; i < ledSpeedVariancesLen; i++) {
    ledSpeedVariances[i] = i - LED_SPEED_VARIANCE_RANGE;
  }

  // Led speeds
  for (int led = 0; led < 3; led++) {
    shuffleArray(ledSpeedVariances, ledSpeedVariancesLen);
    for (int color = 0; color < 3; color++) {
      ledSpeeds[led][color] = LED_BASE_SPEEDS[led] + ledSpeedVariances[color];
    }
  }
}

void loop() {
  for (int led = 0; led < 3; led++) {
    for (int color = 0; color < 3; color++) {
      int pwm = calcLedPwm(led, color);
      if (pwm > LED_MAX_PWM || pwm < LED_MIN_PWM) {
        ledSpeeds[led][color] *= -1;
        pwm = calcLedPwm(led, color);
      }

      // The + 3 is because the first three are the motors
      int ledDriverIndex = led * 3 + color + 3;
      pwmDriver.setPWM(ledDriverIndex, 0, pwm);

      ledPwms[led][color] = pwm;
    }
  }

  delay(50);
}

void swap(int *a, int *b)
{
  int temp = *a;
  *a = *b;
  *b = temp;
}

void shuffleArray(int arr[], int len)
{
  for (int i = len - 1; i > 0; i--)
  {
    long j = random(0, i + 1);
    swap(&arr[i], &arr[j]);
  }
}

int calcLedPwm(int led, int color) {
  return ledPwms[led][color] + LED_BASE_STEPS * ledSpeeds[led][color];
}
