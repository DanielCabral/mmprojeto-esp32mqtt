#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#include <PubSubClient.h>
#include "WiFi.h"
#include <WiFiMulti.h>

//2. Include Firebase ESP32 library (this library)
#include "FirebaseESP32.h"

//3. Declare the Firebase Data object in the global scope
FirebaseData firebaseData;


  WiFiMulti wifiMulti;
  const char* ssid = "ALINHACAR";
  const char* password = "12345678";
const char* mqttServer = "soldier.cloudmqtt.com";
const int mqttPort = 17358;
const char* mqttUser = "xgfuilnu";
const char* mqttPassword = "ww8nPsHUbbJ-";
int LED_BUILTIN = 2;
WiFiClient espClient;
PubSubClient client(espClient);


// Initialize Telegram BOT
#define BOTtoken "1448674545:AAFqM_fO_4L3WqTxSxzf3VB7LvmPgSDLgQU"  // your Bot Token (Get from Botfather)

WiFiClientSecure client2;
UniversalTelegramBot bot(BOTtoken, client2);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

long lastMsg;
void setup() {
    wifiMulti.addAP("ALINHACAR", "12345678");
  
    pinMode (LED_BUILTIN, OUTPUT);
    WiFi.begin(ssid, password);
    Serial.begin(115200);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print("Connecting to WiFi:");
        Serial.println(ssid);
    }

    Serial.println("Connected to the WiFi network");
    Serial.println("");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);

    while (!client.connected()) {
        Serial.println("Connecting to MQTT…");
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), mqttUser, mqttPassword )) {
            Serial.println("connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }

    Serial.print("Tentando enviar a mensagem");
    client.publish("esp∕test", "Hello from ESP32");
    client.subscribe("esp/test");

    Firebase.begin("https://mm-projeto-final.firebaseio.com/", "pG4Qb4IZyGJug9fWPuOwfjLiqw6pkjOrMG4xRnyt");

}

void callback(char* topic, byte* payload, unsigned int length) {

    Serial.print("Message arrived in topic: ");
    Serial.println(topic);

    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }

    if (payload[0] == '0'){
        Serial.println("Desligando luz");
        digitalWrite(LED_BUILTIN, LOW);
    }

    if (payload[0] == '1'){
        Serial.println("Ligando luz");
        digitalWrite(LED_BUILTIN, HIGH);
    }

    Serial.println();
    Serial.println(" — — — — — — — — — — — -");

}

void loop() {
    client.loop();
   
  long now = millis();
  if (now - lastMsg > 6000) {
    lastMsg = now;
  double temp= (temprature_sens_read() - 32) / 1.8;
    Serial.print(temp);
  Serial.println(" C");
  char buffer[10];
  String stringOne =  String(temp);
  String path = "/Esp32/TemperatureInternal";
  //Firebase.getInt(firebaseData, "/red");
  Firebase.pushString(firebaseData, path, stringOne); 
//      if (Firebase.failed()) {
  //    Serial.print("pushing /logs failed:");
    //  Serial.println(Firebase.error());  
  client.publish("temp", stringOne.c_str());
  }

  if (millis() > Bot_lasttime + Bot_mtbs)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      for (int i=0; i<numNewMessages; i++) {
        if(bot.messages[i].text == "/temp"){
          double temp= (temprature_sens_read() - 32) / 1.8;
          String stringOne =  String(temp)+'º';
          bot.sendMessage(bot.messages[i].chat_id, stringOne.c_str(), "");
        }
      }
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Bot_lasttime = millis();
  }
}
