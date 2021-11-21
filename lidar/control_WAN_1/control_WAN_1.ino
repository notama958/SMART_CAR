  
/**
   Final control WAN version 1.2 for smart car 2020-2021
   author: Yen Tran
   Smart Car using Serial communication between Arduino and ESP6288
   Lidar litev3 and CMPS14 compass are used in the project
*/
#include <LiquidCrystal.h>
// compass sensor
#include <Wire.h>
#include <LIDARLite.h>
#include <MsTimer2.h>
LiquidCrystal lcd (37, 36, 35, 34, 33, 32);
int x = A8; // joystick
int y = A9; // joystick
int js_port = 4; // joystick port optional
float xDirection, yDirection;
float pwmX = -1;
float pwmY = -1;

// motor pins
int motorOne = 7;
int motorTwo = 8;
int speedOne = 9;
int speedTwo = 10;

//Address of the CMPS14 compass on i2c
#define _i2cAddress  0x60
int compass = 1;
int sda = 20;
int scl = 21;
byte _byteHigh;
byte _byteLow;

int bearing;
char* direct;
#define BEARING_Register 2
#define COMMUNICATION_TIMEOUT 1000

#define ONE_BYTE   1
#define TWO_BYTES  2



int ENCA_left = 24; // knowing direction
int ENCB_left = 3; // countinng


int ENCA_right = 23; // knowing direction
int ENCB_right = 2; //counting

bool counterUpOne = true;
bool counterUpTwo = true;
int count = 0;

// UART
String myString;
int userInput;
char buff[255];
int is;
char text[10];
String turn_title;
long turn_value = 0;
String move_title;
long move_value = 0;
char terminator = '\n';
char equal = ':';
char delimiter = ';';
//---LiDar-----
LIDARLite lidarLite;
int incre = 0; // increment counter
int distance;
unsigned long currentTime = 0;
unsigned long preTime1 = 0;
unsigned long preTime2 = 0;
String statusCode[] = {"OFF", "RUN FW", "RUN BW"};

void types(String a) {
  Serial.println("it's a String");
}
void types(int a) {
  Serial.println("it's an int");
}
void types(char *a) {
  Serial.println("it's a char*");
}
void types(float a) {
  Serial.println("it's a float");
}
void types(bool a) {
  Serial.println("it's a bool");
}
void types(long a) {
  Serial.println("it's a long");
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);


  // set pinMode for encoder
  pinMode(ENCA_left, INPUT);
  pinMode(ENCB_left, INPUT);
  pinMode(ENCA_right, INPUT);
  pinMode(ENCB_right, INPUT);
  // pinMode for joystick (optional)
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
  // init Lidar
  lidarLite.begin(0, true);

  lidarLite.configure(0);
  /*MsTimer2::set(1000, sendLidar);
    MsTimer2::start();
    MsTimer2::set(1500, sendDegree);
    MsTimer2::start();
  */
}
/**
   Loop for reading coming commands from UI and sending new commands to UI
 * * readCommand() -> with terminator "\n"
 * * parseCMD() -> parsing command
 * * sendLidar() -> send new lidar value to UI in 3000 millis
 * * sendDegree() -> send new direction value to UI in 3000 millis
*/
void loop() {


  // timer for sending lidar and direction
  currentTime = millis();
  // checking Serial for coming commands from the UI
  String cmd_1;
  if (Serial.available())
  {
    readCommand(cmd_1);
  }
  if (cmd_1.length() != 0)
  {
    parseCMD(cmd_1, equal);
    if (turn_value != 0)
    {
      lcd.setCursor(0, 0);
      lcd.print("Turn:");
      lcd.setCursor(6, 0);
      lcd.print("            ");
      lcd.setCursor(6, 0);
      lcd.print(turn_value);
    }
    else {
      lcd.setCursor(2, 1);
      lcd.print("Move:");
      lcd.setCursor(9, 1);
      lcd.print("        ");
      lcd.setCursor(9, 1);
      lcd.print(move_value);
    }
  }



  Move(move_value);
  Turn(turn_value);
  turn_value = 0;
  move_value = 0;
  if (currentTime - preTime1 >= 1000)
  {
    preTime1 = currentTime;
    sendLidar();
  }
  if (currentTime - preTime2 >= 1500)
  {
    preTime2 = currentTime;
    sendDegree();

  }

}

// sending values to UI
void sendLidar()
{
  ReadLidar();
  Serial.print("Lidar:");
  Serial.println( distance);
}
void sendDegree()
{
  long currentDeg;
  readDirection(currentDeg);
  Serial.print("Deg:");
  Serial.println(currentDeg);
}
// checking command end with ;
void readCommand(String & cmd_1)
{

  //Serial.println("Read command");
  // turn:<value>
  // move:<value>
  serial_read(); // update new buff command
  Serial.println(buff);
  int len = strlen(buff);

  for (int i = 0; i < len; i++)
  {
    cmd_1 += buff[i];
  }


}
// parseCMD
void parseCMD(String cmd, char delim)
{
  String temp_title, temp_value;

  //Serial.println("separate CMD");
  if (cmd.length() != 0)
  {

    int len = cmd.length();

    int index = cmd.indexOf(delim);
    if (index > -1)
    {
      for (int i = 0; i < index; i++)
      {
        temp_title += cmd[i];
      }
      if (index != len - 1)
      {

        for (int j = index + 1; j < len; j++)
        {
          temp_value += cmd[j];
        }

      }
    }

    if (temp_title == "Move" || temp_title == "move" || temp_title == "MOVE")
    {
      move_value = temp_value.toInt();
      move_title = temp_title;
    }
    else if (temp_title == "Turn" || temp_title == "turn" || temp_title == "TURN")
    {
      turn_value = temp_value.toInt();
      turn_title = temp_title;
    }
  }
}
// serial read
void serial_read()
{
  boolean done = 0;

  while (done == 0) // wait end of message LF
  {
    while (Serial.available() > 0)
    {
      char t = Serial.read();

      if (t > 13)
      {
        buff[is] = t;
        // Serial.println(int(t));
        is++;
      }

      if (t == 10) {         // check if end of message  LF
        buff[is] = 0; // end of string
        is = 0;
        done = 1;

        for (int j = 0; j < 3; j++)
        {
          //  Serial.println(int(buf[j]));
        }
      }

    }  // end of while serial available

    if (done == 1)
    {
      done = 0;
      break;
    }

  }   // End of message  LF

  // Serial.println(buf);

}
/**
   Moving functions PASSED
   Move: syntax Move|move|MOVE:<int number>;
 * ** ReadLidar() -> update distance globala variable
 * ** Moveto() -> Move forward or backward
*/
void Move( int m)
{
  /*lcd.setCursor(0, 2);
    lcd.print("Puls_ENCB:");


    // then move
    if (m != 0)
    {
    long current = count; // current count;
    long input_temp = abs(round(m * 25));
    while (1)
    {
      lcd.setCursor(10, 2);
      lcd.print("            ");
      lcd.setCursor(10, 2);
      lcd.print(count);
      if (m > 0)
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
      long curr_temp = abs(count - current);
      if (curr_temp >= input_temp)
      {
        Serial.println("OUT");
        resetLeft();
        resetRight();
        break;
      }
    }
    }

  */
  // change to using lidar
  ReadLidar();
  lcd.setCursor(0, 2);
  lcd.print("Distance:");
  /*lcd.setCursor(0, 2);
    lcd.print("Status:");*/
  int saved = distance - m;
  /*if (m != 0)
    {
    Serial.print(">>>>>>>>>>>>>>>>>>>>>>");
    Serial.print(distance);
    Serial.print(">>>>>>>>>>>>>>>>>>>>>>");
    Serial.print(saved);

    }*/
  while (1)
  {
    // checking halting condition
    ReadLidar(); // update the new one
    if (abs(distance - saved) < 2)
    {
      resetLeft();
      resetRight();
      break;
    }
    if (m < 0)
    {
      MoveTo(-1); //  backwards
      lcd.setCursor(13, 2);
      lcd.print("      ");
      lcd.setCursor(13, 2);
      lcd.print(statusCode[2]);
    }
    else if (m > 0) {
      MoveTo(1); //  forward
      lcd.setCursor(13, 2);
      lcd.print("      ");
      lcd.setCursor(13, 2);
      lcd.print(statusCode[1]);

    }

    lcd.setCursor(9, 2);
    lcd.print("    ");
    lcd.setCursor(9, 2);
    lcd.print(distance);

  }


}
// --readLidar--
void ReadLidar() {
  /*
    distance(bool biasCorrection, char lidarliteAddress)
    Take a distance measurement and read the result.
    Parameters
    ----------------------------------------------------------------------------
    biasCorrection: Default true. Take aquisition with receiver bias
    correction. If set to false measurements will be faster. Receiver bias
    correction must be performed periodically. (e.g. 1 out of every 100
    readings).
    lidarliteAddress: Default 0x62. Fill in new address here if changed. See
    operating manual for instructions.
  */
  if (incre == 0)
  {
    distance = lidarLite.distance();
  }
  else distance = lidarLite.distance(false);
  incre++;
  incre %= 10;

  //distance= lidarLite.distance();
}
// -- Move car  forward/backward --
void MoveTo(int val)
{
  if (val == 1) // move forward
  {

    digitalWrite(motorOne, HIGH);
    digitalWrite(motorTwo, HIGH);
  }
  else if (val == -1) // move backward
  {
    digitalWrite(motorOne, LOW);
    digitalWrite(motorTwo, LOW);
  }
  analogWrite(speedOne, 250);
  analogWrite(speedTwo, 250);
}
// reset pin
void resetLeft()
{
  digitalWrite(motorOne, 0);
  analogWrite(speedOne, 0);
}
void resetRight()
{
  digitalWrite(motorTwo, 0);
  analogWrite(speedTwo, 0);

}

/**
   Turning functions PASSED
   Turn: syntax Turn|turn|Turn:<int number>;
 * ** readDirection() -> update distance of variable currentDirection
*/
void Turn(long inputDirection)
{

  long currentDirection, newDirection;
  // update currentDirection value
  readDirection(currentDirection);
  // calculate newDirection
  newDirection = currentDirection + inputDirection;
  // edge case for moving between quarter1 and quarter4 or vice versa
  if (newDirection < 0)
    newDirection = 360 + newDirection;
  else
    newDirection = newDirection % 360;
  if (inputDirection < 0)
  {
    //Serial.println("CCW");
    while (1)
    {
      if (abs(currentDirection - newDirection) >= 0 && abs(currentDirection - newDirection) <= 1)
      {

        resetLeft();
        resetRight();
        break;

      }
      digitalWrite(motorOne, LOW);
      analogWrite(speedOne, 200);
      digitalWrite(motorTwo, HIGH);
      analogWrite(speedTwo, 200);
      readDirection(currentDirection);
      lcd.setCursor(11, 0);
      lcd.print("       ");
      lcd.setCursor(11, 0);
      lcd.print(abs(currentDirection - newDirection));
    }
  }
  else {
    //Serial.println("CW");
    while (1)
    {
      if (abs(currentDirection - newDirection) >= 0 && abs(currentDirection - newDirection) <= 1)
      {
        resetLeft();
        resetRight();
        break;
      }
      digitalWrite(motorOne, HIGH);
      analogWrite(speedOne, 200);
      digitalWrite(motorTwo, LOW);
      analogWrite(speedTwo, 200);
      //Serial.println(abs(currentDirection - newDirection));
      readDirection(currentDirection);
      lcd.setCursor(11, 0);
      lcd.print("       ");
      lcd.setCursor(11, 0);
      lcd.print(abs(currentDirection - newDirection));
    }
  }


}


// compass reading 16bit
void readDirection(long & degree) {

  // Setup timeout parameter
  int timeout = COMMUNICATION_TIMEOUT;

  // Begin communication with CMPS14
  Wire.beginTransmission(_i2cAddress);

  // Tell register you want some data
  Wire.write(BEARING_Register);

  // End the transmission
  int nackCatcher = Wire.endTransmission();

  // Return if we have a connection problem
  if (nackCatcher != 0) {
    bearing =  0;
    return;
  }

  // Request 2 bytes from CMPS14
  Wire.requestFrom(_i2cAddress , TWO_BYTES);

  // Wait for the bytes to arrive.
  // Don't wait forever as this will hang the whole program
  while ((Wire.available() < TWO_BYTES) && (timeout-- > 0))
    delay(1);

  // Timed out so return
  if (timeout <= 0) {
    bearing = 0;
  }

  // Read the values
  _byteHigh = Wire.read(); _byteLow = Wire.read();
  bearing = ((_byteHigh << 8) | _byteLow) ;
  degree = ((long)bearing * 360) / 3599;
  if (degree == 0 || degree == 360)
    direct = "N";
  else if (degree > 0 && degree < 90)
    direct = "NW";
  else if (degree > 90 && degree < 180)
    direct = "SW";
  else if (degree > 180 && degree < 270)
    direct = "SE";
  else if (degree > 270 && degree < 360)
    direct = "NE";
  else if (degree == 90)
    direct = "W";
  else if (degree == 180)
    direct = "S";
  else if (degree == 270)
    direct = "E";
  // Print data to Serial Monitor window

  lcd.setCursor(0, 3);
  lcd.print("Direction:");
  lcd.setCursor(11, 3);
  lcd.print("         ");
  lcd.setCursor(11, 3);
  lcd.print(degree);
  //Serial.println(direct);
}




/**
   Encoder pulse counting for motor 1/2

*/
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
