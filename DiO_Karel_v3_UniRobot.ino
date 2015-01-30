/*----------------------------------------------------------------------------\
|  FILE       : Bulgar_0_0_0    |  This class is dedecated to convert         |
|  DEVELOPER  : DiO             |  a MONO8 byte buffers, to Bitmap image.     |
|  STAND      : See History     |                                             |
|  VERSION    : 1.00            |                                             |
|-----------------------------------------------------------------------------|
|                                                        | Copyright:         |
|                     Bulgar_0_0_1.ino                   |   GPL              |
|                                                        |                    |
|-----------------------------------------------------------------------------|
|                              H I S T O R Y                                  |
|                                                                             |
| 140821 DiO  Creation of the file                                            |
| 140821 DiO  Edit and make doc. Update modifiers.                            |
\----------------------------------------------------------------------------*/

#include <AFMotor.h>
#include <String.h>
#include <stdlib.h>
#include <Wire.h>

//
#define LEFT_MOTOR 3
#define RIGHT_MOTOR 2

//////////////////////////////////////////////////////////////////////////
// Sensor board definitions.
//////////////////////////////////////////////////////////////////////////
#define EXPNDER_ADDRESS   0x20 //56 //B0100000, 0x20
#define OPTO_SENSORS      0x10
#define GAS_SENSORS       0x20
#define DISTANCE_SENSORS  0x40
#define FREE_PIN          0x80
#define ANALOG_IN         0x03

//
String IncommingCommnad = "";

//
boolean Echo = true;

//
AF_DCMotor LeftMotor(LEFT_MOTOR);
AF_DCMotor RightMotor(RIGHT_MOTOR);

//
int sensorValues[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void setup()
{
  // Switch off the motors.
  LeftMotor.run(RELEASE);
  RightMotor.run(RELEASE);
  LeftMotor.setSpeed(0);
  RightMotor.setSpeed(0);
  
  // Join I2C BUS.
  Wire.begin();        
  
  // Setup Serial library at 9600 bps.
  Serial.begin(9600);
  ShowVersion();
}

void loop()
{
  //motor.run(FORWARD);
  //motor.run(BACKWARD);
  //motor.run(RELEASE);
  //motor.setSpeed(i);  

  ReadCommand();
}

//////////////////////////////////////////////////////////////////////////  
// Read incomming data from the serial buffer.
//////////////////////////////////////////////////////////////////////////  
void ReadCommand()
{
  // Fill the command data buffer with command.
  while(Serial.available())
  {
    // Add new char.
    IncommingCommnad += (char)Serial.read();
    // Wait a while for a a new char.
    delay(5);
  }
  
  // If command if not empty parse it.
  if(IncommingCommnad != "")
  {
    boolean isValid = ValidateCommand(IncommingCommnad);
    if(isValid)
    {
      ParseCommand(IncommingCommnad);
    }
    // Print command for feedback.
    if(Echo == true)
    {
      Serial.print("Cmd: ");
      Serial.println(IncommingCommnad);
    }
  }
  
  // Clear the command data buffer.
  IncommingCommnad = ""; 
}

//////////////////////////////////////////////////////////////////////////  
// Validate the incomming data from the serial buffer.
//////////////////////////////////////////////////////////////////////////  
boolean ValidateCommand(String command)
{
  // ?LF255RB255\n
  // |||||||||||||
  // |||||||||||\\- Termin
  // ||||||||\\\--- Right PWM value.
  // |||||||\------ Direction of the right bridge.
  // ||||||\------- Right bridge index.
  // |||\\\-------- Left PWM value.
  // ||\----------- Direction of the left bridge.
  // |\------------ Left bridge index.
  // \------------- Start symbol.
  //
  // Package lenght 1 + 1 + 1 + 3 + 1 + 1 + 3 + 1 = 12 bytes;
  //
  
  boolean state = false;
  
  if(command[0] == '?' && command[11] == '\n')
  {
    if(command[1] == 'L' && command[6] == 'R')
    {
      if((command[2] == 'F' || command[2] == 'B') && (command[7] == 'F' || command[7] == 'B'))
      {
        // PWM values.
        int leftWheelPWM = -1;
        int rightWheelPWM = -1;
        
        // Convert commands from string to numbers.
        leftWheelPWM = atoi(command.substring(3, 6).c_str());
        rightWheelPWM = atoi(command.substring(8, 11).c_str());
        
        //
        if((leftWheelPWM < 256 && leftWheelPWM > -1) && (rightWheelPWM < 256 && rightWheelPWM > -1))
        {
          // If is valid.
          state = true;
        }
      }        
    }
  }
  
  // Input sensor request.
  if(command == "?SENSORS\n")
  {
    // If is valid.
    state = true;
  }
  
  // Show version of the device.
  if(command == "?VERSION\n")
  {
    // If is valid.
    state = true;
  }
  
  // If is not valid.
  return state;
}

//////////////////////////////////////////////////////////////////////////  
// Parse the incomming data from the serial buffer.
//////////////////////////////////////////////////////////////////////////  
void ParseCommand(String command)
{
  //?LF255RF255\n - Motor controll.
  //?SENSORS\n    - Sensor request.
  
  if(command[1] == 'L' && command[6] == 'R')
  {
    // Left motor direction block.
    if(command[2] == 'F')
    {
      LeftMotor.run(FORWARD);
    }
    else if(command[2] == 'B')
    {
      LeftMotor.run(BACKWARD);
    }
    else
    {
      LeftMotor.run(RELEASE);
    }
  
    // Right motor direction block.
    if(command[7] == 'F')
    {
      RightMotor.run(FORWARD);
    }
    else if(command[7] == 'B')
    {
      RightMotor.run(BACKWARD);
    }
    else
    {
      RightMotor.run(RELEASE);
    }
    
    // 
    int leftWheelPWM = -1;
    int rightWheelPWM = -1;
  
    //
    leftWheelPWM = atoi(command.substring(3, 6).c_str());
    rightWheelPWM = atoi(command.substring(8, 11).c_str());
      
    LeftMotor.setSpeed(leftWheelPWM);
    RightMotor.setSpeed(rightWheelPWM);
    
    Serial.print("Left motor speed: ");
    Serial.println(leftWheelPWM, DEC);
    Serial.print("Left motor direction: ");
    Serial.println(command[2] == 'F' ? "Forward" : "Backward");
    Serial.print("Right motor speed: ");
    Serial.println(rightWheelPWM, DEC);
    Serial.print("Right motor direction: ");
    Serial.println(command[7] == 'F' ? "Forward" : "Backward");
    
  }
  else if(command == "?SENSORS\n")
  {
    ReadSensoers();
    Serial.print("Sensors data: ");
    for (char index = 0; index < 16; index++)
    {
      Serial.print(sensorValues[index], DEC);
      //Serial.print(analogRead(ANALOG_IN), DEC);
      if(index < 15)
      {
        Serial.print(", ");
      }
    }
    Serial.println();
  }
  else if(command == "?VERSION\n")
  {
    // Show version of the device.
    ShowVersion();
  }
}

//////////////////////////////////////////////////////////////////////////
// Read sensor value.
//////////////////////////////////////////////////////////////////////////
int ReadSensor(byte chanel)
{
  if((chanel >= 0) && (chanel <= 15))
  {
    int data = 0;
    Wire.beginTransmission(EXPNDER_ADDRESS);
    Wire.write((OPTO_SENSORS + chanel));
    Wire.endTransmission();
    for (char i = 0; i < 5; i++)
    {
      data += analogRead(ANALOG_IN);
      delayMicroseconds(5);
    }
    data /= 5;
    return data;
  }
  else
  {
    return -1;
  }
}

//////////////////////////////////////////////////////////////////////////
// Fill the real sensor array.
//////////////////////////////////////////////////////////////////////////
void ReadSensoers()
{
  int i;
  for(i = 0; i < 16; i++)
  {
    sensorValues[i] = ReadSensor(i);
  }
}

//////////////////////////////////////////////////////////////////////////
// Print the version of the device.
//////////////////////////////////////////////////////////////////////////
void ShowVersion()
{
  Serial.println("UNI : TU-Gabrovo");
  Serial.println("DEP : AIUT");
  Serial.println("NAME: Robot Karel v3.2");
  Serial.println("DATE: 23.10.2014y."); 
}
