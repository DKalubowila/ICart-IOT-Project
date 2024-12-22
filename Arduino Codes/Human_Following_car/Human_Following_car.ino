#include <AFMotor.h> 

#define echoPin 11 // attach pin D9 on Arduino (SER1 on motor shield)) to ECHO pin on ultrasonic sensor
#define trigPin 10 // attach pin D10 on Arduino (SERVO_2 on motor shield) to TRIG pin on ultrasonic sensor

long duration; // duration of sound wave travel
int distance; // distance measured

// Desired distance from the object
int desired_distance = 15; // You can adjust this value as needed

// Initialise the four motors
AF_DCMotor front_left(1, MOTOR12_64KHZ);
AF_DCMotor rear_left(2, MOTOR12_64KHZ);
AF_DCMotor front_right(3, MOTOR12_64KHZ);
AF_DCMotor rear_right(4, MOTOR12_64KHZ);

// PID parameters
double kp = 5.0; // Proportional gain
double ki = 0.1; // Integral gain
double kd = 0.1; // Derivative gain
double prevError = 0;
double integral = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(trigPin, OUTPUT); // set trigPin as OUTPUT
  pinMode(echoPin, INPUT); // set echoPin as INPUT
  
  Serial.begin(9600); //Start serial communication at 9600 baud rate
}

void loop() {
  // put your main code here, to run repeatedly:
  // clear trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Sets trigPin to HIGH for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echoPin and return the travel time of sound wave in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate distance travelled
  distance = (duration * 0.034 / 2); // Speed of sound wave divided by 2 (out and back)
  
  // Display the distance measured in cm in the serial monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  if (distance > 10) {
    // Stop the motors if the object is too far away
    front_left.run(RELEASE);     
    rear_left.run(RELEASE); 
    front_right.run(RELEASE);  
    rear_right.run(RELEASE);
  } else {
    // Calculate PID control
    double error = distance - desired_distance;
    integral += error;
    double derivative = error - prevError;
    double output = kp * error + ki * integral + kd * derivative;
    prevError = error;
    
    // Limit the output to the range [-255, 255]
    output = constrain(output, -255, 255);
    
    // Set the speed for each motor (values can be between 0 and 255)
    int speed = abs(output);
    front_left.setSpeed(speed);
    rear_left.setSpeed(speed);
    front_right.setSpeed(speed);
    rear_right.setSpeed(speed);
    
    // Set the direction of the motor
    if(output > 0) {
      // Object is too close, move backward
      front_left.run(FORWARD);     
      rear_left.run(FORWARD); 
      front_right.run(FORWARD);  
      rear_right.run(FORWARD);
    } else {
      // Object is too far, move forward
      front_left.run(BACKWARD);       
      rear_left.run(BACKWARD);  
      front_right.run(BACKWARD);       
      rear_right.run(BACKWARD); 
    }
  }
  
  delay(100); 
}
