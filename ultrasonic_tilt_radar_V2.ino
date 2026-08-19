#include "Arduino.h"
#include "Servo.h"
#include "NewPing.h"

Servo x;
Servo y;

#define TRIGGER_PIN  12  
#define ECHO_PIN     10  
#define MAX_DISTANCE 60

// EMA (Exponential Moving Average) coefficient for motion prediction. 
// Values close to 1 react faster, values close to 0 make the movement smoother
const float ALPHA = 0.5;

const int X_MIN = 0;      
const int X_MAX = 180;
const int X_STEP = 15;      
const int X_START = 30;
float ema_vel_x = 0;
float prev_pos_x = 0;
int pos_x = 90;

const int Y_MIN = 30;
const int Y_MAX = 150;
const int Y_STEP = 30;   
float ema_vel_y = 0;
int pos_y = Y_MIN;
float prev_pos_y = pos_y; 
int step_y=Y_STEP;

const float VELOCITY_THRESHOLD = 2.0;

// RANGE GATING: The allowed forward/backward movement of the target (in cm) 
// without classifying it as noise or a static background object
const int TARGET_TOLERANCE = 10;  
const int SEARCH_DIST = 40; 

// Centralized timing controls for easy radar speed and stability tuning
const int DELAY_SERVO_STABLE = 60;    
const int DELAY_Y_BEFORE = 15;         
const int DELAY_PING = 25;    
const int DELAY_SCAN_STEP = 50;        
const int DELAY_MOVE_STEP = 150;       

int target_dist;      
int ping;
bool tracking = false;

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// Digital Filter (Double-Check): Fires 2 pings to reject false positives
// The delay between pings prevents strong servos from jittering
int getPing() {
  int  p1 = sonar.ping_cm();

  delay(DELAY_PING);

  int p2 = sonar.ping_cm();

  // If one of the pings fails (returns 0), trust the one that found a target
  if (p1 == 0) return p2;
  if (p2 == 0) return p1;

  // If the difference is big (>15cm), it means we caught a "ghost" (noise)
  if (abs(p1-p2) > 15 ) {
    return 0;
  }

  return (p1+p2) / 2;
}

bool try_horizontal(int direction) {
    
    int try_pos = pos_x + (direction * X_STEP);
    try_pos = constrain(try_pos, X_MIN, X_MAX);
    
    x.write(try_pos);
    delay(DELAY_MOVE_STEP);
    ping = getPing();
    
    // Check if the target is within the ±10cm "window" of its previous position
    if (ping > 0 && ping > target_dist - TARGET_TOLERANCE && ping < target_dist + TARGET_TOLERANCE) {
        pos_x = try_pos;          
        target_dist = ping; //Dynamically update the target's distance 
        prev_pos_x = pos_x;
        return true;
    }
    return false;
}


bool try_vertical(int direction) {

   int try_pos = pos_y + (direction * Y_STEP);
   try_pos = constrain(try_pos, Y_MIN, Y_MAX);
   

    x.write(pos_x);
    delay(DELAY_Y_BEFORE);
    y.write(try_pos);
    delay(DELAY_SERVO_STABLE);

    ping = getPing();

    if (ping > 0 && ping > target_dist - TARGET_TOLERANCE && ping < target_dist + TARGET_TOLERANCE) { 
      pos_y = try_pos; 
      target_dist = ping;
      prev_pos_y = pos_y;
      return true;   
    }
    return false;
}

// Raster Scan Algorithm: Sweeps the area in a zig-zag pattern (using Ternary operators)  
void ScanX(int direction) {
  int start = (direction == 1) ? X_MIN : X_MAX;
  int end   = (direction == 1) ? X_MAX : X_MIN;
  int step  = (direction == 1) ? 2 : -2;

  for (int i = start; (direction == 1) ? (i <= end) : (i >= end); i += step) {
      x.write(i);
      delay(DELAY_SCAN_STEP);
      ping = getPing();
      Serial.println(ping);

      //Lock tracking and store its initial distance
      if (ping > 0 && ping <= SEARCH_DIST) {
          pos_x = i;
          tracking = true;
          target_dist = ping;
          prev_pos_x = pos_x;   
          break;
      }
  }
}

void MoveY() {

      if(pos_y >= Y_MAX) {
        pos_y = Y_MAX;
        step_y = -Y_STEP;
      }
       if (pos_y <= Y_MIN) {
        pos_y = Y_MIN;
        step_y = Y_STEP;
      }

      pos_y = pos_y + step_y;  
      y.write(pos_y);
      delay(DELAY_MOVE_STEP);
}


void setup() {

  Serial.begin(9600);
  
  x.attach(6);
  y.attach(3);
  y.write(Y_MIN);
  delay(DELAY_MOVE_STEP);
  x.write(X_START);
  delay(DELAY_MOVE_STEP);
}

void loop() {

  // PHASE 1: SEARCH MODE
  if (tracking == false) {
    
    ScanX(1);
    MoveY();
    
    if (tracking) return;

    ScanX(-1);
    MoveY();

    if (tracking) return;
  }

  //safe re-center if nothing is found
  if (tracking == false) {
    x.write(0);
    prev_pos_x = pos_x; 
    pos_x = 0;
    
  }

  // PHASE 2: TRACKING MODE
  while (tracking == true) {
    
    ping = getPing();

    // If the target is lost, perform small exploratory sweeps around its last known position
    if (ping == 0 || ping > SEARCH_DIST) {
      if (try_horizontal(1) == true) {
        continue;
      }
      
      x.write(pos_x);
      delay(DELAY_SCAN_STEP);

      if (try_horizontal(-1) == true) {
        continue;
      }

      x.write(pos_x);
      delay(DELAY_SCAN_STEP);

      if (try_vertical(1) == true) {
        continue;
      }

      y.write(pos_y);
      delay(DELAY_SCAN_STEP);

      if (try_vertical(-1) == true) {
        continue;
      }

      // If all attempts fail, break the loop and return to Raster Scan
      tracking = false;

    } else {

      // PREDICTIVE TRACKING
      // Calculate target momentum (Difference between current and previous position)
      float current_vel_x = pos_x - prev_pos_x;
      ema_vel_x = ALPHA * current_vel_x + (1 - ALPHA) * ema_vel_x; //velocity smoothing
      prev_pos_x = pos_x;

      float current_vel_y = pos_y - prev_pos_y;
      ema_vel_y = ALPHA * current_vel_y + (1 - ALPHA) * ema_vel_y;
      prev_pos_y = pos_y;
      
      // If the target is moving faster than the threshold , guess its next step
      if (abs(ema_vel_x) > VELOCITY_THRESHOLD) {
            int dir = (ema_vel_x > 0) ? 1 : -1;
            try_horizontal(dir);
      }
      if (abs(ema_vel_y) > VELOCITY_THRESHOLD) {
            int dir = (ema_vel_y > 0) ? 1 : -1;
            try_vertical(dir);
      }
    }
  }  
}
