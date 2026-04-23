#include <Servo.h>

// Servos
Servo elevonL;
Servo elevonR;
Servo throttleServo;

// Inputs
const int rollPin = A0;
const int pitchPin = A1;
const int throttlePin = A2;

// Controls
const int fuelPumpSwitch = 4;
const int starterBtn     = 3;
const int ignitionBtn    = 2;

// Engine state
enum EngineState {
  OFF,
  STARTING,
  RUNNING,
  SHUTDOWN
};

EngineState state = OFF;

// System vars
float starterSpool = 0;
float engineRPM = 0;

bool ignitionUsed = false;
bool ignitionArmed = false;
unsigned long ignitionTimer = 0;

bool flameout = false;

// thresholds
const int LIGHT_OFF_RPM = 45;
const int IDLE_RPM = 60;

// smoothing target
float rpmTarget = 0;

// physics tuning
float engineInertiaUp = 0.04;
float engineInertiaDown = 0.02;

float starterAccel = 0.06;
float starterDecay = 0.03;

// ignition delay
const unsigned long IGNITION_DELAY = 800;

// ------------------------

int applyDeadzone(int v) {
  if (abs(v) < 20) return 0;
  return v;
}

// ------------------------

void setup() {
  elevonL.attach(11);
  elevonR.attach(10);
  throttleServo.attach(6);

  pinMode(fuelPumpSwitch, INPUT_PULLUP);
  pinMode(starterBtn, INPUT_PULLUP);
  pinMode(ignitionBtn, INPUT_PULLUP);
}

// ------------------------

void loop() {

  bool fuelOn = (digitalRead(fuelPumpSwitch) == LOW);
  bool starterHeld = (digitalRead(starterBtn) == LOW);
  bool ignition = (digitalRead(ignitionBtn) == LOW);

  int thr = analogRead(throttlePin);

  // =========================
  // FLAMEOUT LOGIC
  // =========================
  if (!fuelOn && state == RUNNING && !flameout) {
    flameout = true;
    rpmTarget *= 0.7;
  }

  if (flameout) {
    rpmTarget -= 0.5;

    if (rpmTarget <= 5) {
      rpmTarget = 0;
      flameout = false;
      state = SHUTDOWN;
      ignitionUsed = false;
    }
  }

  // =========================
  // FUEL STATE CONTROL
  // =========================
  if (!fuelOn && state != OFF && state != SHUTDOWN) {
    state = SHUTDOWN;
    ignitionUsed = false;
    ignitionArmed = false;
  }

  if (fuelOn && state == OFF) {
    state = STARTING;
    starterSpool = 0;
    engineRPM = 0;
    ignitionUsed = false;
    ignitionArmed = false;
  }

  // =========================
  // STARTING SEQUENCE
  // =========================
  if (state == STARTING) {

    if (starterHeld) {
      starterSpool += starterAccel * (1.0 - starterSpool / 100.0);
    } else {
      starterSpool -= starterDecay;
    }

    starterSpool = constrain(starterSpool, 0, 80);

    rpmTarget = map(starterSpool, 0, 80, 0, IDLE_RPM);

    // =========================
    // IGNITION DELAY SYSTEM
    // =========================
    if (!ignitionArmed &&
        fuelOn &&
        ignition &&
        starterSpool >= LIGHT_OFF_RPM) {

      ignitionArmed = true;
      ignitionTimer = millis();
    }

    if (ignitionArmed && !ignitionUsed) {
      if (millis() - ignitionTimer >= IGNITION_DELAY) {

        ignitionUsed = true;
        state = RUNNING;

        rpmTarget = IDLE_RPM;
      }
    }
  }

  // =========================
  // RUNNING STATE
  // =========================
  if (state == RUNNING) {

    starterSpool = 0;

    float throttle = thr / 1023.0;
    rpmTarget = IDLE_RPM + throttle * 120;
  }

  // =========================
  // SHUTDOWN
  // =========================
  if (state == SHUTDOWN) {

    ignitionUsed = false;
    ignitionArmed = false;

    rpmTarget -= 2;
    if (rpmTarget <= 0) {
      rpmTarget = 0;
      state = OFF;
    }
  }

  // =========================
  // ENGINE INERTIA
  // =========================
  float diff = rpmTarget - engineRPM;

  if (diff > 0) {
    engineRPM += diff * engineInertiaUp;
  } else {
    engineRPM += diff * engineInertiaDown;
  }

  engineRPM = constrain(engineRPM, 0, 180);

  // =========================
  // ELEVONS
  // =========================
  int roll = applyDeadzone(analogRead(rollPin) - 512);
  int pitch = applyDeadzone(analogRead(pitchPin) - 512);

  roll /= 5;
  pitch /= 5;

  int left = 90 + pitch + roll;
  int right = 90 + pitch - roll;

  elevonL.write(constrain(left, 30, 150));
  elevonR.write(180 - constrain(right, 30, 150));

  // =========================
  // OUTPUT
  // =========================
  throttleServo.write((int)engineRPM);

  delay(10);
}