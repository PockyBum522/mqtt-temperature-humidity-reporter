#include <Arduino.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <PID_v1.h>
#include <sstream>
#include <ETH.h>
#include <AsyncElegantOTA.h>
#include "models/ThermostatStatus/ThermostatStatus.h"
#include "models/PinDefinitions.h"
#include "secrets/SECRETS.h"
#include "logic/MqttLogistics.h"
#include "logic/Sht3xHandler.h"
#include "logic/TemperatureReporter.h"
#include "logic/DebugMessageSender.h"


#define DEBUG_ETHERNET_WEBSERVER_PORT Serial

bool initialSettingsSet = false;

AsyncWebServer webServer(80);
WiFiClient ethernetClient;

ThermostatStatus currentStatus = *new ThermostatStatus();

MqttLogistics mqttLogistics = *new MqttLogistics(&currentStatus, &ethernetClient);

auto temperatureReportSender = *new TemperatureReporter(&currentStatus, &mqttLogistics);
auto debugMqttSender = *new DebugMessageSender(&currentStatus, &mqttLogistics);
auto temperatureSensorHandler = *new Sht3xHandler(&currentStatus);

void setup()
{
    pinMode(PIN_OUTPUT_01, OUTPUT);
    pinMode(PIN_OUTPUT_02, OUTPUT);
    pinMode(PIN_OUTPUT_03, OUTPUT);
    pinMode(PIN_OUTPUT_04, OUTPUT);
    pinMode(PIN_OUTPUT_05, OUTPUT);
    pinMode(PIN_OUTPUT_06, OUTPUT);

    // Turn all pins off
    digitalWrite(PIN_OUTPUT_01, STATE_OFF);
    digitalWrite(PIN_OUTPUT_02, STATE_OFF);
    digitalWrite(PIN_OUTPUT_03, STATE_OFF);
    digitalWrite(PIN_OUTPUT_04, STATE_OFF);
    digitalWrite(PIN_OUTPUT_05, STATE_OFF);
    digitalWrite(PIN_OUTPUT_06, STATE_OFF);

    Serial.begin(115200);

    ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);

    // Static IP, leave without this line to get IP via DHCP
    // ETH.config(myIP, myGW, mySN, myDNS);

    webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(200, "text/plain", "Hi! I am WT32_ETH01_Thermostat.");
    });

    AsyncElegantOTA.begin(&webServer);    // Start ElegantOTA
    webServer.begin();

    Serial.println();
    Serial.println("V03_HTTP server started with MAC: " + ETH.macAddress() + ", at IPv4: " + ETH.localIP().toString());
    Serial.println();
}

void loop()
{
   long startMillis = millis();

    temperatureSensorHandler.loop();

    mqttLogistics.reconnectMqttIfNotConnected();
    mqttLogistics.loopClient();

    temperatureReportSender.SendTemperatureReportEveryTimeout();
    debugMqttSender.SendMqttDebugMessagesEveryTimeout();

    currentStatus.LoopCounter++;
}