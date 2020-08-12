#include "MyRoverC.h"

String MySSID;
WiFiUDP Udp;
IPAddress myIP;


const char *ssid = "M5AP";
const char *password = "77777777";
TFT_eSprite Lcd = TFT_eSprite(&M5.Lcd);
WiFiServer server(80);
uint32_t count = 0;
uint8_t Mode = JoyStickMode; 
bool isMenu = false;
int udplength =0;
bool bJoyReceived = false;
uint8_t cycle = 0;
uint8_t disconnectCycle = 0;

RoverC rover =  RoverC();



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
    
    Mode = NotAvailable;

    
}

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

        disconnectCycle = 0;

    }else if((disconnectCycle%20)==0 && Mode==JoyStickMode){
        
        disconnectCycle++;
        if (disconnectCycle > 5)
        {
            disconnectCycle = 0;
            Mode = NotAvailable;
        }
    }



    switch(Mode)
    {
        case JoyStickMode:
            MoveByJoyStick(packetJoyC.L_Angle,packetJoyC.L_Distance,packetJoyC.L_X,packetJoyC.L_Y,
                    packetJoyC.R_Angle,packetJoyC.R_Distance,packetJoyC.R_X,packetJoyC.R_Y);
            if(bJoyReceived)
            {
                ShowJoyStickMode(packetJoyC);
            }

            break;
        case TrackingMode:
            
            MoveByColorTracking(packetJoyC.R_Press);
            ShowTrackingMode();
            break;
        case NotAvailable:
        default:
            ShowWaitingConnection();
            break;
    }

    cycle++;
    if (cycle > 79)
    {
        cycle = 0;
    }

    
    //delay(50);
}

void SetChargingCurrent(uint8_t CurrentLevel)
{
    Wire1.beginTransmission(0x34);
    Wire1.write(0x33);
    Wire1.write(0xC0 | (CurrentLevel & 0x0f));
    Wire1.endTransmission();
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

    Lcd.deleteSprite();
	M5.Lcd.setRotation(1);
	Lcd.createSprite(160, 80);

    Lcd.fillRect(0, 0, 160, 20, DARKGREY);
    Lcd.setTextColor(WHITE);
    Lcd.setTextSize(1);
    Lcd.drawString("Waiting for Wifi SSID:",5,6,1);

    Lcd.setTextSize(2);
    Lcd.drawCentreString(MySSID,80,25,2);

    //Lcd.fillRect(v_data.x*0.25,22,v_data.w*0.25,v_data.h*0.25,Lcd.color565(100, 100, 100));
    Lcd.pushSprite(0, 0);

    Lcd.deleteSprite();
    M5.Lcd.setRotation(0);
    Lcd.createSprite(80, 160);
    Lcd.setTextSize(1);

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
