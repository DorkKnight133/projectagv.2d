#include <Wire.h>
#include <ZumoShield.h>
#define TURN_BASE_SPEED 75 // Base speed when turning (added to variable speed)

#define sensorIR1 A0 // Sharp IR GP2Y0A41SK0F (4-30cm, analog)
#define sensorIR2 A1 // Sharp IR GP2Y0A41SK0F (4-30cm, analog)
#define witteKnop 4
#define rodeKnop 5

#define CALIBRATION_SAMPLES 100  // Number of compass readings to take when calibrating
#define CRB_REG_M_2_5GAUSS 0x60 // CRB_REG_M value for magnetometer +/-2.5 gauss full scale
#define CRA_REG_M_220HZ    0x1C // CRA_REG_M value for magnetometer 220 Hz update rate

// Allowed deviation (in degrees) relative to target angle that must be achieved before driving straight
#define DEVIATION_THRESHOLD 1


LSM303 compass;
ZumoMotors motors;

// defines pins numbers
const int trigPin = 6;
const int echoPin1 = 11;
// defines variables
long durationUS1;
int distanceUS1;
const int echoPin1 = 12;
// defines variables
long durationUS2;
int distanceUS2;
const int echoPin1 = 13;
// defines variables
long durationUS3;
int distanceUS3;
int SPEED=100;
int actie=4;

void setup() {
pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
pinMode(echoPin1, INPUT); // Sets the echoPin as an Input
pinMode(echoPin2, INPUT); // Sets the echoPin as an Input
pinMode(echoPin3, INPUT); // Sets the echoPin as an Input
Serial.begin(9600); // Starts the serial communication 

 // The highest possible magnetic value to read in any direction is 2047
  // The lowest possible magnetic value to read in any direction is -2047
  LSM303::vector<int16_t> running_min = {32767, 32767, 32767}, running_max = {-32767, -32767, -32767};
  unsigned char index;

  Serial.begin(9600);

  // Initiate the Wire library and join the I2C bus as a master
  Wire.begin();

  // Initiate LSM303
  compass.init();

  // Enables accelerometer and magnetometer
  compass.enableDefault();

  compass.writeReg(LSM303::CRB_REG_M, CRB_REG_M_2_5GAUSS); // +/- 2.5 gauss sensitivity to hopefully avoid overflow problems
  compass.writeReg(LSM303::CRA_REG_M, CRA_REG_M_220HZ);    // 220 Hz compass update rate


  Serial.println("starting calibration");

  // To calibrate the magnetometer, the Zumo spins to find the max/min
  // magnetic vectors. This information is used to correct for offsets
  // in the magnetometer data.
  motors.setLeftSpeed(SPEED);
  motors.setRightSpeed(-SPEED);

  for(index = 0; index < CALIBRATION_SAMPLES; index ++)
  {
    // Take a reading of the magnetic vector and store it in compass.m
    compass.read();

    running_min.x = min(running_min.x, compass.m.x);
    running_min.y = min(running_min.y, compass.m.y);

    running_max.x = max(running_max.x, compass.m.x);
    running_max.y = max(running_max.y, compass.m.y);

    Serial.println(index);

    delay(50);
  }

  motors.setLeftSpeed(0);
  motors.setRightSpeed(0);

  Serial.print("max.x   ");
  Serial.print(running_max.x);
  Serial.println();
  Serial.print("max.y   ");
  Serial.print(running_max.y);
  Serial.println();
  Serial.print("min.x   ");
  Serial.print(running_min.x);
  Serial.println();
  Serial.print("min.y   ");
  Serial.print(running_min.y);
  Serial.println();

  // Set calibrated values to compass.m_max and compass.m_min
  compass.m_max.x = running_max.x;
  compass.m_max.y = running_max.y;
  compass.m_min.x = running_min.x;
  compass.m_min.y = running_min.y;

}
void loop() {
// Clears the trigPin
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
// Sets the trigPin on HIGH state for 10 micro seconds
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
// Reads the echoPin, returns the sound wave travel time in microseconds
durationUS1 = pulseIn(echoPin1, HIGH);
// Calculating the distanceUS1
distanceUS1= durationUS1*0.034/2;

// Clears the trigPin
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
// Sets the trigPin on HIGH state for 10 micro seconds
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
// Reads the echoPin, returns the sound wave travel time in microseconds
durationUS2 = pulseIn(echoPin2, HIGH);
// Calculating the distanceUS1
distanceUS2= durationUS2*0.034/2;

// Clears the trigPin
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
// Sets the trigPin on HIGH state for 10 micro seconds
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
// Reads the echoPin, returns the sound wave travel time in microseconds
durationUS3 = pulseIn(echoPin3, HIGH);
// Calculating the distanceUS1
distanceUS3= durationUS3*0.034/2;



float volts1 = analogRead(sensorIR1)*0.0048828125;  // value from sensor * (5/1024)
  int distanceIR1 = 13*pow(volts1, -1); // worked out from datasheet graph

float volts2 = analogRead(sensorIR2)*0.0048828125;  // value from sensor * (5/1024)
  int distanceIR2 = 13*pow(volts2, -1); // worked out from datasheet graph
  
  
if (rodeKnop==1){
  actie=0;
}else{
  if (witteKnop==1){
    //volgen
  }else{
    if(distanceUS3<20){
      actie=4;
    }else{
      if(distanceUS2<10||distanceUS1<10){
        actie=0;
      }else{
        if(distanceIR1<distanceIR2){
          actie=2;
        }else{
          if(distanceIR1>distanceIR2){
            actie=3;
          }else{
            actie=1;
          }
        }
      }
    }
  }
}
     
switch(actie){
  case 0: //stoppen
  motors.setSpeeds(0, 0);
  break;
  case 1: //vooruit
  motors.setSpeeds(SPEED, SPEED);
  break;
  case 2: //flauw naar rechts
  motors.setSpeeds(SPEED-25, SPEED);
  break;
  case 3: //flauw naar links
  motors.setSpeeds(SPEED, SPEED-25);
  break;
  case 4: //90 graden
    float heading, relative_heading;
  int speed;
  static float target_heading = averageHeading();

  // Heading is given in degrees away from the magnetic vector, increasing clockwise
  heading = averageHeading();

  // This gives us the relative heading with respect to the target angle
  relative_heading = relativeHeading(heading, target_heading);

  Serial.print("Target heading: ");
  Serial.print(target_heading);
  Serial.print("    Actual heading: ");
  Serial.print(heading);
  Serial.print("    Difference: ");
  Serial.print(relative_heading);

  // If the Zumo has turned to the direction it wants to be pointing, go straight and then do another turn
  if(abs(relative_heading) < DEVIATION_THRESHOLD)
  {
    motors.setSpeeds(SPEED, SPEED);

    Serial.print("   Straight");

    delay(1000);

    // Turn off motors and wait a short time to reduce interference from motors
    motors.setSpeeds(0, 0);
    delay(100);

    // Turn 90 degrees relative to the direction we are pointing.
    // This will help account for variable magnetic field, as opposed
    // to using fixed increments of 90 degrees from the initial
    // heading (which might have been measured in a different magnetic
    // field than the one the Zumo is experiencing now).
    // Note: fmod() is floating point modulo
    //button.waitForButton();
    target_heading = fmod(averageHeading() + 90, 360);
  }
  else
  {
    // To avoid overshooting, the closer the Zumo gets to the target
    // heading, the slower it should turn. Set the motor speeds to a
    // minimum base amount plus an additional variable amount based
    // on the heading difference.

    speed = SPEED*relative_heading/750;

    if (speed < 0)
      speed -= TURN_BASE_SPEED;
    else
      speed += TURN_BASE_SPEED;

    motors.setSpeeds(speed, -speed);

    Serial.print("   Turn");
  }
  Serial.println();
  break;
  

}

}



// Converts x and y components of a vector to a heading in degrees.
// This function is used instead of LSM303::heading() because we don't
// want the acceleration of the Zumo to factor spuriously into the
// tilt compensation that LSM303::heading() performs. This calculation
// assumes that the Zumo is always level.
template <typename T> float heading(LSM303::vector<T> v)
{
  float x_scaled =  2.0*(float)(v.x - compass.m_min.x) / ( compass.m_max.x - compass.m_min.x) - 1.0;
  float y_scaled =  2.0*(float)(v.y - compass.m_min.y) / (compass.m_max.y - compass.m_min.y) - 1.0;

  float angle = atan2(y_scaled, x_scaled)*180 / M_PI;
  if (angle < 0)
    angle += 360;
  return angle;
}

// Yields the angle difference in degrees between two headings
float relativeHeading(float heading_from, float heading_to)
{
  float relative_heading = heading_to - heading_from;

  // constrain to -180 to 180 degree range
  if (relative_heading > 180)
    relative_heading -= 360;
  if (relative_heading < -180)
    relative_heading += 360;

  return relative_heading;
}

// Average 10 vectors to get a better measurement and help smooth out
// the motors' magnetic interference.
float averageHeading()
{
  LSM303::vector<int32_t> avg = {0, 0, 0};

  for(int i = 0; i < 10; i ++)
  {
    compass.read();
    avg.x += compass.m.x;
    avg.y += compass.m.y;
  }
  avg.x /= 10.0;
  avg.y /= 10.0;

  // avg is the average measure of the magnetic vector.
  return heading(avg);
}
