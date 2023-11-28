// ------------ Don't mind the code encapsulated here ------------
#include <WiFi.h>
#include "PubSubClient.h"

const char *ssid = "Wokwi-GUEST";
const char *password = "";

String stMac;
char mac[50];
char clientId[50];

WiFiClient espClient;
PubSubClient client(espClient);
// ------------ Don't mind the code encapsulated here ------------

// ------------ Change as needed below:
const char *mqttServer = "mqtt.flespi.io";                                                  // set mqtt broker, dont change
const char *mqtt_user = "9ZyBpVIyRCvGXSgw97QZjd6reDOf4s24hotu7ZQOs9jdPpAOkeCthyJj1lmUDeMI"; // change your token in flespi
const char *mqtt_password = "123";                                                          // change with your corresponding password below the token in flespi
int port = 1883;                                                                            // set mqtt port, dont change

// change here your lastnames
const char *topicFromNodered = "Butaslac/Rojo/fromNodered";
const char *topicFromWokwi = "Butaslac/Rojo/fromWokwi";

const char *msgOff = "OFF";
const char *msgOn = "ON";

const int ledPin = 2;     // change based on what pin you connect the LED
const int switchPin = 13; // change based on what pin you connect the Switch

int switchVal, prevVal;

void setup()
{
    Serial.begin(115200);
    randomSeed(analogRead(0));

    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    wifiConnect();

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.macAddress());
    stMac = WiFi.macAddress();
    stMac.replace(":", "_");
    Serial.println(stMac);

    client.setServer(mqttServer, port);
    client.setCallback(callback); // Listen to the incoming MQTT messages - change here the topic

    pinMode(ledPin, OUTPUT);
    pinMode(switchPin, INPUT);
}

// Connect to the virtual Wokwi wifi generated to create a VPN
void wifiConnect()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
}

void mqttReconnect()
{
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        long r = random(1000);
        sprintf(clientId, "clientId-%ld", r);
        if (client.connect(clientId, mqtt_user, mqtt_password))
        {
            Serial.print(clientId);
            Serial.println(" connected");
            client.subscribe(topicFromNodered);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

// What to do with the message (in our case, we want to turn control the Wokwi LED)
void callback(char *topic, byte *message, unsigned int length)
{
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String stMessage;

    for (int i = 0; i < length; i++)
    {
        Serial.print((char)message[i]);
        stMessage += (char)message[i];
    }
    Serial.println();

    // if receive ON/OFF message, change the LED state
    if (String(topic) == topicFromNodered)
    {
        Serial.print("Changing LED to ");
        if (stMessage == "ON")
        {
            Serial.println("on");
            digitalWrite(ledPin, HIGH);
        }
        else if (stMessage == "OFF")
        {
            Serial.println("off");
            digitalWrite(ledPin, LOW);
        }
    }
}

void loop()
{
    delay(10); // introduce some delay so as not to overwork the cpu

    // read the switch value, and only do something the first time it switched
    switchVal = digitalRead(switchPin);
    if (switchVal != prevVal)
    {

        Serial.print("Changing switch to ");
        if (switchVal == 1)
        {
            Serial.println("on");
            client.publish(topicFromWokwi, msgOn);
        }
        else if (switchVal == 0)
        {
            Serial.println("off");
            client.publish(topicFromWokwi, msgOff);
        }

        prevVal = switchVal;
    }

    if (!client.connected())
    {
        mqttReconnect();
    }
    client.loop();
}
