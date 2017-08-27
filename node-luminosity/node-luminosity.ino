#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string.h>;

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

// Update these with values suitable for your network.
//const char* ssid = "elpropio";
//const char* password = "d798fd798f";
const char* ssid = "LAS CHICHIS";
const char* password = "140805092105";
const char* mqtt_server = "192.168.1.11";
const char* clientID = "lumen1";
const char* sensor_name = "";
const char* separator = ":";

// WIFI and Comunication
WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];
char data[50];
float lux; //Stores luminosity value


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topico, byte* payload, unsigned int length) {
  // Conver the incoming byte array to a string
  payload[length] = '\0'; // Null terminator used to terminate the char array
  String message = (char*)payload;
  String topic = String(topico);

  Serial.print("Message arrived on topic: [");
  Serial.print(topic);
  Serial.print("], ");
  Serial.println(message);

  if(topic == "testing" && message == "testing"){
    client.publish("testing-response", "alive");
  }

  if(topic == "luminosity" && message == "data"){
   // -------- SENSOR 1 -------------------------
    memset(msg, 0, sizeof(msg));
    memset(data, 0, sizeof(data));
    /* Get a new sensor event */ 
    sensors_event_t event;
    tsl.getEvent(&event); 
    /* Display the results (light is measured in lux) */
    if (event.light)
    {
      lux = event.light;
    }
    else
    {
      lux = 0;
    }    
    strcat(msg, clientID);
    strcat(msg, separator);
    sensor_name = "l1";
    strcat(msg, sensor_name);
    dtostrf(lux , 3, 2, data);
    strcat(msg, separator);
    strcat(msg, data);
    client.publish("luminosity-response", msg);
    // -------------------------------------------
    
    // -------- SENSOR 2 -------------------------
    memset(msg, 0, sizeof(msg));
    memset(data, 0, sizeof(data));
    float volts = analogRead(A0) * 5.0 / 1024.0;
    lux = 2.0*(100*volts);
    strcat(msg, clientID);
    strcat(msg, separator);
    sensor_name = "lout";
    strcat(msg, sensor_name);
    dtostrf(lux , 3, 2, data);
    strcat(msg, separator);
    strcat(msg, data);
    client.publish("luminosity-response", msg);
    // -------------------------------------------
  }
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("conexions", clientID);
      // ... and resubscribe
      client.subscribe("testing");
      client.subscribe("luminosity");
      // ---------------- Subscruptions
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/**************************************************************************/
/*
    Configures the gain and integration time for the TSL2561
*/
/**************************************************************************/
void configureSensor(void)
{
  /* You can also manually set the gain or enable auto-gain support */
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
  
  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

}

/**************************************************************************/
/*
    Arduino setup function (automatically called at startup)
*/
/**************************************************************************/
void setup(void) 
{
  Wire.begin(D1, D2); // sda, scl
  Serial.begin(115200);
  Serial.println("Light Sensor Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!tsl.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
    
  /* Setup the sensor gain and integration time */
  configureSensor();
  
  /* We're ready to go! */
  Serial.println("");

  // MQTT AND CONECTIONS SETUP
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

/**************************************************************************/
/*
    Arduino loop function, called once 'setup' is complete (your own code
    should go here)
*/
/**************************************************************************/

void loop() {     
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
