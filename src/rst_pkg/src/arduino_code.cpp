#include <PID_v1.h>
#include <Encoder.h>
#include <ros.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Vector3Stamped.h>

// Motor control pins
const int pwmPin = 6;    // PWM output to motor driver
const int dirPin = 5;    // Direction control for motor driver
const int pwmPin2 = 7;   // PWM output to motor driver
const int dirPin2 = 9;   // Direction control for motor driver

// Encoder pins
const int encoderPinA = 18; // Encoder A pin
const int encoderPinB = 19; // Encoder B pin
const int encoder2PinA = 3; // Encoder A pin
const int encoder2PinB = 2; // Encoder B pin

// Encoder objects
Encoder myEnc(encoderPinA, encoderPinB);
Encoder myEnc2(encoder2PinA, encoder2PinB);

// PID parameters
double Setpoint, Setpoint2, Input, Output, Input2, Output2;
double Kp = 20.0, Ki = 10.0, Kd = 0.03;
double Kp2 = 5.0, Ki2 = 3.0, Kd2 = 2.0;

// PID instances
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
PID myPID2(&Input2, &Output2, &Setpoint2, Kp2, Ki2, Kd2, DIRECT);

// Speed calculation variables
volatile long prevEncoderCount = 0, prevEncoderCount2 = 0;
unsigned long prevTime = 0;
double actualSpeed = 0, actualSpeed2 = 0;

// Timing constants
const unsigned long speedInterval = 100; // Speed calculation interval (ms)

// Constants for robot and motor
float max_motor_speed = 255; // Maximum motor PWM
int min_motor_speed = 50;    // Minimum PWM to overcome motor deadzone
const int cpr = 2797;        // Encoder counts per revolution
const float wheel_base = 0.22;  // Wheelbase in meters

// ROS NodeHandle and messages

// Function to handle velocity commands from cmd_vel topic
void handle_cmd_vel(const geometry_msgs::Twist &msg) {
    float linear_x = msg.linear.x;
    float angular_z = msg.angular.z;

    // Calculate individual wheel speeds
    float right_speed = linear_x + (wheel_base / 2.0) * angular_z;
    float left_speed = linear_x - (wheel_base / 2.0) * angular_z;

    // Convert to PWM values
    int right_motor_pwm = constrain(right_speed * max_motor_speed / 0.5, -max_motor_speed, max_motor_speed);
    int left_motor_pwm = constrain(left_speed * max_motor_speed / 0.5, -max_motor_speed, max_motor_speed);

    // Convert PWM to desired speed (RPM)
    Setpoint = 251 * (right_motor_pwm) / 255;
    Setpoint2 = 251 * (left_motor_pwm) / 255; 
}
ros::NodeHandle nh;
ros::Subscriber<geometry_msgs::Twist> sub("cmd_vel", handle_cmd_vel);
geometry_msgs::Vector3Stamped speed_msg;
ros::Publisher speed_pub("speed", &speed_msg);

void setup() {
    nh.initNode();
    nh.subscribe(sub);
    nh.advertise(speed_pub);

    // Initialize Serial Monitor
    Serial.begin(9600);

    // Motor pin setup
    pinMode(pwmPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    pinMode(pwmPin2, OUTPUT);
    pinMode(dirPin2, OUTPUT);

    // Initialize PID controllers
    Setpoint = 0;
    Setpoint2 = 0;
    myPID.SetMode(AUTOMATIC);
    myPID.SetOutputLimits(-255, 255);
    myPID2.SetMode(AUTOMATIC);
    myPID2.SetOutputLimits(-255, 255);
}
double vel,vel2;
void loop() {
    unsigned long currentTime = millis();

    // Update speed every interval
    if (currentTime - prevTime >= speedInterval) {
        prevTime = currentTime;

        // Read encoder counts
        long currentEncoderCount = myEnc.read();
        long currentEncoderCount2 = myEnc2.read();

        // Calculate speeds in RPM
        actualSpeed = (currentEncoderCount - prevEncoderCount) * (60000.0 / speedInterval) / cpr;
        actualSpeed2 = (currentEncoderCount2 - prevEncoderCount2) * (60000.0 / speedInterval) / cpr;
        prevEncoderCount = currentEncoderCount;
        prevEncoderCount2 = currentEncoderCount2;

        // Update PID inputs and compute outputs
        Input = actualSpeed;
        Input2 = actualSpeed2;
        myPID.Compute();
        myPID2.Compute();

        // Control motors
        controlMotor(Output, dirPin, pwmPin);
        controlMotor(Output2, dirPin2, pwmPin2);

        // Publish speeds to ROS
         vel = (actualSpeed * 2 * M_PI * 0.065) / 60.0;  // Angular velocity (rad/s)
         vel2 = (actualSpeed2 * 2 * M_PI * 0.065) / 60.0;

        // Debugging
        Serial.print("Setpoint: ");
        Serial.print(Setpoint);
        Serial.print(" RPM | Actual: ");
        Serial.print(actualSpeed);
        Serial.print(" RPM | PWM: ");
        Serial.print(Output);
        Serial.print(" | Actual2: ");
        Serial.print(actualSpeed2);
        Serial.print(" RPM | PWM2: ");
        Serial.print(Output2);
        Serial.println();
    }
      publishSpeed(vel, vel2);

    nh.spinOnce();
    delay(10);
}

void publishSpeed(double vel, double vel2) {
    speed_msg.header.stamp = nh.now();
    speed_msg.vector.x = vel;
    speed_msg.vector.y = vel2;
    speed_msg.vector.z = 0;  // Placeholder for other data
    speed_pub.publish(&speed_msg);
}

void controlMotor(double pwmValue, int dirPin, int pwmPin) {
    if (pwmValue < 0) {
        digitalWrite(dirPin, LOW);  // Reverse direction
        analogWrite(pwmPin, -pwmValue);
    } else {
        digitalWrite(dirPin, HIGH); // Forward direction
        analogWrite(pwmPin, pwmValue);
    }
}
