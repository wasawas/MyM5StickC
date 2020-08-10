#ifndef _ROVERC_H_
#define _ROVERC_H_

#include <M5StickC.h>

#define ROVER_ADDRESS	0X38

enum Direction{
         Forward = 1,
         Backward = 2,
         Left = 3,
         Right = 4,
         RotateLeft =5,
         RotateRight = 6,
         TurnLeft =7,
         TurnRight=8
};

class RoverC
{
    public:

        void Init(void);	//sda  0     scl  26
        
        uint8_t I2CWrite1Byte(uint8_t Addr, uint8_t Data);
        uint8_t I2CWritebuff(uint8_t Addr, uint8_t *Data, uint16_t Length);
        void Send_iic(uint8_t Register, uint8_t Speed);
        uint8_t GetI2CState();
        int GetLeftFront();
        int GetLeftRear();
        int GetRightFront();
        int GetRightRear();
        void Stop();

        void Go(Direction direction, uint16_t speed);

        void MoveByJoyStick(uint16_t angle_L,uint16_t distance_L,int8_t x_L,int8_t y_L,
                          uint16_t angle_R,uint16_t distance_R,int8_t x_R,int8_t y_R);

    private:
        uint8_t _i2cState = I2C_ERROR_NO_BEGIN;
        int _leftFront = 0;
        int _leftRear = 0;
        int _rightFront = 0;
        int _rightRear = 0;

};

#endif
