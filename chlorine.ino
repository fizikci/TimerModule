#include <LiquidCrystal.h>
#include "IRremote.h"

IRrecv irrecv(8); //Signal Pin of IR receiver to Arduino Digital Pin 11
decode_results results;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int deltaT = 0;
#define SCR_HOME 0
#define SCR_TIME 1
#define SCR_SET 2
int screen = SCR_HOME;
char* key;
int lastKeyReadTime;
int timeSetup;
int scheduleSetup;
int timePartSet = 0;
int timeParts[3];

char* translateIR() {
  Serial.println(results.value);
  switch(results.value)
  {
    case 16580863:
    case 0xFFA25D: return "POWER";
    case 16613503:
    case 0xFF629D: return "UP";
    case 16589023:
    case 0xFF22DD: return "LEFT";
    case 16621663:
    case 0xFF02FD: return "ENTER";
    case 16605343:
    case 0xFFC23D: return "RIGHT";
    case 16617583:
    case 0xFFA857: return "DOWN";
  }
  return "???";
}

void screenInit(){
  lcd.clear();
  lcd.setCursor(0, 0);
  
      char str[8];
      int seconds = millis() / 1000 + deltaT;
  
  switch(screen){
    case SCR_HOME:
      lcd.print("Current Time");
      break;
    case SCR_TIME:
      lcd.print("Set Time");
      // display time
      timeParts[0] = seconds / 60 / 60 % 24;
      timeParts[1] = seconds / 60 % 60;
      timeParts[2] = seconds % 60;
      sprintf(str,"%02d:%02d:%02d", timeParts[0], timeParts[1], timeParts[2]);
      lcd.setCursor(8, 1);
      lcd.print(str);
      break;
    case SCR_SET:
      lcd.print("Set Schedule");
      break;
  }
}

void processKeyForScreen(){
  if(strcmp(key, "RIGHT")==0){
    screen = (screen == 2 ? 0 : screen+1);
    screenInit();
  }
  else if(strcmp(key, "LEFT")==0){
    screen = (screen == 0 ? 2 : screen-1);
    screenInit();
  }
  key = "";
}

void showHome(){
  // calculate time
  char str[8];
  int seconds = millis() / 1000 + deltaT;
  sprintf(str,"%02d:%02d:%02d", seconds / 60 / 60 % 24, seconds / 60 % 60, seconds % 60);
  
  // display time
  lcd.setCursor(8, 1);
  lcd.print(str);
  
  processKeyForScreen();
}

void showTimeSetup(){
  if(timeSetup == 0){
    if(strcmp(key,"ENTER")==0){
      timeSetup = 1;
      key = "";
    } 
    processKeyForScreen();
  }
  else {
    if(strcmp(key, "DOWN")==0){
      timeParts[timePartSet] = timeParts[timePartSet]==0 ? 59 : timeParts[timePartSet]-1;
      key = "";
    }
    else if(strcmp(key, "UP")==0){
      timeParts[timePartSet] = timeParts[timePartSet]==59 ? 0 : timeParts[timePartSet]+1;
      key = "";
    }
    else if(strcmp(key, "RIGHT")==0){
      timePartSet = timePartSet==2 ? 2 : timePartSet+1;
      key = "";
    }
    else if(strcmp(key, "LEFT")==0){
      timePartSet = timePartSet==0 ? 0 : timePartSet-1;
      key = "";
    }
    else if(strcmp(key,"ENTER")==0){
      timeSetup = 0;
      deltaT = timeParts[0] * 60 * 60 * 1000 + timeParts[1] * 60 * 1000 + timeParts[2] * 1000;
      key = "";
    }

    char str[8];
    if(millis() % 1000 < 500)
      switch(timePartSet){
      	case 0: sprintf(str, "  :%02d:%02d", timeParts[1], timeParts[2]); break;
      	case 1: sprintf(str, "%02d:  :%02d", timeParts[0], timeParts[2]); break;
      	case 2: sprintf(str, "%02d:%02d:  ", timeParts[0], timeParts[1]); break;
      }
    else
    	sprintf(str, "%02d:%02d:%02d", timeParts[0], timeParts[1], timeParts[2]);
    lcd.setCursor(8, 1);
    lcd.print(str);
  }
}

void showScheduleSetup(){
  if(strcmp(key,"DOWN")==0)
    scheduleSetup = 1;
  else if(strcmp(key,"ENTER")==0)
    scheduleSetup = 0;
  
  lcd.setCursor(2, 1);
  lcd.print(key);

  if(scheduleSetup == 0)
  	processKeyForScreen();
}

void setup() {
  irrecv.enableIRIn(); // Start the receiver
  lcd.begin(16, 2);
  screenInit();
  lastKeyReadTime = millis();
  Serial.begin(9600);
}

void loop() {
  switch(screen){
    case SCR_HOME: showHome(); break;
    case SCR_TIME: showTimeSetup(); break;
    case SCR_SET: showScheduleSetup(); break;
  }
  
  if (irrecv.decode(&results)) // have we received an IR signal?
  {
    int ms = millis();
    if(lastKeyReadTime+500>ms)
      return;
    
    key = translateIR(); 
    lastKeyReadTime = ms;
    Serial.println(key);
    
    irrecv.resume();
  }  

}