
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

void RoverC::Move(uint16_t angle_L,uint16_t distance_L,int8_t x_L,int8_t y_L,
                          uint16_t angle_R,uint16_t distance_R,int8_t x_R,int8_t y_R)
{
  int16_t moving = 0;
  int16_t steering = 0;
  if(distance_L==0 && distance_R==0)
  {
    Stop();
    return;
  }


  if( ((y_R>0 && y_L<0) || (y_R<0 && y_L>0)) && (x_R==0 && x_L==0)){
    // Rotate Mode (By using both stick on y axle)
    _leftFront = y_L ;
    _leftRear = y_L ;
    _rightFront = y_R;
    _rightRear = y_R;
  }else if(x_L==0 && y_L!=0 ){
    // Forward-Backward Mode
    moving = (y_L);
    steering = (y_L*x_R)/100;
    
    _leftFront = moving - steering;
    _leftRear = moving - steering;
    _rightFront = moving + steering;
    _rightRear = moving + steering;
  
  }else if(x_L!=0){
    // Left-Right Mode
    _leftFront = -x_L-x_R;
    _leftRear = x_L-x_R;
    _rightFront = x_L;
    _rightRear = -x_L;

  }else if(y_L==0 && y_R==0){  
    // Rotate Mode (By right stick on x axle)
    _leftFront = -x_R/2 ;
    _leftRear = -x_R/2 ;
    _rightFront = x_R/2;
    _rightRear = x_R/2;
    
  }


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

