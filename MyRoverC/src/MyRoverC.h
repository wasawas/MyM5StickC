#ifndef _MYROVERC_H_
#define _MYROVERC_H_

#include <M5StickC.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "RoverC.h"


#define SYSNUM 3

#define PKT_START 0xAA
#define PKT_M_JOYC 0x55
#define PKT_M_UNITV 0x66
#define PKT_END 0xEE
#define UDP_PORT 1003

typedef struct 
{
    uint8_t Header = 0;
    uint8_t Marker = 0;
    uint8_t Mode = 0;
    uint16_t L_Angle = 0;
    uint16_t L_Distance = 0;
    uint8_t L_X = 0;
    uint8_t L_Y = 0;
    uint8_t L_Press = 0;
    uint16_t R_Angle = 0;
    uint16_t R_Distance = 0;
    uint8_t R_X = 0;
    uint8_t R_Y = 0;
    uint8_t R_Press = 0;
	uint8_t End = 0;
} JoyCommPacket;

typedef struct
{
	const uint8_t Header = PKT_START;
	const uint8_t Marker = PKT_M_UNITV;
	int16_t x;
	int16_t y;
	int16_t w;
	int16_t h;
	int16_t cx;
	int16_t cy;
	uint32_t area;
	uint8_t state;
	const uint8_t End = PKT_END;

} UnitVPacket;

enum JoyState
{
    Release = 0,
    Pressed = 1
};


enum ModeType{
    NotAvailable = 0,
    JoyStickMode = 1,
    TrackingMode = 2
};

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    int16_t cx;
    int16_t cy;
    uint32_t area;
}v_response_t;

enum 
{
    kSeeking = 0,
    kSeekingL,
    kSeekingR,
    kRotateL,
    kRotateR,
    kLeft,
    kRight,
    kStraight,
    kTooClose
};


extern WiFiUDP Udp;
extern TFT_eSprite Lcd;
extern RoverC rover;
extern HardwareSerial VSerial;
extern String MySSID;
extern WiFiUDP Udp;
extern IPAddress myIP;
extern uint8_t cycle;

extern void MoveByColorTracking(uint8_t JoyPressed);
extern void ResetPacket(JoyCommPacket packetJoyC);
extern void ShowJoyStickMode(JoyCommPacket packetJoyC);
extern void WriteBattery();
extern void MoveByJoyStick(uint16_t angle_L,uint16_t distance_L,int8_t x_L,int8_t y_L,
                          uint16_t angle_R,uint16_t distance_R,int8_t x_R,int8_t y_R);

extern void ShowTrackingMode();












#endif