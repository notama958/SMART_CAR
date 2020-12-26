

#include <LiquidCrystal.h>
// compass sensor
#include <Wire.h>

LiquidCrystal lcd (37, 36, 35, 34, 33, 32);
int x = A8; // joystick
int y = A9; // joystick
int js_port = 4; // joystick port
float xDirection, yDirection;
float pwmX = -1;
float pwmY = -1;

// motor pins
int motorOne = 7;
int motorTwo = 8;
int speedOne = 9;
int speedTwo = 10;

//compass pins
#define CMPS14  0x60
int compass = 1;
int sda = 20;
int scl = 21;


int ENCA_left = 24; // knowing direction
int ENCB_left = 3; // countinng


int ENCA_right = 23; // knowing direction
int ENCB_right = 2; //counting

bool counterUpOne = true;
bool counterUpTwo = true;
int count = 0;

// control with key board
String myString;
int userInput;
char buff[255];
char text[10];
String turn_title;
long turn_value=0;
String move_title;
long move_value=0;
char terminator='\n';
char equal=':';
char delimiter=';';

//
void types(String a) { Serial.println("it's a String"); }
void types(int a) { Serial.println("it's an int"); }
void types(char *a) { Serial.println("it's a char*"); }
void types(float a) { Serial.println("it's a float"); }
void types(bool a) { Serial.println("it's a bool"); }
void types(long a){Serial.println("it's a long");}
void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
  
  
    //
    pinMode(ENCA_left, INPUT);
    pinMode(ENCB_left, INPUT);
    pinMode(ENCA_right, INPUT);
    pinMode(ENCB_right, INPUT);
  
    pinMode(js_port, INPUT);
  
    //compass sensor
    pinMode(sda, INPUT);
    pinMode(scl, INPUT);
    Wire.begin();

    pinMode(motorOne, OUTPUT);
    pinMode(motorTwo, OUTPUT);
    pinMode(speedOne, OUTPUT);
    pinMode(speedTwo, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(ENCB_left), ENCB_1, FALLING);
    attachInterrupt(digitalPinToInterrupt(ENCB_right), ENCB_2, FALLING);
    
    lcd.begin(20, 4);
    lcd.setCursor(0, 0);

}

void loop() {


  // get analog read from joystick
  /*xDirection = analogRead(x);
  yDirection = analogRead(y);
  getScaleXY(xDirection, yDirection); // convert into %(-100 - +100)*/

 String cmd_1,cmd_2;
  if(Serial.available())
  {
    readCommand(delimiter,cmd_1,cmd_2);
  }
  if(cmd_1.length()!=0)
  {
    separateCMD(cmd_1,equal);
    if(turn_value!=0)
    {
      lcd.setCursor(0,0);
      lcd.print("Turn:");
      lcd.setCursor(6,0);
      lcd.print("            ");
      lcd.setCursor(6,0);
      lcd.print(turn_value);
    }
    else{
      lcd.setCursor(2,1);
      lcd.print("Move:");
      lcd.setCursor(9,1);
      lcd.print("        ");
      lcd.setCursor(9,1);
      lcd.print(move_value);
    }
  }
  if(cmd_2.length()!=0)
  {
    separateCMD(cmd_2,equal);
    if(turn_value!=0)
    {
      lcd.setCursor(0,0);
      lcd.print("Turn:");
      lcd.setCursor(6,0);
      lcd.print("            ");
      lcd.setCursor(6,0);
      lcd.print(turn_value);
    }
    else{
      lcd.setCursor(2,1);
      lcd.print("Move:");
      lcd.setCursor(10,1);
      lcd.print("             ");
      lcd.setCursor(10,1);
      lcd.print(move_value);
    }
  }

  controlWithCMDs(turn_value, move_value);
   turn_value=0;
   move_value=0;
}

// separate 2 cmds
void readCommand(char delim,String &cmd_1, String &cmd_2)
{

  //Serial.println("Read command");
  myString=Serial.readStringUntil(terminator);
  int len=myString.length();
  int index=myString.indexOf(delim);
  if(index>-1)
  {
    for(int i=0;i<index;i++)
    {
        cmd_1+=myString[i];
    }
    if(index!=len-1)
    {
       for(int j=index+1;j<len;j++)
       {
        cmd_2+=myString[j];
       }

    }
    
  }
}

void separateCMD(String cmd, char delim)
{
  String temp_title, temp_value;
  
  //Serial.println("separate CMD");
  if(cmd.length()!=0)
  {

      int len=cmd.length();

      int index=cmd.indexOf(delim);
      if(index>-1)
    {
      for(int i=0;i<index;i++)
      {
          temp_title+=cmd[i];
      }
      if(index!=len-1)
      {

         for(int j=index+1;j<len;j++)
         {
          temp_value+=cmd[j];
         }

      }
    }
        if(temp_title=="Move"||temp_title=="move"||temp_title=="MOVE")
    {
        move_value=temp_value.toInt();
        move_title=temp_title;
    }
        else if(temp_title=="Turn"||temp_title=="turn"||temp_title=="TURN")
    {
        turn_value=temp_value.toInt();
        turn_title=temp_title;
    }
  }
}

//control with input
void controlWithCMDs(int t, int m)
{
    lcd.setCursor(0, 2);
    lcd.print("Puls_ENCB:");
     
  // rotate first
  if(t!=0)
  {
    turn(t);
  }
  // then move
  // 1000puls =40cm
  if(m!=0)
  {
    Serial.println("MOVE");
    long current=count;// current count;
    long input_temp=abs(round(m*25));
    while (1)
    {
        lcd.setCursor(10, 2);
        lcd.print("            ");
        lcd.setCursor(10, 2);
        lcd.print(count);
      if(m>0)
      {
        digitalWrite(motorOne, HIGH);
        digitalWrite(motorTwo, HIGH);
      }
      else
      {
        digitalWrite(motorOne, LOW);
        digitalWrite(motorTwo, LOW);
      }
        // set speed into motor
      analogWrite(speedOne, 100); 
      analogWrite(speedTwo, 100);
      long curr_temp=abs(count-current);
      if(curr_temp>=input_temp)
      {
              Serial.println("OUT");
             resetLeft();
             resetRight();
              break;
      }
    }
  }
  
    

   
}

//turn function
void turn(long inputDirection)
{
 // Read Direction
  long currentDirection,newDirection;
  readDirection(currentDirection);
  
      newDirection=currentDirection+inputDirection;
  if( currentDirection+inputDirection>0)
  {
    if(currentDirection+inputDirection>360)
    {
      newDirection-=360;
    }
  
  }
  else{
    newDirection+=360;
  }
  if(inputDirection<0)
  {
       Serial.println("CW");
    while(1)
    {
      if(abs(currentDirection-newDirection)>=0&&abs(currentDirection-newDirection)<=1)
      {
        
        Serial.println("STOP");
        resetLeft();
        resetRight();
        break;
        
      }
        digitalWrite(motorOne,LOW);
        analogWrite(speedOne,200);
        digitalWrite(motorTwo,HIGH);
        analogWrite(speedTwo,200);
        readDirection(currentDirection);
        lcd.setCursor(11,0);
          lcd.print("                ");
          lcd.setCursor(11,0);
          lcd.print(abs(currentDirection-newDirection));
    }
  }
  else{
           Serial.println("CCW");
    while(1)
    {
      if(abs(currentDirection-newDirection)>=0&&abs(currentDirection-newDirection)<=1)
      {
        Serial.println("STOP");
        resetLeft();
        resetRight();
        break;
      }
        digitalWrite(motorOne,HIGH);
        analogWrite(speedOne,200);
        digitalWrite(motorTwo,LOW);
        analogWrite(speedTwo,200);
        Serial.println(abs(currentDirection-newDirection));
        readDirection(currentDirection);
        lcd.setCursor(11,0);
          lcd.print("                ");
          lcd.setCursor(11,0);
          lcd.print(abs(currentDirection-newDirection));
    }
  }
           
  
}

// reset pin
void resetLeft()
{
  digitalWrite(motorOne,0);
  analogWrite(speedOne,0);
}
void resetRight()
{
  digitalWrite(motorTwo,0);
  analogWrite(speedTwo,0);

}


// compass bearing 0-255
void readDirection(long &degree){
    // reading sensor value
  Wire.beginTransmission(CMPS14);
  Wire.write(compass);
  Wire.endTransmission(false);
  Wire.requestFrom(CMPS14, compass, true);
  if (Wire.available() >= 1)
  {
    long raw = Wire.read();
     degree=((long)raw*360)/255;
    char* direct;
    if(degree==0||degree==360)
      direct="N";
    else if(degree>0&&degree<90)
      direct="NW";
    else if(degree>90&&degree<180)
      direct="SW";
    else if(degree>180&&degree<270)
      direct="SE";
    else if(degree>270&&degree<360)
      direct="NE";
    else if(degree==90)
      direct="W";
    else if(degree==180)
      direct="S";
    else if(degree==270)
      direct="E";
    lcd.setCursor(0, 3);
    lcd.print("Degree:");
    lcd.print("             ");
    lcd.setCursor(7,3);
    lcd.print(degree);
    lcd.setCursor(11,3);
    lcd.print(direct);
  }
}




void ENCB_2() { // count for motor two
  int value = digitalRead(ENCA_right);
  if (value == 1)
  {
    ++count;
  }
  else {
    --count;
  }

}
void ENCB_1() { // count for motor one
  int value = digitalRead(ENCA_left);
  if (value == 1)
  {
    ++count;
  }
  else {
    --count;
  }

}
