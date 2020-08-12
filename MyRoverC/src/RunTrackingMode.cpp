
#include "MyRoverC.h"

#define REMOTE_CTRL 0xAF
#define WHITE_PAPER 0xBF

uint8_t TrackTarget = REMOTE_CTRL;
uint8_t PrevState = kSeekingL;
uint8_t PrevSeeking = kSeekingR;
uint8_t PrevCX = 0;
uint16_t kThreshold = 40; 
v_response_t v_data;
uint8_t state = 0;
HardwareSerial VSerial(1);
bool bVisualAvailable = false;
uint8_t vcycle = 0;
uint16_t FoundAreaSize = 15000;
uint8_t MotorSpeed = 15; //13
uint8_t DelayTime = 50; //0
void MoveByColorTracking(uint8_t JoyPressed)
{
    if(JoyPressed==Pressed)
    {
        if(TrackTarget==REMOTE_CTRL)
        {
            TrackTarget = WHITE_PAPER;
            
        }
        else if(TrackTarget==WHITE_PAPER)
        {
            TrackTarget = REMOTE_CTRL;

        }
    }


    VSerial.write(TrackTarget);
    v_data.x = 0;
    v_data.y = 0;
    v_data.w = 0;
    v_data.h = 0;
    v_data.cx = 0;
    v_data.cy = 0;
    v_data.area = 0;

    if(DelayTime!=0)
    {
        delay(DelayTime);
        rover.Stop();
    }

    bVisualAvailable = false;

    if(VSerial.available())
    {
        bVisualAvailable = true;
        uint8_t buffer[15];
        int16_t width = 320;
        //int16_t height = 240;
        VSerial.readBytes(buffer, 15);
        v_data.x = (buffer[0] << 8) | buffer[1];
        v_data.y = (buffer[2] << 8) | buffer[3];
        v_data.w = (buffer[4] << 8) | buffer[5];
        v_data.h = (buffer[6] << 8) | buffer[7];
        v_data.cx = (buffer[8] << 8) | buffer[9];
        v_data.cy = (buffer[10] << 8) | buffer[11];
        v_data.area = (buffer[12] << 16) | (buffer[13] << 8) | buffer[14];

        if(v_data.cx > 0 && v_data.cx < width && v_data.area>150 && v_data.area<30000)
        {
            if(v_data.area > FoundAreaSize)
            {
                state = kTooClose;  // Stop
            }
            else if(v_data.cx > (width/2)-kThreshold && v_data.x < (width/2)+kThreshold)
            {
                state = kStraight;  // Go straight
            }
            else if(v_data.cx <= (width/2)-kThreshold-30)
            {
                state = kRotateL; // Rotate left
            }
            else if(v_data.cx >= (width/2)+kThreshold+30)
            {
                state = kRotateR;  // Rotate right
            }
            else if(v_data.cx <= (width/2)-kThreshold)
            {
                state = kLeft; // Go left
            }
            else if(v_data.cx >= (width/2)+kThreshold)
            {
                state = kRight;  // Go right
            }
            else
            {
                state = kSeeking;
            }
        }
        else
        {
            state = kSeeking;
        }
        if(state==kSeeking)
        {
            if(PrevCX>160 && PrevCX<=320 && PrevState!=kSeekingL && PrevState!=kSeekingR)
            {
                state = kSeekingR;  // Rotate
            }
            else if(PrevCX<160 && PrevCX>0 && PrevState!=kSeekingL && PrevState!=kSeekingR)
            {
                state = kSeekingL;  // Rotate
            }
            else if(PrevState==kSeekingL)
            {
                state = kSeekingL;  // Rotate
            }
            else if(PrevState==kSeekingR)
            {
                state = kSeekingR;  // Rotate
            }
            
        }

        Serial.printf("%d, %d, %d\n", v_data.x, v_data.area, state);

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
        Udp.beginPacket(udp_client, UDP_PORT);
        Udp.write((uint8_t *)&packetUnitV, sizeof(packetUnitV));
        Udp.endPacket();

        PrevState = state;
        PrevCX = v_data.cx;

        //The speed and time here may need to be modified according to the actual situation
        switch(state)
        {
            case kSeekingL:rover.Go(RotateLeft,MotorSpeed);break;
            case kSeekingR:rover.Go(RotateRight,MotorSpeed);break;
            case kLeft:rover.Go(Left,MotorSpeed);break;
            case kRight:rover.Go(Right,MotorSpeed);break;
            case kRotateL:rover.Go(RotateLeft,MotorSpeed);break;
            case kRotateR:rover.Go(RotateRight,MotorSpeed);break;
            case kStraight:rover.Go(Forward,MotorSpeed);break;
            case kTooClose:rover.Stop();break;
            default:rover.Go(RotateLeft,MotorSpeed);break;
        }
        vcycle++;
        if (vcycle > 79)
        {
            vcycle = 0;
        }
  
    }


}

void ShowTrackingMode()
{
    char text_buff[100];
    if(state == kSeekingR || state == kSeekingL){
        Lcd.fillScreen(RED);            
    }else{
        Lcd.fillScreen(GREEN);
    }

    Lcd.fillRect(0, 0, 80, 20, Lcd.color565(50, 50, 50));


    Lcd.setTextColor(WHITE);

    if(bVisualAvailable==false)
    {
        return;
    }
    String strTarget = "";
    switch(TrackTarget)
    {
        case WHITE_PAPER:strTarget="Paper";break;
        case REMOTE_CTRL:
        default:
            strTarget="Remote";break;

    }
	sprintf(text_buff, "Track:%s",strTarget.c_str());
	Lcd.drawCentreString(text_buff, 40, 6, 1);

    Lcd.setTextColor(BLACK);



    Lcd.fillRect((v_data.x*80)/320,22+((54*v_data.y)/240),(v_data.w*80)/320,(v_data.h*54)/240,Lcd.color565(100, 100, 100));




	sprintf(text_buff, " X:%3d  Y:%3d", v_data.x, v_data.y);
	Lcd.drawString(text_buff, 0, 76, 1);
	sprintf(text_buff, " W:%3d  H:%3d", v_data.w, v_data.h);
	Lcd.drawString(text_buff, 0, 90, 1);
	sprintf(text_buff, "CX:%3d CY:%3d", v_data.cx, v_data.cy);
	Lcd.drawString(text_buff, 0, 104, 1);

    Lcd.fillRect(((vcycle%8)*10),118,30,2,Lcd.color565(50, 50, 50));
    Lcd.fillRect(((cycle%8)*10),120,30,2,Lcd.color565(30, 30, 30));

	sprintf(text_buff, " AR:%d", v_data.area);
	Lcd.drawString(text_buff, 0, 132, 1);





    String strState;

    switch(state)
    {
        case kSeekingL:strState="Seek-L";break;
        case kSeekingR:strState="Seek-R";break;
        case kLeft:strState="Left";break;
        case kRight:strState="Right";break;
        case kRotateL:strState="Rotate-L";break;
        case kRotateR:strState="Rotate-R";break;
        case kStraight:strState="Forward";break;
        case kTooClose:strState="Stop";break;
        default:strState="*";break;
    }
	sprintf(text_buff, " ST:%s", strState.c_str());
	Lcd.drawString(text_buff, 0, 146, 1);

    Lcd.pushSprite(0, 0);

}