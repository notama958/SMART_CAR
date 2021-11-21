
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



int ENCA_left=24; // knowing direction
int ENCB_left=3; // countinng


int ENCA_right=23;// knowing direction
int ENCB_right=2;//counting

bool counterUpOne=true;
bool counterUpTwo=true;
int count=0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);


  //
  pinMode(ENCA_left, INPUT);
  pinMode(ENCB_left,INPUT);
  pinMode(ENCA_right, INPUT);
  pinMode(ENCB_right,INPUT);
  
  pinMode(js_port,INPUT);
  //Serial.println(title);
  pinMode(motorOne,OUTPUT);
  pinMode(motorTwo,OUTPUT);
  pinMode(speedOne,OUTPUT);
  pinMode(speedTwo,OUTPUT);
  attachInterrupt(digitalPinToInterrupt(ENCB_left),ENCB_1,FALLING);
  //attachInterrupt(digitalPinToInterrupt(ENCA_right),ENCA,RISING);
  attachInterrupt(digitalPinToInterrupt(ENCB_right),ENCB_2,FALLING);
  
  lcd.begin(20,4);
  lcd.setCursor(0,0);
  //char *title="-100      0      100";
  //lcd.print(title);
}
long input=-30;

void loop() {
    
 
  // get analog read from joystick
  xDirection=analogRead(x);
  yDirection=analogRead(y);
  //getScaleXY(xDirection,yDirection); // convert into %(-100 - +100)
 
  // move with 40cm forward or backward -40cm
  // 1000 pulses =40cm

  MoveTo(input);
  input=0;
 
}
void MoveTo(long distance)
{

  Serial.println("MOVE");
    long current=count;// current count;
    long input_temp=abs(round(distance*25));
    while (1)
    {
       lcd.setCursor(0,0);
      lcd.print("Puls_ENCB:");
      lcd.setCursor(10,0);
      lcd.print("            ");
      lcd.setCursor(10,0);
      lcd.print(count);
      if(distance>0)
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
      Serial.println(curr_temp);
      if(curr_temp>=input_temp)
      {
              Serial.println("OUT");
             resetLeft();
             resetRight();
              break;
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
// Scaling
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

void ENCA_RIGHT(){ // motor 1
  int value=digitalRead(ENCA_left);
    value==1?counterUpOne=true:counterUpOne=false;
         
}
void ENCB_2(){ // count for motor two
      int value=digitalRead(ENCA_right);
   if(value==1)
   {
      ++count;
   }
   else {
    --count;
   }
  
}
void ENCB_1(){ // count for motor one 
      int value=digitalRead(ENCA_left);
   if(value==1)
   {
      ++count;
   }
   else {
    --count;
   }
    
}
