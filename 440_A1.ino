
/* This code reports the current location of the International Space Station (ISS) and the time 
 *  the code is run and report the latitude and longitude (along with the time of the request in unix
 *  time stamp format) to the serial monitor. It should then find the city closest to this latitude
 *  and longitude, reporting the city's name, country, latitude, and longitude to the serial monitor.
 *  
 *  Device Used: Adafruit Feather HUZZAH ESP8266
 *  ISS API: http://api.open-notify.org/iss-now.json
 *  Geolocation API: http://api.geonames.org
*/

#include <Time.h>                   //include the math and time libraries
#include <TimeLib.h>
#include <Math.h>
#include <ESP8266WiFi.h>            //include the library for the device
#include <ESP8266HTTPClient.h>      //include the library for the wifi connectivity component
#include <ArduinoJson.h>                   //provides the ability to parse and construct JSON objects
const char* ssid = "USERNAME";           //sets the wifi network you are connecting to
const char* pass = "PASSWORD";      //sets the password to connect to the wifi network
const char* username = "lauradickinson";   //sets the username of the account used to gain access to the geonames data

//constucts a blank space for the variables wanted from the ISS data to fill in later called StaData
typedef struct {
   String ts;
   int lt;
   int ln;
} StaData;

//constucts a blank space for the variables wanted from the geonames data to fill in later called LocData
typedef struct {
    String cy;
    String lt;
    String ln;
    String cn;
} LocData;

//creates an instance of StaData called station
StaData station;

//creates an instance of LocData called location
LocData location;

//set up serial monitor, wifi connection, and get and parse through desired api data
void setup() {
  Serial.begin(115200);                                   //sets the serial monitor speed
  delay(10);                                              //wait for 10 milliseconds
  Serial.print("Connecting to "); Serial.println(ssid);   //print the we are connecting to the wifi network
  WiFi.mode(WIFI_STA);                                    //we are connecting from an access point as a station
  WiFi.begin(ssid, pass);                                 //connect to the wifi using the id and password
  while (WiFi.status() != WL_CONNECTED) {                 //while the wifi is working but we haven't connected yet
    delay(500);                                           //wait a half second
    Serial.print(".");                                    //print a dot
  }
  
  Serial.println(); Serial.println("WiFi connected"); Serial.println();   //print the wifi is connected
  Serial.print("Your ESP has been assigned the internal IP address ");    //print the words in the quotes
  Serial.println(WiFi.localIP());                                         //append with the device's assigned ip address
  
  //gets the data (parsed) for the ISS's current location
  getSta();                                                               

  //prints where the ISS is currently located and what time is what received (in unix time stamp form) 
  Serial.println("Currently, at " + station.ts);
  Serial.println();
  Serial.println("The International Space Station is located at " + String(station.lt) + " degrees by " + String(station.ln) + " degrees. ");

  //converts the latitude of the space station to the tenths place for use in getLoc
  float lat = station.lt;    
  int latCalc = (int) lat * 10.0; 
  lat = (float) latCalc / 10.0;   

  //converts the longitude of the space station to the tenths place for use in getLoc
  float lon = station.ln;
  int lonCalc = (int) lon * 10.0;
  lon = (float) lonCalc / 10.0;

  //uses the latitude and longitude of the ISS to find the nearest city, and prints this to the serial monitor
  getLoc(lat,lon);
  Serial.println("The closest city to this location is " + location.cy + ", " + location.cn + ", ");
  Serial.println("which is located at " + location.lt + "degrees by " + location.ln + "degrees.");

}

//code that executes repeatedly as the device has power
void loop() {
}

  
  //uses the ISS api to receive the current location of the ISS in latitude and longitude
  void getSta() {
    HTTPClient theClient;                                         //creating a miniwebsite to create and access our url from
    theClient.begin("http://api.open-notify.org/iss-now.json");
    int httpCode = theClient.GET();
    if (httpCode > 0) {                                           //if a valid http code is found
        if (httpCode == 200){                                     //and if the http code is one of successful connection
          DynamicJsonBuffer jsonBuffer;                           
          String payload = theClient.getString();                 //get the info from the miniwebsite
          JsonObject& root = jsonBuffer.parseObject(payload);     //make sure the info is in json format

          //testing if the parsing succeeds
          if (!root.success()){                                   //if the parsing to check for json formating is unsuccessful, tell the user
            Serial.println("parseObject() failed in getSta(). ");
            return;
          }

          //add the cooresponding values to their variable names
          station.ts = root["timestamp"].as<String>();
          station.lt = root["iss_position"]["latitude"].as<int>();    //used int to manipulate this number later
          station.ln = root["iss_position"]["longitude"].as<int>();   //used into to manipulate this number later

      //if the http code is negative, print that something went wrong
      } else {                                                        
        Serial.println("Something went wrong with connecting to the endpoint.");
      }
    }
  }

  //uses the location data (latitude and longitude, which were converted to type float)
  //to find the closest city to the ISS's current location using the geonames api
  void getLoc(float,float) {
    //creating a miniwebsite to create and access our url from
    HTTPClient theClient;
    //I get an error that my lat and lon variables are undeclared here, but they should be declared when they are used as parameters
    theClient.begin("http://api.geonames.org/findNearbyPlaceNameJSON?lat=" + String(lat) + "&lng=" + String(lon) + "&username=" + username);
    int httpCode = theClient.GET();
    if (httpCode > 0) {                                          //if a valid http code is found
        if (httpCode == 200){                                    //and if the http code is one of successful connection
          DynamicJsonBuffer jsonBuffer;
          String payload = theClient.getString();                //get the info from the mini website
          JsonObject& root = jsonBuffer.parseObject(payload);    //make sure the info is in json format

          //testing if the parsing succeeds
          if (!root.success()){                                   //if the parsing to check for json formating is unsuccessful, tell the user
            Serial.println("parseObject() failed in getLoc(). ");
            return;
          }

          //add the cooresponding values to their variable names
          location.cy = root["geoname"]["toponymName"].as<String>();
          location.lt = root["geoname"]["lat"].as<String>();
          location.ln = root["geoname"]["lng"].as<String>();
          location.cn = root["geoname"]["countryName"].as<String>();
          
        //if the http code is negative, print that something went wrong
        } else {
        Serial.println("Something went wrong with connecting to the endpoint.");
      }
    }
  }
    

  
  
