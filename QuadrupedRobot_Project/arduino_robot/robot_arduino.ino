#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <math.h>

 Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// -------------------- Servo Assignment (Centralized Management) --------------------
// Defines the index for each servo automatically: FL_KNEE=0, FL_HIP=1, etc.
enum ServoIndex {
  FL_KNEE, // index 0: Front Left Knee
  FL_HIP,  // index 1: Front Left Hip
  FR_KNEE, // index 2: Front Right Knee
  FR_HIP,  // index 3: Front Right Hip
  RL_KNEE, // index 4: Rear Left Knee
  RL_HIP,  // index 5: Rear Left Hip
  RR_KNEE, // index 6: Rear Right Knee
  RR_HIP   // index 7: Rear Right Hip
};

struct ServoConfig {
  String name;
  int ch;          // PCA9685 Channel (0, 2, 4...)
  int minPulse;
  int centerPulse;
  int maxPulse;
  int direction;   // 1 or -1
};

// -------------------- Servo Configurations --------------------
// Order must match the ServoIndex enum above.
ServoConfig servos[] = {
  // Name,      Ch,  Min,  Center, Max,  Dir
  {"FL_KNEE",   0,   145,  330,    520,   1}, // index 0
  {"FL_HIP",    2,   140,  325,    450,   1}, // index 1 (Max increased to 450 for kicking)
  {"FR_KNEE",   4,   140,  320,    520,  -1}, // index 2
  {"FR_HIP",    6,   275,  300,    485,  -1}, // index 3
  {"RL_KNEE",   8,   200,  310,    500,  -1}, // index 4
  {"RL_HIP",   10,   280,  305,    495,  -1}, // index 5
  {"RR_KNEE",  12,   100,  300,    400,   1}, // index 6
  {"RR_HIP",   14,   120,  300,    325,   1}  // index 7
};

const int SERVO_COUNT = 8;

int mode = 0;  // 0: Idle, 2: Walking
float waveAngle = 0;
float stick_Y = 0.0;
float stick_X = 0.0;

void setup() {
  Serial.begin(115200);
  
  // I2C anti-hang measures (specific to R4 noise safety)
  Wire.begin();
  Wire.setWireTimeout(25000, true); 

  pwm.begin();
  pwm.setOscillatorFrequency(25000000);
  pwm.setPWMFreq(50);
  resetToCenter();
}

// -------------------- Servo Control Functions --------------------

/**
 * Moves a servo to a specific position based on a ratio (-1.0 to 1.0)
 */
void moveServo(int index, float ratio) {
  ServoConfig s = servos[index];
  
  // Apply rotation direction internally
  float final_ratio = ratio * s.direction;

  float p_range = (final_ratio >= 0) ? (s.maxPulse - s.centerPulse) : (s.centerPulse - s.minPulse);
  float targetPulse = s.centerPulse + (final_ratio * p_range);
  int safePulse = constrain((int)targetPulse, s.minPulse, s.maxPulse);
  pwm.setPWM(s.ch, 0, safePulse);
}

// Reset all servos to neutral position
void resetToCenter() {
  for (int i = 0; i < SERVO_COUNT; i++) {
    pwm.setPWM(servos[i].ch, 0, servos[i].centerPulse);
  }
}

// Set Squat (Crouching) pose
void setSquatPose() {
  moveServo(FL_KNEE, -1.4); 
  moveServo(FR_KNEE, -1.4); 
  moveServo(RL_KNEE, -1.4); 
  moveServo(RR_KNEE, -1.4); 
  moveServo(FL_HIP,   0.0);  
  moveServo(FR_HIP,   0.0);  
  moveServo(RL_HIP,   0.0);  
  moveServo(RR_HIP,   0.0);
}

// Jump Action
void executeJump() {
  setSquatPose();
  delay(150);
  moveServo(FL_KNEE, 1.0); 
  moveServo(FR_KNEE, 1.0); 
  delay(35);
  moveServo(RL_KNEE, 1.0); 
  moveServo(RR_KNEE, 1.0); 
  delay(150);
  moveServo(FL_KNEE, 0.0); moveServo(FR_KNEE, 0.0);
  moveServo(RL_KNEE, 0.0); moveServo(RR_KNEE, 0.0);
  delay(80);
  setSquatPose();
  delay(200);
}

// Kick Action
void kick(){
  setSquatPose();
  delay(150);
  moveServo(FL_HIP, -1.0); // Use the expanded range for a powerful kick
  delay(150);
  moveServo(FL_KNEE, 0.0);
  delay(200);
  setSquatPose();
  delay(200);
}

// Wave Hand Action
void executeWaveHand() {
  // 1. Stability: Crouch 3 legs and bend Rear-Right knee deeply
  moveServo(FR_KNEE, -1.4); 
  moveServo(RL_KNEE, -1.4); 
  
  // Shift center of gravity to Rear-Right
  moveServo(RR_HIP,  -0.6); 
  moveServo(RR_KNEE, -2.5); 

  // Gravity compensation
  moveServo(FR_HIP,  -0.3); 
  moveServo(RL_HIP,   0.2);  
  delay(250);
  
  // 2. Extend Front-Left leg (the "hand") forward
  moveServo(FL_KNEE,  0.8);  
  moveServo(RR_KNEE, -0.4);  
  moveServo(FL_HIP,  -1.0); 
  delay(200);

  // 3. Wave the "hand" 3 times
  for (int i = 0; i < 3; i++) {
    moveServo(FL_HIP, -0.5); 
    delay(150);
    moveServo(FL_HIP, -1.3); 
    delay(150);
  }

  // 4. Return to Squat pose smoothly
  setSquatPose();
  delay(200);
}

void loop() {
  // Handle Serial Commands
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.equalsIgnoreCase("w")) {
      mode = (mode == 2) ? 0 : 2;
      stick_Y = 0.0;
      stick_X = 0.0;
      if (mode == 0) setSquatPose();
    }
    else if (input.equalsIgnoreCase("j")) {
      executeJump();
    }
    else if (input.equalsIgnoreCase("h")) {
      executeWaveHand();
    }
    else if (input.equalsIgnoreCase("b")) {
      kick();
    }
    else if (input.startsWith("f") && input.indexOf(",r") != -1) {
      int rIndex = input.indexOf(",r");
      String fStr = input.substring(1, rIndex);
      String rStr = input.substring(rIndex + 2);
      stick_Y = fStr.toFloat();
      stick_X = rStr.toFloat();
    }
  }

  // ===== MODE 2: WALKING =====
  if (mode == 2) {
    if (abs(stick_Y) > 0.05 || abs(stick_X) > 0.05) {
      if (abs(stick_Y) > 0.05) {
        if (stick_Y > 0) {
          waveAngle += (0.04 + stick_Y * 0.04);
        } else {
          waveAngle -= (0.04 + abs(stick_Y) * 0.04);
        }
      } else {
        waveAngle += 0.04 + (2.5 * 0.04); 
      }
    }

    // Gait Phase Calculation
    float phaseFL = waveAngle;
    float phaseFR = waveAngle + PI;
    float phaseRL = waveAngle + PI;
    float phaseRR = waveAngle;

    float hip_phaseFL = phaseFL;
    float hip_phaseFR = phaseFR;
    float hip_phaseRL = phaseRL + PI;
    float hip_phaseRR = phaseRR + PI;

    float fl_knee, fr_knee, rl_knee, rr_knee;

    // Knee movement logic
    if (abs(stick_Y) > 0.05 || abs(stick_X) > 0.05) {
      fl_knee = (sin(phaseFL) - 3) / 2.0 * 0.7;
      fr_knee = (sin(phaseFR) - 3) / 2.0 * 0.7;
      rl_knee = (sin(phaseRL) - 3) / 2.0 * 0.7;
      rr_knee = (sin(phaseRR) - 3) / 2.0 * 0.7;
    } else {
      fl_knee = -1.4; fr_knee = -1.4; rl_knee = -1.4; rr_knee = -1.4;
    }

    // Steering calculation
    float base_Y = abs(stick_Y);
    float left_multiplier, right_multiplier;
    if (stick_Y >= 0) {
      left_multiplier  = base_Y + (stick_X * 1);
      right_multiplier = base_Y - (stick_X * 1);
    } else {
      left_multiplier  = base_Y - (stick_X * 1);
      right_multiplier = base_Y + (stick_X * 1);
    }

    left_multiplier  = constrain(left_multiplier, 0.0, 5.0);
    right_multiplier = constrain(right_multiplier, 0.0, 5.0);

    // Hip movement logic
    float fl_hip = cos(hip_phaseFL) * 0.6 * (left_multiplier / 5.0);
    float fr_hip = cos(hip_phaseFR) * 0.6 * (right_multiplier / 5.0);
    float rl_hip = cos(hip_phaseRL) * 0.6 * (left_multiplier / 5.0);
    float rr_hip = cos(hip_phaseRR) * 0.6 * (right_multiplier / 5.0);

    // Final Servo Output
    moveServo(FL_KNEE, fl_knee);
    moveServo(FL_HIP,  fl_hip);
    moveServo(FR_KNEE, fr_knee);
    moveServo(FR_HIP,  fr_hip);
    moveServo(RL_KNEE, rl_knee);
    moveServo(RL_HIP,  rl_hip);
    moveServo(RR_KNEE, rr_knee);
    moveServo(RR_HIP,  rr_hip);

    delay(20);
  }
}