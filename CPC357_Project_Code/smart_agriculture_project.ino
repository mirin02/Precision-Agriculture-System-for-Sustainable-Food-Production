#include "VOneMqttClient.h"
#include "DHT.h"

int MinMoistureValue = 4095;
int MaxMoistureValue = 1800;
int MinMoisture = 0;
int MaxMoisture = 100;
int Moisture = 0;

// Threshold for activating the water pump
const int waterThreshold = 30; // Adjust as needed

// Define device IDs
const char* DHT11Sensor = "6412e3e7-a9d5-4706-bee9-d14ba88351eb"; 
const char* MoistureSensor = "07e3b717-5bb1-43b3-9caa-c840cb702326"; 

// Used Pins
const int dht11Pin = 22;
const int moisturePin = 34;

// Water pump pins (adjust based on your wiring)
const int pumpPin = 32; 

// Input sensor
#define DHTTYPE DHT11
DHT dht(dht11Pin, DHTTYPE);

// Create an instance of VOneMqttClient
VOneMqttClient voneClient;

// last message time
unsigned long lastMsgTime = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  setup_wifi();
  voneClient.setup();
  dht.begin();
  pinMode(pumpPin, OUTPUT); // Set pump pin as output
  Serial.begin(115200); // Initialize serial communication
}

void loop() {
  if (!voneClient.connected()) {
    voneClient.reconnect();
    String errorMsg = "DHTSensor Fail";
    voneClient.publishDeviceStatusEvent(DHT11Sensor, true);
    voneClient.publishDeviceStatusEvent(MoistureSensor, true); 
  }
  voneClient.loop();

  unsigned long cur = millis();
  if (cur - lastMsgTime > INTERVAL) {
    lastMsgTime = cur;

    // Publish telemetry data 1
    float h = dht.readHumidity();
    int t = dht.readTemperature();

    JSONVar payloadObject;
    payloadObject["Humidity"] = h;
    payloadObject["Temperature"] = t;
    voneClient.publishTelemetryData(DHT11Sensor, payloadObject);

    // Publish telemetry data 3
    int sensorValue = analogRead(moisturePin);
    Moisture = map(sensorValue, MinMoistureValue, MaxMoistureValue, MinMoisture, MaxMoisture);
    voneClient.publishTelemetryData(MoistureSensor, "Soil moisture", Moisture);

    // Activate water pump if moisture level is below threshold
    if (Moisture < waterThreshold) {
      digitalWrite(pumpPin, HIGH); // Turn on the pump
      Serial.print("Moisture: ");
      Serial.print(Moisture);
      Serial.print("%, ");
      Serial.println("Pump ON"); 
    } else {
      digitalWrite(pumpPin, LOW); // Turn off the pump
      Serial.print("Moisture: ");
      Serial.print(Moisture);
      Serial.print("%, ");
      Serial.println("Pump OFF");
    }
  }
}