#include <LGPS.h>
#include <LGPRS.h>
#include <LGPRSClient.h>
#include <LGPRSServer.h>
#include <LGPRSUdp.h>
#include <LBattery.h>

#include <Wire.h>

//#include <LTask.h>

#include <PubSubClient.h>
#include <ATT_IOT.h>

int location_GPS = 2;
int map_GPS = 10;
int hasFix_GPS = 3;

char buff[256];

float lat;
float lng;
float prevLat = -1;
float prevLng = -1;
unsigned int hasFix = 0;

unsigned long startTime = millis();

gpsSentenceInfoStruct info;  // instantiate

ATTDevice device("ZYeZlf0TlnlUPOh0ItwEa8nv", "maker:4SIGPzLBnbWOm0lqFwXF9MlLpKa14Ed7iLEeqr3");    // Create the object that provides the connection to the cloud to manager the device.
char httpServer[] = "api.allthingstalk.io";       // HTTP API Server host                  
char mqttServer[] = "api.allthingstalk.io";		    // MQTT Server Address

// Required for the device
void callback(char* topic, byte* payload, unsigned int length);
void connectToPlatform();
unsigned long getTime();
float calc_dist(float flat1, float flon1, float flat2, float flon2);
LGPRSClient c;
PubSubClient pubSub(mqttServer, 1883, callback, c);

void setup()
{
  Serial.begin(115200);
  //while(!Serial);               // for the linkit, we need to wait until the serial monitor is initialized correctly, if we don't do this, we don't get to see the logging.
                                  // Warning for battery and mains power usage, comment out the above line, otherwise the setup() will continue to wait for the Serial to become available
  Serial.println("Starting");
  
  while(!LGPRS.attachGPRS("internet.be", "",""))
  {
    Serial.println("Connecting");
    delay(200);
  }
  Serial.println("Connected");

  LGPS.powerOn();                                       // Start the GPS first as it takes time to get a fix
  Serial.println("GPS Powered on and waiting.");
  delay(200);

  Serial.println("\nTime since startup for setup: " + String(getTime()) + " ms");
}

unsigned long time;
String locLatestVal = "";
float dlay = 1000;              // delay till next fix in milliseconds
                                // start at 3 seconds. When we get a fix, delay is upped to 60 seconds
unsigned int minDistance = 20;  // minimum distance between fixes
bool connected = false;
bool firstFix = false;

void loop()
{  
  unsigned long curTime = millis();
  if (curTime > (time + dlay))                                           // Publish data reading every 15 seconds
  {
    LGPS.getData(&info);                        // Get a GPS fix
    hasFix = ParseLocation((const char*)info.GPGGA);

    if(hasFix)  // This is where we break out needed location information
    {
      dlay = 60000;  // 1 minute delay

      // print time to first fix
      if(!firstFix)
      {
        firstFix = true;
        Serial.println("\nTime since startup to first fix: " + String(getTime()) + " ms\n");
      }

      // check if we already have a connection with the platform
      if(!connected)
        connectToPlatform();

      if(prevLat != -1)     // We have a previous point (so we can calculate distance to our current point)
      {
        float dist = calc_dist(lat, lng, prevLat, prevLng);
        Serial.println("Distance: " + String(dist,2));
        if(dist > minDistance)   // Send coordinates of points are at least xx meters apart
        {
          sendCoordinates();  
          
          prevLat = lat;    // Update coordinates
          prevLng = lng;
        }
      }
      else  // send first fix
      {
        sendCoordinates();

        prevLat = lat;
        prevLng = lng;
      }
    }
    else
    {
      dlay = 1000;  // smaller delay if we don't have a fix
    }
    Serial.println("Delay: " + String(dlay));
    time = curTime;   // Update time
  }
 
  if(connected)
    device.process(); 
}

boolean ParseLocation(const char* GPGGAstr)
// Refer to http://www.gpsinformation.org/dale/nmea.htm#GGA
// Sample data: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
{
  char latarray[6];
  char lngarray[6];
  int index = 0;
  Serial.println(GPGGAstr);
  Serial.print("Fix Quality: ");
  Serial.println(GPGGAstr[43]);
  if (GPGGAstr[43]=='0')        //  This is the place in the sentence that shows Fix Quality 0 means no fix
  {
    Serial.println("No GPS Fix");
    return 0;
  }
  String GPSstring = String(GPGGAstr);

  /**
   * parse Latitude
   */
  for (int i=20; i<=26; i++)           // We have to jump through some hoops here
  {
    latarray[index] = GPGGAstr[i];     // we need to pick out the minutes from the char array
    index++;
  }
  float latdms = atof(latarray);            // and convert them to a float
  float lattitude = latdms/60;              // and convert that to decimal degrees
  String lattstring = String(lattitude,6);  // Then put back into a string

  lat = (GPSstring.substring(18,20) + "." + lattstring.substring(2,8)).toFloat();  // latitude

  if(GPGGAstr[28] == 'S')   // negative in southern hemisphere
    lat = -lat;

  /**
   * parse Longitude
   */
  index = 0;
  for (int i=33; i<=38; i++)            // And do the same thing for longitude
  {
    lngarray[index] = GPGGAstr[i];     // the good news is that the GPS data is fixed column
    index++;
  }
  float lngdms = atof(lngarray);          // and convert them to a float
  float lngitude = lngdms/60;             // and convert that to decimal degrees
  String lngstring = String(lngitude,6);  // Then put back into a string

  lng = (GPSstring.substring(30,33) + "." + lngstring.substring(2,8)).toFloat();  // longitude   

  if(GPGGAstr[41] == 'W')   // negative in western hemisphere
    lng = -lng;

  Serial.println("Lat: " + String(lat,6));
  Serial.println("Lng: " + String(lng,6));

  if(lat != 0 && lng != 0)
    return 1;
  else
    return 0;
}

// get duration since last call on screen
unsigned long getTime()
{
  return millis() - startTime;
}

/*************************************************************************
 * Send coordinates
 *************************************************************************/
void sendCoordinates()
{
  String Location = String(lat,6) + "," + String(lng,6);
  String coor = "{'lat':" + String(lat,6) + ",'long':" + String(lng,6) + "}";
  device.send(Location, String(location_GPS));   // as String
  device.send(coor, String(map_GPS));            // as numbers in a complex json
  Serial.println("Coordinates: " + Location);
}

/*************************************************************************
 * Connect to SmartLiving
 *************************************************************************/
void connectToPlatform()
{
  if(device.connect(&c, httpServer))  // connect the device with the IOT platform.
  {
    device.addAsset(String(location_GPS), "location", "location", "sensor", "string");
    device.addAsset(String(map_GPS), "map", "realtime map", "sensor", "{\"type\": \"object\", \"properties\": {\"lat\": {\"type\": \"number\"},\"long\": {\"type\": \"number\"}}}");
    device.subscribe(pubSub);
    connected = true;
  }
  else 
    while(true);

  Serial.println("\nTime since startup to connect to platform: " + String(getTime()) + " ms\n");
}

/*************************************************************************
 * Function to calculate the distance between two waypoints
 *************************************************************************/
float calc_dist(float flat1, float flon1, float flat2, float flon2)
{
  float dist_calc = 0;
  float dist_calc2 = 0;
  float diflat = 0;
  float diflon = 0;

  // I've to split all the calculation in several steps. If I try to do it in a single line the arduino will explode.
  diflat = radians(flat2-flat1);
  flat1 = radians(flat1);
  flat2 = radians(flat2);
  diflon = radians((flon2)-(flon1));

  dist_calc = (sin(diflat/2.0)*sin(diflat/2.0));
  dist_calc2 = cos(flat1);
  dist_calc2 *= cos(flat2);
  dist_calc2 *= sin(diflon/2.0);
  dist_calc2 *= sin(diflon/2.0);
  dist_calc += dist_calc2;

  dist_calc = (2*atan2(sqrt(dist_calc),sqrt(1.0-dist_calc)));
  dist_calc *= 6371000.0; // Converting to meters

  return dist_calc;
}

void callback(char* topic, byte* payload, unsigned int length) 
{
}