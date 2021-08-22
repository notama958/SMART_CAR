#include <Ethernet.h>

/*

  ESP Steering wheel example 23.2.2021 KN
  Lidar reading

*/

// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Wire.h>

 

//            network credentials



const char *ssid = "Your_WIFI";
const char *password = "Your_PW";


bool    spiffsActive = false;


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


String Lidar = ""; // lidar distance value in String format
String Degree = "";
int is;
char buf[40];



void setup()
{
  // Serial port for debugging purposes
  Serial.begin(9600);

  Serial.println("");

  Serial.println("----------------");

  Serial.println("----------------");

  // Start filing subsystem
  if (SPIFFS.begin()) {
    Serial.println("SPIFFS Active");
    Serial.println();
    spiffsActive = true;
  } else {
    Serial.println("Unable to activate SPIFFS");
  }


  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());


  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/index.html", String(), false, processor);

    //             Serial.println(" HTML GET ");

    int paramsNr = request->params();

    //             Serial.println(" ------ ");
    //             Serial.print("Param qty: ");
    //             Serial.println(paramsNr);

    for (byte i = 0; i < paramsNr; i++)
    {

      AsyncWebParameter* p = request->getParam(0);

      //                      Serial.print("Param name: ");
      //                      Serial.println(p->name());

      if ((p->name()) == "dist") {
        Serial.print("Move:");
        Serial.print(p->value());
        Serial.println(";");

      };
      if ((p->name()) == "dire") {
        Serial.print("Turn:");
        Serial.print(p->value());
        Serial.println(";");

      };

      //                   Serial.print("Param value: ");
      //                   Serial.println(p->value());
    };  //    end of for loop


  });   // server on END



  // Route to load style.css file
  server.on("/js/jquery-1.11.3.min.js", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/js/jquery-1.11.3.min.js", "text/javascript");

    Serial.println(" JS jquery GET ");

  });

  // Route to load style.css file
  server.on("/js/roundslider.min.js", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/js/roundslider.min.js", "text/javascript");

    Serial.println(" min JS GET ");

  });
  // Route to load index.js file
  server.on("/js/index.js", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/js/index.js", "text/javascript");

    Serial.println(" index JS GET ");

  });

  // Route to load style.css file
  server.on("/css/roundslider.min.css", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/css/roundslider.min.css", "text/css");

    Serial.println(" min css GET ");

  });

  // Route to load index.css file
  server.on("/css/index.css", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/css/index.css", "text/css");

    Serial.println(" index css GET ");

  });

  // Route to load car.png file
  server.on("/img/car.png", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/img/car.png", "text/image");

    Serial.println(" index image GET ");

  });

  //      distance query (Lidar) from web page tag is /Lid_dist to send Lidar value
  server.on("/Lid_dist", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", getDist().c_str()); // return value to web page JS

  });
  //      distance query (Degree) from web page tag is /Deg_dist to send Degree value
  server.on("/Deg_dist", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", getDeg().c_str()); // return value to web page JS

  });

  // Start server
  server.begin();

} //  END OF setup



// reservation
String processor(const String& var)
{  }


// get method for getting Lidar and Degree from Arduino
String getDist()
{
  return Lidar;
}
String getDeg()
{
  return Degree;
}



void loop()
{
  // parse the command from Lidar and Compass sensor
  if (Serial.available() > 0)
  {
    serial_read();
    whatCmd(buf); // Parsing the command
    Serial.println(buf);
    //Serial.println(Lidar);
  }
}

void whatCmd(char *buf) {
  String title = "";
  String value = "";
  String *ptr = NULL;
  boolean readValue = false;
  for (int i = 0; i < strlen(buf); ++i)
  {
    if (buf[i] == ':')
    {
      // checking which command is sedning out from UI
      // assign the address of the variable to ptr
      // thus to change the value after parsing and out of the loop
      if (title == "lidar" || title == "Lidar" || title == "LIDAR")
      {
        ptr = &Lidar;
      }
      else
        ptr = &Degree;
      readValue = true;
      continue;
    }
    if (readValue)
    {
      value += buf[i];
    }
    else
      title += buf[i];
  }
  // after parse out the value from the command syntax
  // change the value of the ptr pointing to the desired value
  *ptr = value;
}



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
        buf[is] = t;
        // Serial.println(int(t));
        is++;
      }

      if (t == 10) {         // check if end of message  LF
        buf[is] = 0; // end of string
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

}   // end of func
