/*
 * @Author: Sorzn
 * @Date: 2019-11-22 14:48:10
 * @LastEditTime: 2019-11-22 15:45:27
 * @Description: M5Stack project
 * @FilePath: /M5StickC/examples/Hat/JoyC/JoyC.ino
 */

#include <M5StickC.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "EEPROM.h"
#include "JoyC.h"

#define EEPROM_SIZE 64
#define SYSNUM 3

enum ModeType{
	SettingMode = 0,
    JoyStickMode = 1,
    BallTrackingMode = 2
};


typedef struct
{
	const uint8_t Header = 0xAA;
	const uint8_t Marker = 0x55;
	uint8_t Mode;
	uint16_t L_Angle;
	uint16_t L_Distance;
	uint8_t L_X;
	uint8_t L_Y;
	uint8_t L_Press;
	uint16_t R_Angle;
	uint16_t R_Distance;
	uint8_t R_X;
	uint8_t R_Y;
	uint8_t R_Press;
	const uint8_t End = 0xee;

} JoyCommPacket;

JoyC joyL(LeftJoy);
JoyC joyR(RightJoy);

uint8_t show_flag = 0;

TFT_eSprite Lcd = TFT_eSprite(&M5.Lcd);

uint64_t realTime[4], time_count = 0;
bool k_ready = false;
uint32_t key_count = 0;

IPAddress local_IP(192, 168, 4, 100 + SYSNUM);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);	//optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

const char *ssid = "M5AP";
const char *password = "77777777";

WiFiUDP Udp;
uint32_t send_count = 0;
uint8_t system_state = 0;

bool ModeSelection = false;

const char *MenuList[] = {"Setting","JoyStick","BallTracking"};
int SelMode = JoyStickMode;
int ModeCount = 3;
bool StickFree = false;
char text_buff[100];

void SendUDP(JoyCommPacket packet)
{
	if (WiFi.status() == WL_CONNECTED)
	{
		Udp.beginPacket(IPAddress(192, 168, 4, 1), 1000 + SYSNUM);
		Udp.write((uint8_t *)&packet, sizeof(packet));
		Udp.endPacket();
	}
}

char APName[20];
String WfifAPBuff[16];
uint32_t count_bn_a = 0, choose = 0;
String ssidname;
uint32_t count = 0;
JoyCommPacket packet;
uint32_t holdCount = 20;

//============================================================
// Setup
//============================================================
void setup()
{
	// put your setup code here, to run once:
	M5.begin();
	Wire.begin(0, 26, 10000);
	EEPROM.begin(EEPROM_SIZE);

	M5.Lcd.setRotation(4);
	M5.Lcd.setSwapBytes(false);
	Lcd.createSprite(80, 160);
	Lcd.setSwapBytes(true);

	Lcd.fillRect(0, 0, 80, 20, Lcd.color565(50, 50, 50));
	Lcd.setTextSize(2);
	Lcd.setTextColor(GREEN);
	Lcd.setCursor(55, 6);

	M5.update();
	if ((EEPROM.read(0) != 0x56) || (M5.BtnA.read() == 1))
	{
		WiFi.mode(WIFI_STA);
		int n = WiFi.scanNetworks();
		Lcd.setTextSize(1);
		Lcd.setTextColor(WHITE);
		Lcd.fillRect(0, 0, 80, 20, Lcd.color565(50, 50, 50));
		Lcd.drawCentreString("Wifi SSIDs", 40, 6, 1);
		Lcd.setTextColor(GREEN);

		if (n == 0)
		{
			Lcd.setCursor(5, 20);
			Lcd.printf("no networks");
		}
		else
		{
			int count = 0;
			for (int i = 0; i < n; ++i)
			{
				if (WiFi.SSID(i).indexOf("M5AP") != -1)
				{
					if (count == 0)
					{
						Lcd.setTextColor(GREEN);
					}
					else
					{
						Lcd.setTextColor(WHITE);
					}
					Lcd.setCursor(5, 25 + count * 10);
					String str = WiFi.SSID(i);
					Lcd.printf(str.c_str());
					WfifAPBuff[count] = WiFi.SSID(i);
					count++;
				}
			}
			Lcd.pushSprite(0, 0);
			while (1)
			{
				if (M5.BtnA.read() == 1)
				{
					if (count_bn_a >= holdCount)
					{
						count_bn_a = holdCount+1;
						EEPROM.writeUChar(0, 0x56);
						EEPROM.writeString(1, WfifAPBuff[choose]);
						ssidname = WfifAPBuff[choose];
						break;
					}
					count_bn_a++;
					Serial.printf("count_bn_a %d \n", count_bn_a);
				}
				else if ((M5.BtnA.isReleased()) && (count_bn_a != 0))
				{
					Serial.printf("count_bn_a %d", count_bn_a);
					if (count_bn_a > holdCount)
					{
					}
					else
					{
						choose++;
						if (choose >= count)
						{
							choose = 0;
						}
						Lcd.fillRect(0, 0, 80, 20, Lcd.color565(50, 50, 50));
						for (int i = 0; i < count; i++)
						{
							Lcd.setCursor(5, 25 + i * 10);
							if (choose == i)
							{
								Lcd.setTextColor(GREEN);
							}
							else
							{
								Lcd.setTextColor(WHITE);
							}
							Lcd.printf(WfifAPBuff[i].c_str());
						}
						Lcd.pushSprite(0, 0);
					}
					count_bn_a = 0;
				}
				delay(10);
				M5.update();
			}
			//EEPROM.writeString(1,WfifAPBuff[0]);
		}
	}
	else if (EEPROM.read(0) == 0x56)
	{
		ssidname = EEPROM.readString(1);
		EEPROM.readString(1, APName, 16);
	}

	Lcd.fillRect(0, 20, 80, 140, BLACK);

	if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
	{
		Serial.println("STA Failed to configure");
	}

	WiFi.begin(ssidname.c_str(), password);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}

	Udp.begin(2000);

	// Lcd.pushImage(0,0,20,20,(uint16_t *)connect_on);
	Lcd.pushSprite(0, 0);
}

//============================================================
// Loop
//============================================================
void loop()
{

	M5.update();
	// img.fillSprite(TFT_BLACK);


	delay(10);

	if (WiFi.status() != WL_CONNECTED)
	{
		ShowDisconnectedScreen();

		count++;
		if (count > 500)
		{
			WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
			count = 0;
		}
	}
	else
	{

		joyL.ReadData();
		joyR.ReadData();

		// if(M5.BtnA.wasPressed())
		// {
		// 	joyL.SetLedColor(0x100010);
		// 	joyR.SetLedColor(0x100010);
		// 	show_flag = 1 - show_flag;
		// }
		if( StickFree == true &&  (joyL.Press() == Pressed && joyR.Press() == Pressed) 
		&& (joyL.IsPressStateChanged() == true || joyR.IsPressStateChanged() == true) )
		{
			if(ModeSelection == false)
			{
				// Entry Mode Selection
				ModeSelection = true;
				joyL.SetLedColor(0x0000FF);
				joyR.SetLedColor(0x0000FF);
			}
			else
			{
				// Exit Mode Selection
				ModeSelection = false;
				switch(SelMode)
				{
					case SettingMode:
						joyL.ResetRangeValue();
						joyL.SetLedColor(0x100010);
						joyR.SetLedColor(0x100010);
						break;

					case BallTrackingMode:
						break;
						joyL.SetLedColor(0x00FF00);
						joyR.SetLedColor(0x00FF00);

					case JoyStickMode:
					default:
						joyL.SetLedColor(0x00FF00);
						joyR.SetLedColor(0x00FF00);

						break;
				}
			}
			StickFree = false;
		}
		

		if(StickFree == false && joyL.GetDirection()==Neutral && joyR.GetDirection()==Neutral 
		     && joyL.Press()==Release && joyR.Press()==Release)
		{
			StickFree = true;
		}


		

		if (ModeSelection == true)
		{
			// Show Mode Selection Screen
			sprintf(text_buff, "Select Mode:%d", SelMode);
			ShowModeSelect(text_buff,MenuList,ModeCount,0,SelMode);
			ChangeMode();
		}
		else
		{
			// Show Selected Screen Mode
			switch(SelMode)
			{
				case SettingMode:
					joyL.UpdateValueRange();
					joyR.UpdateValueRange();
					ShowSettingMode();
					break;

				case BallTrackingMode:
					break;

				case JoyStickMode:
				default:
					joyL.SetLedColor(0x00FF00);
					joyR.SetLedColor(0x00FF00);
					ShowRunningMode();
					break;
			}

			// Transfer data to RoverC
			packet.Mode = SelMode;
			packet.L_Angle =  joyL.Angle(100);
			packet.L_Distance = joyL.Distance(100);
			
			packet.L_X = Conv(joyL.X(),joyL.XMin,joyL.XMax);
			packet.L_Y = Conv(joyL.Y(),joyL.YMin,joyL.YMax);
			packet.L_Press = joyL.Press();
			packet.R_Angle = joyR.Angle(100);
			packet.R_Distance = joyR.Distance(100);
			packet.R_X = Conv(joyR.X(),joyR.XMin,joyR.XMax);
			packet.R_Y = Conv(joyR.Y(),joyR.YMin,joyR.YMax);
			packet.R_Press = joyR.Press();
			SendUDP(packet);

		}



	}

	delay(10);
}

void ChangeMode()
{
	if(StickFree==true && joyL.GetDirection()==Forward)
	{
		SelMode--;  
		StickFree = false;

	}
	else if(StickFree==true && joyL.GetDirection()==Backward)
	{
		SelMode++;  
		StickFree = false;

	}
	

	if(SelMode>ModeCount-1)
	{
		SelMode=0;
	}
	if(SelMode<0)
	{
		SelMode=ModeCount-1;
	}

}

uint8_t Conv(uint8_t value,uint8_t min,uint8_t max)
{
	if(value > (max-min)*0.6 )
	{
		return value-((max-min)/2);
	}
	else if(value < (max-min)*0.4)
	{
		return value-((max-min)/2);
	}
	return 0;
}
void ShowDisconnectedScreen()
{
	Lcd.fillRect(0, 0, 80, 160, BLACK);
	Lcd.drawCentreString("Disconnect", 40, 6, 1);
	Lcd.pushSprite(0, 0);
}

void ShowModeSelect(char *title,const char *menuList[],int itemCount,int startIndex,int selIndex )
{
	int max = 9;
	int index = 0;
	int posY = 34;
	uint8_t fontSize = 1;
	int lineSpace = 14;

	Lcd.fillRect(0, 0, 80, 160, BLACK);
	Lcd.setTextColor(GREEN);
	sprintf(text_buff, "%s", title);
	Lcd.fillRect(0, 0, 80, 20, Lcd.color565(50, 50, 50));
	Lcd.drawCentreString(text_buff, 40, 6, 1);

	for(int i = 0; i<max;i++)
	{

		if(startIndex+i >=itemCount)
		{
			break;
		}
		if(selIndex==startIndex+i)
		{
			Lcd.setTextColor(GREEN);
		}
		else
		{
			Lcd.setTextColor(WHITE);
		}

		sprintf(text_buff, "%s", MenuList[startIndex+i]);
		Lcd.drawCentreString(text_buff, 40, posY+(i*lineSpace), 1);

	}

	Lcd.pushSprite(0, 0);


}

void ShowRunningMode()
{
	Lcd.fillRect(0, 0, 80, 160, BLACK);

	sprintf(text_buff, "%s", ssidname.c_str());
	Lcd.fillRect(0, 0, 80, 20, Lcd.color565(50, 50, 50));
	Lcd.drawCentreString(text_buff, 40, 6, 1);

	Lcd.drawCentreString("A", 40, 34, 1);
	Lcd.drawCentreString("D", 40, 48, 1);
	Lcd.drawCentreString("X", 40, 62, 1);
	Lcd.drawCentreString("Y", 40, 76, 1);
	Lcd.drawCentreString("P", 40, 90, 1);

	// Right Side Info
	sprintf(text_buff, "%d", packet.R_Angle);
	Lcd.drawRightString(text_buff, 80, 34, 1);
	sprintf(text_buff, "%d", packet.R_Distance);
	Lcd.drawRightString(text_buff, 80, 48, 1);
	sprintf(text_buff, "%d", (int8_t)packet.R_X);
	Lcd.drawRightString(text_buff, 80, 62, 1);
	sprintf(text_buff, "%d", (int8_t)packet.R_Y);
	Lcd.drawRightString(text_buff, 80, 76, 1);
	sprintf(text_buff, "%d", packet.R_Press);
	Lcd.drawRightString(text_buff, 80, 90, 1);

	// Left Side Info
	sprintf(text_buff, "%d", packet.L_Angle);
	Lcd.drawString(text_buff, 0, 34, 1);
	sprintf(text_buff, "%d", packet.L_Distance);
	Lcd.drawString(text_buff, 0, 48, 1);
	sprintf(text_buff, "%d", (int8_t)packet.L_X);
	Lcd.drawString(text_buff, 0, 62, 1);
	sprintf(text_buff, "%d", (int8_t)packet.L_Y);
	Lcd.drawString(text_buff, 0, 76, 1);
	sprintf(text_buff, "%d", joyL.Press());
	Lcd.drawString(text_buff, 0, 90, 1);

	Lcd.pushSprite(0, 0);
}

void ShowSettingMode()
{
	Lcd.fillRect(0, 0, 80, 160, BLACK);

	Lcd.drawCentreString("A", 40, 6, 1);
	Lcd.drawCentreString("D", 40, 20, 1);
	Lcd.drawCentreString("X", 40, 34, 1);
	Lcd.drawCentreString("Y", 40, 48, 1);
	Lcd.drawCentreString("P", 40, 62, 1);

	// Right Side Info
	sprintf(text_buff, "%d", joyR.AngleMax);
	Lcd.drawRightString(text_buff, 80, 6, 1);
	sprintf(text_buff, "%d", joyR.DistanceMax);
	Lcd.drawRightString(text_buff, 80, 20, 1);
	sprintf(text_buff, "%d", joyR.XMax);
	Lcd.drawRightString(text_buff, 80, 34, 1);
	sprintf(text_buff, "%d", joyR.YMax);
	Lcd.drawRightString(text_buff, 80, 48, 1);
	sprintf(text_buff, "%d", joyR.Press());
	Lcd.drawRightString(text_buff, 80, 62, 1);

	// Left Side Info
	sprintf(text_buff, "%d", joyL.AngleMax);
	Lcd.drawString(text_buff, 0, 6, 1);
	sprintf(text_buff, "%d", joyL.DistanceMax);
	Lcd.drawString(text_buff, 0, 20, 1);
	sprintf(text_buff, "%d", joyL.XMax);
	Lcd.drawString(text_buff, 0, 34, 1);
	sprintf(text_buff, "%d", joyL.YMax);
	Lcd.drawString(text_buff, 0, 48, 1);
	sprintf(text_buff, "%d", joyL.Press());
	Lcd.drawString(text_buff, 0, 62, 1);

	Lcd.drawCentreString("A", 40, 90, 1);
	Lcd.drawCentreString("D", 40, 104, 1);
	Lcd.drawCentreString("X", 40, 118, 1);
	Lcd.drawCentreString("Y", 40, 132, 1);
	Lcd.drawCentreString("P", 40, 146, 1);

	// Right Side Info
	sprintf(text_buff, "%d", joyR.AngleMin);
	Lcd.drawRightString(text_buff, 80, 90, 1);
	sprintf(text_buff, "%d", joyR.DistanceMin);
	Lcd.drawRightString(text_buff, 80, 104, 1);
	sprintf(text_buff, "%d", joyR.XMin);
	Lcd.drawRightString(text_buff, 80, 118, 1);
	sprintf(text_buff, "%d", joyR.YMin);
	Lcd.drawRightString(text_buff, 80, 132, 1);
	sprintf(text_buff, "%d", joyR.Press());
	Lcd.drawRightString(text_buff, 80, 146, 1);

	// Left Side Info
	sprintf(text_buff, "%d", joyL.AngleMin);
	Lcd.drawString(text_buff, 0, 90, 1);
	sprintf(text_buff, "%d", joyL.DistanceMin);
	Lcd.drawString(text_buff, 0, 104, 1);
	sprintf(text_buff, "%d", joyL.XMin);
	Lcd.drawString(text_buff, 0, 118, 1);
	sprintf(text_buff, "%d", joyL.YMin);
	Lcd.drawString(text_buff, 0, 132, 1);
	sprintf(text_buff, "%d", joyL.Press());
	Lcd.drawString(text_buff, 0, 146, 1);

	Lcd.pushSprite(0, 0);
}

