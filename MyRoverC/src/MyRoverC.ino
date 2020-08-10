#include <M5StickC.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "RoverC.h"

#define SYSNUM 3

#define PKT_START 0xAA
#define PKT_M_JOYC 0x55
#define PKT_M_UNITV 0x66
#define PKT_END 0xEE


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



enum ModeType{
    NotAvailable = 0,
    JoyStickMode = 1,
    BallTrackingMode = 2
};


const char *ssid = "M5AP";
const char *password = "77777777";

TFT_eSprite Lcd = TFT_eSprite(&M5.Lcd);
WiFiServer server(80);

WiFiUDP Udp;

RoverC rover =  RoverC();
uint32_t count = 0;
uint8_t Mode = JoyStickMode; 
bool isMenu = false;

IPAddress myIP;

HardwareSerial VSerial(1);

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
const uint16_t kThreshold = 30; // If target is in range ±kThreshold, the car will go straight
v_response_t v_data;    // Data read back from V
uint8_t state = 0;  // Car's movement status

uint8_t cycle = 0;

void SetChargingCurrent(uint8_t CurrentLevel)
{
    Wire1.beginTransmission(0x34);
    Wire1.write(0x33);
    Wire1.write(0xC0 | (CurrentLevel & 0x0f));
    Wire1.endTransmission();
}

String MySSID;

void setup()
{
    M5.begin();
    M5.update();
    rover.Init();
    VSerial.begin(115200, SERIAL_8N1, 33, 32);

    uint64_t chipid = ESP.getEfuseMac();
    MySSID = ssid + String((uint32_t)(chipid >> 32), HEX);
    Lcd.createSprite(80, 160);

    //M5.Lcd.setRotation(1);
    //M5.Lcd.setSwapBytes(false);
    //Disbuff.createSprite(160, 80);
    // Lcd.createSprite(80, 160);
    //Disbuff.setSwapBytes(true);
    //Disbuff.fillRect(0, 0, 160, 20, Disbuff.color565(50, 50, 50));
    // Lcd.setTextSize(1);
    // Lcd.setTextColor(GREEN);
    // Lcd.drawCentreString(str,40,6,1);
    // Lcd.pushSprite(0, 0);

    SetChargingCurrent(4);

    Serial.begin(115200);
    //Set device in STA mode to begin with
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1),
                      IPAddress(192, 168, 4, 1),
                      IPAddress(255, 255, 255, 0));

    WiFi.softAP(MySSID.c_str(), password);
    myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    //server.begin();

    Udp.begin(1003);
    //rover.RoverC_Init();
    

    ShowWaitingConnection();
}


int udplength =0;
bool bJoyReceived = false;

void loop()
{
    JoyCommPacket packetJoyC;
    ResetPacket(packetJoyC);
    bJoyReceived = false;
    //rover.Move_forward(50);
    udplength = Udp.parsePacket();

    if (udplength )
    {
        char udodata[udplength];
        Udp.read(udodata, udplength);
        //Udp1.read((char *)&packet, sizeof(packet));
        
        if( udplength == sizeof(packetJoyC))
        {
            memcpy(&packetJoyC,udodata,sizeof(packetJoyC));
            if (packetJoyC.Header== PKT_START && packetJoyC.Marker == PKT_M_JOYC && packetJoyC.End == PKT_END)
            {
                Mode = packetJoyC.Mode;
                bJoyReceived = true;
            }
        }
        

    }



    switch(Mode)
    {
        case JoyStickMode:
            rover.MoveByJoyStick(packetJoyC.L_Angle,packetJoyC.L_Distance,packetJoyC.L_X,packetJoyC.L_Y,
                    packetJoyC.R_Angle,packetJoyC.R_Distance,packetJoyC.R_X,packetJoyC.R_Y);
            if(bJoyReceived)
            {
                ShowCommandInfo(packetJoyC);
            }

            break;
        case BallTrackingMode:
            //Lcd.fillRect( 0,0,80,160,BLACK);

            BallTracking();
            break;
        default:
            break;
    }

    cycle++;
    if (cycle > 99)
    {
        cycle = 0;
    }

    
    //delay(50);
}

void ResetPacket(JoyCommPacket packetJoyC)
{
	packetJoyC.Mode = 0;
	packetJoyC.L_Angle =  0;
	packetJoyC.L_Distance = 0;
	packetJoyC.L_X = 0;
	packetJoyC.L_Y = 0;
	packetJoyC.L_Press = 0;
	packetJoyC.R_Angle = 0;
	packetJoyC.R_Distance = 0;
	packetJoyC.R_X = 0;
	packetJoyC.R_Y = 0;
	packetJoyC.R_Press = 0;

}


void ShowWaitingConnection()
{
    char text_buff[100];

    Lcd.setTextColor(WHITE);
    Lcd.drawCentreString(MySSID,40,6,1);

    Lcd.fillRect( 0,0,80,160,BLACK);


    sprintf(text_buff, "%s", MySSID);
    Lcd.drawCentreString(text_buff, 40, 6, 1);

    if(cycle / 33 == 1)
    {
        Lcd.drawString("Waiting.", 0, 20, 1);
    }
    else if(cycle / 33 == 2)
    {
        Lcd.drawString("Waiting..", 0, 20, 1);
    }
    else if(cycle / 33 == 3)
    {
        Lcd.drawString("Waiting...", 0, 20, 1);
    }
    Lcd.pushSprite(0, 0);
}

void ShowCommandInfo(JoyCommPacket packetJoyC)
{        
    char text_buff[100];

    //Lcd.setSwapBytes(true);
    //Lcd.fillRect(0, 0, 160, 20, Lcd.color565(50, 50, 50));
    Lcd.setTextColor(GREEN);
    Lcd.drawCentreString(MySSID,40,6,1);

    Lcd.fillRect( 0,0,80,160,BLACK);
    
    // sprintf(text_buff, "%d %d", udplength,sizeof(packetJoyC));
    // Lcd.drawString(text_buff, 0, 6, 1);

    sprintf(text_buff, "%s", MySSID);
    Lcd.drawCentreString(text_buff, 40, 6, 1);

    Lcd.drawCentreString("A", 40, 20, 1);
    Lcd.drawCentreString("D", 40, 34, 1);       
    Lcd.drawCentreString("X", 40, 48, 1);        
    Lcd.drawCentreString("Y", 40, 62, 1);
    Lcd.drawCentreString("P", 40, 76, 1);

    // Right Side Info
    sprintf(text_buff, "%d", packetJoyC.R_Angle);
    Lcd.drawRightString(text_buff, 80, 20, 1);
    sprintf(text_buff, "%d", packetJoyC.R_Distance);
    Lcd.drawRightString(text_buff, 80, 34, 1);       
    sprintf(text_buff, "%d", (int8_t)packetJoyC.R_X);
    Lcd.drawRightString(text_buff, 80, 48, 1);        
    sprintf(text_buff, "%d", (int8_t)packetJoyC.R_Y);
    Lcd.drawRightString(text_buff, 80, 62, 1);
    sprintf(text_buff, "%d", packetJoyC.R_Press);
    Lcd.drawRightString(text_buff, 80, 76, 1);

    // Left Side Info
    sprintf(text_buff, "%d", packetJoyC.L_Angle);
    Lcd.drawString(text_buff, 0, 20, 1);
    sprintf(text_buff, "%d", packetJoyC.L_Distance);
    Lcd.drawString(text_buff, 0, 34, 1);       
    sprintf(text_buff, "%d", (int8_t)packetJoyC.L_X);
    Lcd.drawString(text_buff, 0, 48, 1);        
    sprintf(text_buff, "%d", (int8_t)packetJoyC.L_Y);
    Lcd.drawString(text_buff, 0, 62, 1);
    sprintf(text_buff, "%d", packetJoyC.L_Press);
    Lcd.drawString(text_buff, 0, 76, 1);

    // Tire Speed
    sprintf(text_buff, "%d", rover.GetLeftFront());
    Lcd.drawString(text_buff, 0, 104, 1);
    sprintf(text_buff, "%d", rover.GetLeftRear());
    Lcd.drawString(text_buff, 0, 118, 1);       
    sprintf(text_buff, "%d", rover.GetRightFront());
    Lcd.drawRightString(text_buff, 80, 104, 1);        
    sprintf(text_buff, "%d", rover.GetRightRear());
    Lcd.drawRightString(text_buff, 80, 118, 1);

    WriteBattery();

    Lcd.pushSprite(0, 0);
}

void WriteBattery()
{
    char text_buff[100];
    Lcd.setTextColor(WHITE);

    Lcd.fillRect(0, 134, 80, 26, Lcd.color565(50, 50, 50));
    sprintf(text_buff, "%d,%.2fV", rover.GetI2CState(),M5.Axp.GetBatVoltage());
    Lcd.drawCentreString(text_buff, 40, 138, 1);

    sprintf(text_buff, "%.2fmA", M5.Axp.GetBatCurrent());
    Lcd.drawCentreString(text_buff, 40, 150, 1);


}
uint8_t PrevState = kSeekingL;
uint8_t PrevSeeking = kSeekingR;
uint8_t PrevCX = 0;
void BallTracking()
{
    char text_buff[100];
    String strState;
    Lcd.setTextColor(BLACK);
    bool bFound = false;
    VSerial.write(0xAF);

    v_data.x = 0;
    v_data.y = 0;
    v_data.w = 0;
    v_data.h = 0;
    v_data.cx = 0;
    v_data.cy = 0;
    v_data.area = 0;

    delay(50);
    rover.Stop();
        


    if(VSerial.available())
    {
        uint8_t buffer[15];
        int16_t width = 320;
        int16_t height = 240;
        VSerial.readBytes(buffer, 15);
        v_data.x = (buffer[0] << 8) | buffer[1];
        v_data.y = (buffer[2] << 8) | buffer[3];
        v_data.w = (buffer[4] << 8) | buffer[5];
        v_data.h = (buffer[6] << 8) | buffer[7];
        v_data.cx = (buffer[8] << 8) | buffer[9];
        v_data.cy = (buffer[10] << 8) | buffer[11];
        v_data.area = (buffer[12] << 16) | (buffer[13] << 8) | buffer[14];

        String prevStateStr = "";
        switch(PrevState)
        {
            case kSeekingL:prevStateStr="Seek-L";break;
            case kSeekingR:prevStateStr="Seek-R";break;
            case kLeft:prevStateStr="Left";break;
            case kRight:prevStateStr="Right";break;
            case kRotateL:prevStateStr="RotateL";break;
            case kRotateR:prevStateStr="RotateR";break;
            case kStraight:prevStateStr="Straight";break;
            case kTooClose:prevStateStr="Stop";break;
            default:break;
        }



        if(v_data.cx > 0 && v_data.cx < width && v_data.area>150 && v_data.area<30000)
        {
            
            bFound = true;            
            if(v_data.area > 20000)
            {
                state = kTooClose;  // Stop
                strState = "Stop";
            }
            else if(v_data.cx > 160-kThreshold && v_data.x < 160+kThreshold)
            {
                state = kStraight;  // Go straight
                strState = "Forward";
            }
            else if(v_data.cx <= 160-kThreshold-30)
            {
                state = kSeekingL; // Rotate left
                strState = "Rotate-L";
            }
            else if(v_data.cx >= 160+kThreshold+30)
            {
                state = kRotateR;  // Rotate right
                strState = "Rotate-R";
            }
            else if(v_data.cx <= 160-kThreshold)
            {
                state = kLeft; // Go left
                strState = "Left";
            }
            else if(v_data.cx >= 160+kThreshold)
            {
                state = kRight;  // Go right
                strState = "Right";
            }
            else
            {
                state = kSeeking;
            }
            Lcd.fillScreen(GREEN);
        }
        else
        {
            state = kSeeking;
            Lcd.fillScreen(RED);            
        }
        if(state==kSeeking)
        {
            if(PrevCX>160 && PrevCX<=320 && PrevState!=kSeekingL && PrevState!=kSeekingR)
            {
                state = kSeekingR;  // Rotate
                strState = "Seek1-R";
                PrevState = state;
            }
            else if(PrevCX<160 && PrevCX>0 && PrevState!=kSeekingL && PrevState!=kSeekingR)
            {
                state = kSeekingL;  // Rotate
                strState = "Seek1-L";
                PrevState = state;
            }
            else if(PrevState==kSeekingL)
            {
                state = kSeekingL;  // Rotate
                strState = "Seek2-L";
            }
            else if(PrevState==kSeekingR)
            {
                state = kSeekingR;  // Rotate
                strState = "Seek2-R";
            }
            
        }

        Serial.printf("%d, %d, %d\n", v_data.x, v_data.area, state);

        sprintf(text_buff, "x:%d", v_data.x);
        Lcd.drawString(text_buff, 0, 6, 1);
        sprintf(text_buff, "y:%d", v_data.y);
        Lcd.drawString(text_buff, 0, 20, 1);
        sprintf(text_buff, "w:%d", v_data.w);
        Lcd.drawString(text_buff, 0, 34, 1);
        sprintf(text_buff, "h:%d", v_data.h);
        Lcd.drawString(text_buff, 0, 48, 1);
        sprintf(text_buff, "cx:%d", v_data.cx);
        Lcd.drawString(text_buff, 0, 62, 1);
        sprintf(text_buff, "cy:%d", v_data.cy);
        Lcd.drawString(text_buff, 0, 76, 1);

        sprintf(text_buff, "ar:%d", v_data.area);
        Lcd.drawString(text_buff, 0, 90, 1);
        sprintf(text_buff, "st:%s", strState.c_str());
        Lcd.drawString(text_buff, 0, 104, 1);


        sprintf(text_buff, "pst:%s", prevStateStr.c_str());
        Lcd.drawString(text_buff, 0, 118, 1);

        sprintf(text_buff, "found:%d", bFound);
        Lcd.drawString(text_buff, 0, 132, 1);

        sprintf(text_buff, "prevCX:%d", PrevCX);
        Lcd.drawString(text_buff, 0, 146, 1);

        UnitVPacket packetUnitV;

        packetUnitV.x = v_data.x;
        packetUnitV.y = v_data.y;
        packetUnitV.w = v_data.w;
        packetUnitV.h = v_data.h;
        packetUnitV.cx = v_data.cx;
        packetUnitV.cy = v_data.cy;
        packetUnitV.area = v_data.area;
        packetUnitV.state = state;
        

        IPAddress udp_client = Udp.remoteIP();
        //IPAddress udp_client = IPAddress(192, 168, 4, 100 + SYSNUM);
        Udp.beginPacket(udp_client, 1003);
        Udp.write((uint8_t *)&packetUnitV, sizeof(packetUnitV));
        Udp.endPacket();

        PrevState = state;
        PrevCX = v_data.cx;

        Lcd.pushSprite(0, 0);


        uint8_t speed = 20;
        //The speed and time here may need to be modified according to the actual situation
        switch(state)
        {
            case kSeekingL:
                rover.Go(RotateLeft,speed);
                break;
            case kSeekingR:
                rover.Go(RotateRight,speed);
                break;

            case kLeft:
                rover.Go(Left,speed);                
                break;

            case kRight:
                rover.Go(Right,speed);
                break;

            case kRotateL:
                rover.Go(RotateLeft,speed);                
                break;

            case kRotateR:
                rover.Go(RotateRight,speed);
                break;

            case kStraight:
                rover.Go(Forward,speed);
                
                break;

            case kTooClose:
                rover.Stop();
                break;
            default:
                rover.Go(RotateLeft,speed);
                break;
        }
        
    }


}