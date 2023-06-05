//DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>
const int oneWireBus = 4;   
const int RELAY_PIN = 27;
#define TEMPERATURE_THRESHOLD 31

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

//GY906
#include<Wire.h>
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

//Wifi connection
#include <WiFi.h>
#include <PubSubClient.h>

const char* mqtt_server = "52.14.168.247";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

const char* ssid = "Redmi9i";
const char* password = "qwertyuiop";


void setup_wifi() {
  delay(10);
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

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  Serial.println();
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
    }
    else if(messageTemp == "off"){
      Serial.println("off");
    }
}
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe("SurfaceTemperature");
      client.subscribe("RoomTemperature");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 seconds");
      delay(1000);
    }
  }
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  Serial.begin(115200);
  sensors.begin();
  pinMode(RELAY_PIN,OUTPUT);
  mlx.begin(); 
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
    digitalWrite(RELAY_PIN, HIGH); 
  }

uint8_t alertDetected = 0;

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // DS18B20
  sensors.requestTemperatures(); 
  Serial.println("DS18B20 readings");
  float surfaceTemperature = sensors.getTempCByIndex(0);
  float temperatureF = sensors.getTempFByIndex(0);
  Serial.print("Temperature : ");
  Serial.print(surfaceTemperature);
  Serial.print("ºC \t ");
  Serial.print(temperatureF);
  Serial.println("ºF");
  delay(1000);

  //GY906C
  Serial.println("GY906C readings");
  Serial.print("Ambient temperature = "); 
  float ambientTemp =  mlx.readAmbientTempC();
  Serial.print(ambientTemp);
  Serial.print("°C");      
  Serial.print("   ");
  Serial.print("Object temperature = "); 
  Serial.print(mlx.readObjectTempC()); 
  Serial.println("°C");
  Serial.print("Ambient temperature = ");
  Serial.print(mlx.readAmbientTempF());
  Serial.print("°F");      
  Serial.print("   ");
  Serial.print("Object temperature = "); 
  Serial.print(mlx.readObjectTempF()); 
  Serial.println("°F");

  delay(500);
//
  if (surfaceTemperature > TEMPERATURE_THRESHOLD)
  {
    digitalWrite(RELAY_PIN, LOW); // turn on Relay
    Serial.println("high");
    delay(1000);
    
  }
  else
  {
    digitalWrite(RELAY_PIN, HIGH); 
  }

  char ds18b20Readings[10] = {0};
  char gy906Readings[10] = {0};
  sprintf(ds18b20Readings,"%0.2f",surfaceTemperature);
  sprintf(gy906Readings,"%0.2f", ambientTemp);
  client.publish("SurfaceTemperature", ds18b20Readings);
  client.publish("RoomTemperature", gy906Readings);
}
