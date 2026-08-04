#include "Servo.h"
#include "NewPing.h"

Servo x;
Servo y;

#define TRIGGER_PIN  12  
#define ECHO_PIN     10  
#define MAX_DISTANCE 60

int ping;
bool tracking = false;
int pos_x=90;
int pos_y=30;
int step_y=15;
int target_dist;
int median_time;

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

bool tryLeft() {
  int try_pos = pos_x - 15;
  if (try_pos < 15) try_pos = 0; 
  
  x.write(try_pos);
  
  delay(150);
  
  median_time = sonar.ping_median(3);
  ping = sonar.convert_cm(median_time);
  
  //optional value print
  //Serial.println(ping); 

  if (ping > 0 && ping > target_dist - 10 && ping < target_dist + 10) { 
    pos_x = try_pos;
    target_dist = ping; 
    return true;   
  }
  return false; 
}

bool tryRight() {
  int try_pos = pos_x + 15;
  if (try_pos > 165) try_pos = 180; 
  
  x.write(try_pos);
  
  delay(150); 
  
  median_time = sonar.ping_median(3);
  ping = sonar.convert_cm(median_time);
  //Serial.println(ping);

  if (ping > target_dist - 10 && ping < target_dist + 10) {
    pos_x = try_pos;
    target_dist = ping;
    return true;
  }
  return false;
}


bool tryUp() {
  int try_pos = pos_y - 30;
  if(try_pos < 30) {
    try_pos = 30;
  }

  x.write(pos_x);
  delay(15);
  y.write(try_pos);

  delay(150);

  median_time = sonar.ping_median(3);
  ping = sonar.convert_cm(median_time);
  //Serial.println(ping);

  if (ping > target_dist - 10 && ping < target_dist + 10) { 
    pos_y = try_pos; 
    target_dist = ping;
    return true;   
  }
  return false;
}


bool tryDown() {
  int try_pos = pos_y + 45;
  if(try_pos > 120) {
    try_pos = 120;
  }
  x.write(pos_x);
  delay(15);
  y.write(try_pos);

  delay(150);

  median_time = sonar.ping_median(3);
  ping = sonar.convert_cm(median_time);
  //Serial.println(ping);

  if (ping > target_dist - 10 && ping < target_dist + 10) { 
    pos_y = try_pos; 
    target_dist = ping;
    return true;   
  }
  return false;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  x.attach(6);
  y.attach(3);
  
  y.write(pos_y);
  delay(150);
  x.write(pos_x);
  delay(150);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (tracking == false) {
        
    if (tracking == false) {
        
      pos_y = pos_y + step_y;

      if(pos_y >= 90) {
          pos_y = 90;
          step_y = -15;
      }
      
      if (pos_y <= 30) {
          pos_y = 30;
          step_y = 15;
      }
        
      y.write(pos_y);
      delay(150);
    }

      for (int i=0; i<=180; i+=2) {
        x.write(i);
        
        delay(50);
        
        median_time = sonar.ping_median(3);
        ping = sonar.convert_cm(median_time);
        //Serial.println(ping);

        if (ping > 0 && ping <= 40) { 
          pos_x = i;              
          tracking = true;
          target_dist = ping;      
          break; 
        }    
      }
    
    if (tracking == false) {
      
      pos_y = pos_y + step_y;

      if(pos_y >= 90) {
          pos_y = 90;
          step_y = -15;
      }

      if (pos_y <= 30) {
          pos_y = 30;
          step_y = 15;
      }
        
      y.write(pos_y);
      delay(150);
    }
    

    if (tracking == false) {
      for (int i=180; i>=0; i-=2) {
        x.write(i);
        
        delay(50);

        median_time = sonar.ping_median(3);
        ping = sonar.convert_cm(median_time);
        //Serial.println(ping);

        if (ping > 0 && ping <= 40) { 
          pos_x = i;              
          tracking = true;
          target_dist = ping;      
          break; 
        }
      }
    }

    
  }

  if (tracking == false) {
    pos_x = 0;
  }

  while (tracking == true) {
    
    delay(50);
    median_time = sonar.ping_median(3);
    ping = sonar.convert_cm(median_time);
    //Serial.println(ping);

    if (ping == 0 || ping > 40) {
      if (tryLeft() == true) {
        continue;
      }
      
      x.write(pos_x);
      delay(150);

      if (tryRight() == true) {
        continue;
      }

      x.write(pos_x);
      delay(150);

      if (tryUp() == true) {
        continue;
      }

      y.write(pos_y);
      delay(150);

      if (tryDown() == true) {
        continue;
      }

      tracking = false;

    }
  }
}