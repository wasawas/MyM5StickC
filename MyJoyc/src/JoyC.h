/*
 * @Author: Sorzn
 * @Date: 2019-11-22 14:48:31
 * @LastEditTime: 2019-11-22 15:22:45
 * @Description: M5Stack HAT JOYC project
 * @FilePath: /M5StickC/examples/Hat/JoyC/JoyC.h
 */

#ifndef _HAT_JOYC_H_
#define _HAT_JOYC_H_

#include "Arduino.h"

#define JOYC_ADDR 0x38

#define JOYC_COLOR_REG          0x20
#define JOYC_LEFT_X_REG         0x60
#define JOYC_LEFT_Y_REG         0x61
#define JOYC_RIGHT_X_REG        0x62
#define JOYC_RIGHT_Y_REG        0x63
#define JOYC_PRESS_REG          0x64
#define JOYC_LEFT_ANGLE_REG     0x70
#define JOYC_LEFT_DISTANCE_REG  0x74
#define JOYC_RIGHT_ANGLE_REG    0x72
#define JOYC_RIGHT_DISTANCE_REG 0x76

enum StickSide
{
    Left = 0,
    Right = 1
};

enum JoyState
{
    Release = 0,
    Pressed = 1
};

class JoyC
{
    public:
        JoyC(StickSide side);

        /**
         * @description: 
         * @param color: (r << 16) | (g << 8) | b 
         * @return: 
         */        
        void SetLedColor(uint32_t color);

        /**
         * @description: 
         * @param side: 0: left, 1: right 
         * @return: x value: 0 ~ 200
         */        
        uint8_t GetX(StickSide side);
        uint8_t GetX();
        
        /**
         * @description: 
         * @param side: 0: left, 1: right 
         * @return: y value: 0 ~ 200
         */       
        uint8_t GetY(StickSide side);
        uint8_t GetY();
        
        /**
         * @description: 
         * @param side: 0: left, 1: right 
         * @return: angle value: 0 ~ 360
         */        
        uint16_t GetAngle(StickSide side);
        uint16_t GetAngle();

        /**
         * @description: 
         * @param side: 0: left, 1: right 
         * @return: 
         */        
        uint16_t GetDistance(StickSide side);
        uint16_t GetDistance();
        
        /**
         * @description: 
         * @param side: 0: left, 1: right 
         * @return: 0 or 1
         */        
        uint8_t GetPress(StickSide side);
        uint8_t GetPress();

        /**
         * @description: get all joy stick information and set to private member
         */
        void ReadData();

        uint16_t Angle(uint16_t range);
        uint16_t Distance(uint16_t range);
        uint8_t X(uint8_t range);
        uint8_t Y(uint8_t range);
        uint8_t Press();

        uint16_t Angle();
        uint16_t Distance();
        uint8_t X();
        uint8_t Y();


        bool IsPressStateChanged();
        void ResetRangeValue();
        void UpdateValueRange();
        

        uint16_t AngleMax = 360;
        uint16_t AngleMin = 0;
        uint16_t DistanceMax = 100;
        uint16_t DistanceMin = 0;
        uint8_t XMax = 200;
        uint8_t XMin = 0;
        uint8_t YMax = 200;
        uint8_t YMin = 0;


    private:
        /**
         * @description: Joy side
         */        
        StickSide _side = Left;

        uint16_t _angle = 0;
        uint16_t _distance = 0;
        uint8_t _x = 0;
        uint8_t _y = 0;
        uint8_t _pressed = 0;
        bool _pressStateChanged = false;


};

#endif