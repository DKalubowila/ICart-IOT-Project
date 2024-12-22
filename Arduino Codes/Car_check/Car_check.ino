#include <AFMotor.h> 

// Initialise the four motors
AF_DCMotor front_left(1, MOTOR12_64KHZ);
AF_DCMotor rear_left(2, MOTOR12_64KHZ);
AF_DCMotor front_right(3, MOTOR12_64KHZ);
AF_DCMotor rear_right(4, MOTOR12_64KHZ);

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:

  // Set the speed for each motor (values can be between 0 and 255)
  front_left.setSpeed(255);
  rear_left.setSpeed(255);
  front_right.setSpeed(255);
  rear_right.setSpeed(255);

  // Set the direction of the motor - can be FORWARD, BACKWARD or RELEASE (release is the same as brakes)
  front_left.run(FORWARD);       
  rear_left.run(FORWARD);  
  front_right.run(FORWARD);       
  rear_right.run(FORWARD);    
   
  delay(1000); 
  front_left.run(BACKWARD);     
  rear_left.run(BACKWARD); 
  front_right.run(BACKWARD);  
  rear_right.run(BACKWARD);   
  
  delay(1000);  
  front_left.run(RELEASE);     
  rear_left.run(RELEASE); 
  front_right.run(RELEASE);  
  rear_right.run(RELEASE); 
  delay(1000); 
}
