#include "MyRoverC.h"


void ShowJoyStickMode(JoyCommPacket packetJoyC)
{        
    char text_buff[100];

    Lcd.fillRect( 0,0,80,160,BLACK);

    //Lcd.setSwapBytes(true);
    Lcd.fillRect(0, 0, 160, 20, Lcd.color565(50, 50, 50));
    Lcd.setTextColor(WHITE);
    Lcd.drawCentreString(MySSID,40,6,1);

    
    // sprintf(text_buff, "%d %d", udplength,sizeof(packetJoyC));
    // Lcd.drawString(text_buff, 0, 6, 1);

    sprintf(text_buff, "%s", MySSID.c_str());
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

void MoveByJoyStick(uint16_t angle_L,uint16_t distance_L,int8_t x_L,int8_t y_L,
                          uint16_t angle_R,uint16_t distance_R,int8_t x_R,int8_t y_R)
{
  int16_t moving = 0;
  int16_t steering = 0;
  int leftFront = 0;
  int leftRear = 0;
  int rightFront = 0;
  int rightRear = 0;

  if(distance_L==0 && distance_R==0)
  {
    rover.Stop();
    return;
  }

  if( ((y_R>0 && y_L<0) || (y_R<0 && y_L>0)) && (x_R==0 && x_L==0)){
    // Rotate Mode (By using both stick on y axle)
    leftFront = y_L ;
    leftRear = y_L ;
    rightFront = y_R;
    rightRear = y_R;

  }else if(x_L!=0 && y_L==0 && abs(x_L-y_L) >20){
    // Left-Right Mode
    leftFront = -x_L-x_R;
    leftRear = x_L-x_R;
    rightFront = x_L;
    rightRear = -x_L;

  }else if(x_L==0 && y_L!=0 ){
    // Forward-Backward Mode
    moving = (y_L);
    steering = (y_L*x_R)/100;
    
    leftFront = moving - steering;
    leftRear = moving - steering;
    rightFront = moving + steering;
    rightRear = moving + steering;
  
  }else if(y_L==0 && y_R==0){  
    // Rotate Mode (By right stick on x axle)
    leftFront = -x_R/2 ;
    leftRear = -x_R/2 ;
    rightFront = x_R/2;
    rightRear = x_R/2;
    
  }

  rover.MoveMotor(leftFront,leftRear,rightFront,rightRear);

}
