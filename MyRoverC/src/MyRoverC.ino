#include <M5StickC.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "RoverC.h"

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

enum ModeType{
    NotAvailable = 0,
    JoyStickMode = 1,
    BallTrackingMode = 2
};


const char *ssid = "M5AP";
const char *password = "77777777";

TFT_eSprite Lcd = TFT_eSprite(&M5.Lcd);
WiFiServer server(80);

WiFiUDP Udp1;

RoverC rover =  RoverC();
uint32_t count = 0;
ModeType mode = JoyStickMode; 
bool isMenu = false;




void SetChargingCurrent(uint8_t CurrentLevel)
{
    Wire1.beginTransmission(0x34);
    Wire1.write(0x33);
    Wire1.write(0xC0 | (CurrentLevel & 0x0f));
    Wire1.endTransmission();
}


void setup()
{
    M5.begin();
    M5.update();
    rover.Init();

    uint64_t chipid = ESP.getEfuseMac();
    String str = ssid + String((uint32_t)(chipid >> 32), HEX);
    //M5.Lcd.setRotation(1);
    //M5.Lcd.setSwapBytes(false);
    //Disbuff.createSprite(160, 80);
    Lcd.createSprite(80, 160);
    //Disbuff.setSwapBytes(true);
    //Disbuff.fillRect(0, 0, 160, 20, Disbuff.color565(50, 50, 50));
    Lcd.setTextSize(1);
    Lcd.setTextColor(GREEN);
    Lcd.drawCentreString(str,40,6,1);
    Lcd.pushSprite(0, 0);

    SetChargingCurrent(4);

    Serial.begin(115200);
    //Set device in STA mode to begin with
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1),
                      IPAddress(192, 168, 4, 1),
                      IPAddress(255, 255, 255, 0));

    WiFi.softAP(str.c_str(), password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.begin();

    Udp1.begin(1003);
    //rover.RoverC_Init();
}




void loop()
{
    //rover.Move_forward(50);
    int udplength = Udp1.parsePacket();

    if (udplength )
    {
        JoyCommPacket packet;
        char udodata[udplength];
        Udp1.read(udodata, udplength);
        //Udp1.read((char *)&packet, sizeof(packet));

        memcpy(&packet,udodata,sizeof(packet));


        IPAddress udp_client = Udp1.remoteIP();
        if (packet.Header== 0xAA && packet.Marker == 0x55 && packet.End == 0xee)
        {
            memcpy(&packet,udodata,sizeof(packet));

            switch(packet.Mode)
            {
                case JoyStickMode:
                    rover.Move(packet.L_Angle,packet.L_Distance,packet.L_X,packet.L_Y,
                            packet.R_Angle,packet.R_Distance,packet.R_X,packet.R_Y);
                    ShowCommandInfo(packet);
                    break;
                case BallTrackingMode:

                    break;
                default:
                    break;
            }
        }
        
    }
    count++;
    if (count > 100)
    {
        count = 0;
        ShowBattery();
    }
}

void ShowCommandInfo(JoyCommPacket packet)
{        
    char text_buff[100];

    Lcd.fillRect( 0,20,80,114,BLACK);
    Lcd.drawCentreString("A", 40, 20, 1);
    Lcd.drawCentreString("D", 40, 34, 1);       
    Lcd.drawCentreString("X", 40, 48, 1);        
    Lcd.drawCentreString("Y", 40, 62, 1);
    Lcd.drawCentreString("P", 40, 76, 1);

    // Right Side Info
    sprintf(text_buff, "%d", packet.R_Angle);
    Lcd.drawRightString(text_buff, 80, 20, 1);
    sprintf(text_buff, "%d", packet.R_Distance);
    Lcd.drawRightString(text_buff, 80, 34, 1);       
    sprintf(text_buff, "%d", (int8_t)packet.R_X);
    Lcd.drawRightString(text_buff, 80, 48, 1);        
    sprintf(text_buff, "%d", (int8_t)packet.R_Y);
    Lcd.drawRightString(text_buff, 80, 62, 1);
    sprintf(text_buff, "%d", packet.R_Press);
    Lcd.drawRightString(text_buff, 80, 76, 1);

    // Left Side Info
    sprintf(text_buff, "%d", packet.L_Angle);
    Lcd.drawString(text_buff, 0, 20, 1);
    sprintf(text_buff, "%d", packet.L_Distance);
    Lcd.drawString(text_buff, 0, 34, 1);       
    sprintf(text_buff, "%d", (int8_t)packet.L_X);
    Lcd.drawString(text_buff, 0, 48, 1);        
    sprintf(text_buff, "%d", (int8_t)packet.L_Y);
    Lcd.drawString(text_buff, 0, 62, 1);
    sprintf(text_buff, "%d", packet.L_Press);
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

}

void ShowBattery()
{
    char text_buff[100];
    Lcd.fillRect(0, 134, 80, 26, Lcd.color565(50, 50, 50));
    sprintf(text_buff, "%d,%.2fV", rover.GetI2CState(),M5.Axp.GetBatVoltage());
    Lcd.drawCentreString(text_buff, 40, 138, 1);

    sprintf(text_buff, "%.2fmA", M5.Axp.GetBatCurrent());
    Lcd.drawCentreString(text_buff, 40, 150, 1);

    Lcd.pushSprite(0, 0);

}