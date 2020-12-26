#include <LiquidCrystal.h>
LiquidCrystal lcd (37,36,35,34,33,32);
int x=A8;// joystick
int y=A9;// joystick
int js_port=4; // joystick port
float xDirection,yDirection;
float pwmX=-1;
float pwmY=-1;

int motorOne=7;
int motorTwo=8;
int speedOne=9;
int speedTwo=10;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
    lcd.begin(20,4);

   pinMode(js_port,INPUT);
  //Serial.println(title);
  pinMode(motorOne,OUTPUT);
  pinMode(motorTwo,OUTPUT);
  pinMode(speedOne,OUTPUT);
  pinMode(speedTwo,OUTPUT);
  lcd.print("Joystick control");
}

void loop() {
    
  xDirection=analogRead(x);
  yDirection=analogRead(y);
  getScaleXY(xDirection,yDirection); // convert into %(-100 - +100)
  
}

void getScaleXY(float a, float b)
{
  char ret[100];

  // because of unstablility of joystick 
  // convert so at the stable state it has value 0
  pwmX=((a*200)/1004)-100;
  pwmY=((b*200)/1054)-100;
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  sprintf(ret,"X: %d",round(pwmX)); // print out
  lcd.print(ret);
  //
  lcd.setCursor(0,2);
  lcd.print("                ");
  lcd.setCursor(0,2);
  sprintf(ret,"Y: %d",round(pwmY)); // print out
  lcd.print(ret);
  
  //set direction if (+) go up (-) go down
  if(pwmX>0)
    digitalWrite(motorOne,HIGH);
  else
    digitalWrite(motorOne,LOW);
  if(pwmY>0)
    digitalWrite(motorTwo,HIGH);
  else
    digitalWrite(motorTwo,LOW);
   
   // set speed into motor
   analogWrite(speedOne,abs(round((pwmX*985)/100))); //FIXED need replace weak
   analogWrite(speedTwo,abs(round((pwmY*1023)/100))); //FIXED 
  
}
