/*
 * @Author: Sorzn
 * @Date: 2019-11-22 14:48:24
 * @LastEditTime: 2019-11-22 15:44:44
 * @Description: M5Stack project
 * @FilePath: /M5StickC/examples/Hat/JoyC/JoyC.cpp
 */

#include "JoyC.h"
#include "M5StickC.h"

JoyC::JoyC(StickSide side)
{
    _side=side;
}

void JoyC::SetLedColor(uint32_t color)
{
    uint8_t color_buff[3];
    color_buff[0] = (color & 0xff0000) >> 16;
    color_buff[1] = (color & 0xff00) >> 8;
    color_buff[2] = color & 0xff;
    M5.I2C.writeBytes(JOYC_ADDR, JOYC_COLOR_REG, color_buff, 3);
}

uint8_t JoyC::GetX(StickSide side)
{
    uint8_t read_reg;
    read_reg = (side == LeftJoy) ? JOYC_LEFT_X_REG : JOYC_RIGHT_X_REG;
    M5.I2C.readByte(JOYC_ADDR, read_reg, &_x);
    return _x;
}
uint8_t JoyC::GetX()
{
    return GetX(_side);
}

uint8_t JoyC::GetY(StickSide side)
{
    uint8_t read_reg;
    read_reg = (side == LeftJoy) ? JOYC_LEFT_Y_REG : JOYC_RIGHT_Y_REG;
    M5.I2C.readByte(JOYC_ADDR, read_reg, &_y);
    return _y;
}
uint8_t JoyC::GetY()
{
    return GetY(_side);
}

uint16_t JoyC::GetAngle(StickSide side)
{
    uint8_t i2c_read_buff[2];
    uint8_t read_reg;
    read_reg = (side == LeftJoy) ? JOYC_LEFT_ANGLE_REG : JOYC_RIGHT_ANGLE_REG;
    M5.I2C.readBytes(JOYC_ADDR, read_reg, 2, i2c_read_buff);
    _angle = (i2c_read_buff[0] << 8) | i2c_read_buff[1];
    return _angle;
}
uint16_t JoyC::GetAngle()
{
    return GetAngle(_side);
}

uint16_t JoyC::GetDistance(StickSide side)
{
    uint8_t i2c_read_buff[2];
    uint8_t read_reg;
    read_reg = (side == LeftJoy) ? JOYC_LEFT_DISTANCE_REG : JOYC_RIGHT_DISTANCE_REG;
    M5.I2C.readBytes(JOYC_ADDR, read_reg, 2, i2c_read_buff);
    _distance = (i2c_read_buff[0] << 8) | i2c_read_buff[1]; 
    return _distance;   
}
uint16_t JoyC::GetDistance()
{
    return GetDistance(_side);    
}

uint8_t JoyC::GetPress(StickSide side)
{
    uint8_t press_value = 0;
    uint8_t prevPress = 0;
    M5.I2C.readByte(JOYC_ADDR, JOYC_PRESS_REG, &press_value);
    prevPress = _pressed;
    _pressed = (press_value & ((side == LeftJoy) ? 0x10 : 0x01)) != 0;
    if(_pressed != prevPress )
    {
        _pressStateChanged = true;
    }else{
        _pressStateChanged = false;
    }
    return _pressed;
}
uint8_t JoyC::GetPress()
{
    return GetPress(_side);
}

void JoyC::ReadData()
{
    GetAngle();
    GetDistance();
    GetX();
    GetY();
    GetPress();
}

uint16_t JoyC::Angle(uint16_t range)
{
    if(AngleMax-AngleMin != 0)
    {
        return (_angle*range)/(AngleMax-AngleMin);
    }
    return _angle;
}
uint16_t JoyC::Angle()
{
    return _angle;
}

uint16_t JoyC::Distance(uint16_t range)
{
    if(DistanceMax-DistanceMin != 0)
    {
        return (_distance*range)/(DistanceMax-DistanceMin);
    }
    return _distance;
}
uint16_t JoyC::Distance()
{
    return _distance;
}
uint8_t JoyC::X(uint8_t range)
{
    if(XMax-XMin != 0)
    {
        return (_x*range)/(XMax-XMin);
    }
    return _x;
}
uint8_t JoyC::X()
{
    return _x;
}

uint8_t JoyC::Y(uint8_t range)
{
    if(YMax-YMin != 0)
    {
        return (_y*range)/(YMax-YMin);
    }
    return _y;
}
uint8_t JoyC::Y()
{
    return _y;
}
uint8_t JoyC::Press()
{
    return _pressed;    
}

bool JoyC::IsPressStateChanged()
{
    return _pressStateChanged;    
}


void JoyC::ResetRangeValue()
{
    AngleMin = 0xFFFF;
    AngleMax = 0x0000;
    DistanceMin = 0xFFFF;
    DistanceMax = 0x0000;
    XMin = 0xFF;
    XMax = 0x00;
    YMin = 0xFF;
    YMax = 0x00;
    
}

Direction JoyC::GetDirection()
{
    uint8_t thesY = 200*0.2;
    uint8_t midY = 200*0.5;
    uint8_t thesX = thesY;
    uint8_t midX = midY;

    if(_y > midY+thesY && _x < midX+thesX && _x > midX-thesX) 
    {
        return Forward;
    }
    else if(_y < midY-thesY && _x < midX+thesX && _x > midX-thesX)
    {
        return Backward;
    }
    else if(_x > midX+thesX && _y < midY+thesY && _y > midY-thesY)
    {
        return Left;
    }
    else if(_x < midX-thesX && _y < midY+thesY && _y > midY-thesY)
    {
        return Right;
    }
    else if(_y > midY+thesY && _x > midX+thesX)
    {
        return ForwardLeft;
    }
    else if(_y > midY+thesY && _x < midX-thesX)
    {
        return ForwardRight;

    }
    else if(_y < midY-thesY && _x > midX+thesX)
    {
        return BackwardLeft;
    }
    else if(_y < midY-thesY && _x < midX-thesX)
    {
        return BackwardRight;
    }
    else
    {
        return Neutral;
    }
    
}


void JoyC::UpdateValueRange()
{
    if(_angle < AngleMin )
    {
        AngleMin=_angle;
    }
    if(_angle > AngleMax )
    {
        AngleMax=_angle;
    }

    if(_distance < DistanceMin )
    {
        DistanceMin=_distance;
    }
    if(_distance > DistanceMax )
    {
        DistanceMax=_distance;
    }

    if(_x < XMin )
    {
        XMin=_x;
    }
    if(_x > XMax )
    {
        XMax=_x;
    }

    if(_y < YMin )
    {
        YMin=_y;
    }
    if(_y > YMax )
    {
        YMax=_y;
    }

}

