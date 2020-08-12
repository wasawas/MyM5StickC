
#include "RoverC.h"

void RoverC::Init(void)    //sda  0     scl  26
{
    Wire.begin(0, 26, 10000);
}

uint8_t RoverC::I2CWrite1Byte(uint8_t Addr, uint8_t Data)
{
    Wire.beginTransmission(ROVER_ADDRESS);
    Wire.write(Addr);
    Wire.write(Data);
    return Wire.endTransmission();
}

uint8_t RoverC::I2CWritebuff(uint8_t Addr, uint8_t *Data, uint16_t Length)
{
    Wire.beginTransmission(ROVER_ADDRESS);
    Wire.write(Addr);
    for (int i = 0; i < Length; i++)
    {
        Wire.write(Data[i]);
    }
    return Wire.endTransmission();
}

void RoverC::Send_iic(uint8_t Register, uint8_t Speed)
{
  Wire.beginTransmission(ROVER_ADDRESS);
  Wire.write(Register);
  Wire.write(Speed);
  _i2cState = Wire.endTransmission();
}

uint8_t RoverC::GetI2CState()
{
  return _i2cState;
}

int RoverC::GetLeftFront()
{
  return _leftFront;
}

int RoverC::GetLeftRear()
{
  return _leftRear;
}

int RoverC::GetRightFront()
{
  return _rightFront;
}

int RoverC::GetRightRear()
{
  return _rightRear;
}

void RoverC::Stop()
{
  Send_iic(0x00, 0 );
  Send_iic(0x01, 0 );
  Send_iic(0x02, 0 );
  Send_iic(0x03, 0 );
  
}

void RoverC::Go(Direction direction, uint16_t speed)
{
  switch(direction)
  {
    case Forward:
      _leftFront = speed;
      _leftRear = speed;
      _rightFront = speed;
      _rightRear = speed;
      break;
    case Backward:
      _leftFront = -speed;
      _leftRear = -speed;
      _rightFront = -speed;
      _rightRear = -speed;
      break;
    case Left:
      _leftFront = -speed;
      _leftRear = speed;
      _rightFront = speed;
      _rightRear = -speed;
      break;
    case Right:
      _leftFront = speed;
      _leftRear = -speed;
      _rightFront = -speed;
      _rightRear = speed;
      break;
    case RotateLeft:
      _leftFront = -speed;
      _leftRear = -speed;
      _rightFront = speed;
      _rightRear = speed;
      break;
    case RotateRight:
      _leftFront = speed;
      _leftRear = speed;
      _rightFront = -speed;
      _rightRear = -speed;
      break;
    case TurnLeft:
      _leftFront = speed/2;
      _leftRear = speed/2;
      _rightFront = speed;
      _rightRear = speed;
      break;
    case TurnRight:
      _leftFront = speed;
      _leftRear = speed;
      _rightFront = speed/2;
      _rightRear = speed/2;
      break;
    default:
      break;
  }

  // Left Front
  Send_iic(0x00, _leftFront );
  // Left Rear
  Send_iic(0x02, _leftRear);
  // Right Front
  Send_iic(0x01, _rightFront);
  // Right Rear
  Send_iic(0x03, _rightRear);

}

void RoverC::MoveMotor(int leftFront,int leftRear,int rightFront,int rightRear)
{
  _leftFront = leftFront;
  _leftRear = leftRear;
  _rightFront = rightFront;
  _rightRear = rightRear;


  if(_leftFront > 100){_leftFront=100;}
  if(_leftFront < -100){_leftFront=-100;}
  if(_leftRear > 100){_leftRear=100;}
  if(_leftRear < -100){_leftRear=-100;}
  if(_rightFront > 100){_rightFront=100;}
  if(_rightFront < -100){_rightFront=-100;}
  if(_rightRear > 100){_rightRear=100;}
  if(_rightRear < -100){_rightRear=-100;}

  // Left Front
  Send_iic(0x00, _leftFront );
  // Left Rear
  Send_iic(0x02, _leftRear);
 
  // Right Front
  Send_iic(0x01, _rightFront);
  // Right Rear
  Send_iic(0x03, _rightRear);

}


