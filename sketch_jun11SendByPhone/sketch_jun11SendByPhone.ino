#include "motoWiFiEsp.h"
#include <motoTM1637.h>

#include <motoMQTTClient.h>
#include <SoftwareSerial.h>
TM1637 tm1637led_2_3(2,3);
long  CurrentTime;
long  PreTime;
long  CurrentTimeIFTTT;
long  PreTimeIFTTT;
long  IntervalTime;
long  IntervalTimeIFTTT;
SoftwareSerial esp8266_Serial(A5,A4);


//WiFiEspClient   esp_client_mqtt;
WiFiEspClient   esp_client_ifttt;
WiFiEspClient   esp_client_thingspeak;


MQTTClient mqtt_client;
int connect_status = WL_IDLE_STATUS;
int temp=0;
String ipAddressToString(const IPAddress& ipAddress ) {
       return String(ipAddress[0]) + String(".")+
       String(ipAddress[1]) + String(".")+
       String(ipAddress[2]) + String(".")+
       String(ipAddress[3]);
}

void setup()
{
  Serial.begin(9600);
  esp8266_Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(A3, INPUT);
  WiFi.init(&esp8266_Serial); 
  if(WiFi.status()==WL_NO_SHIELD) {
     Serial.println("Esp8266 module no present");
     while(true);
  }
  while(connect_status != WL_CONNECTED) {
     Serial.println("Connect to router...");
     connect_status = WiFi.begin("Sid","password");
  }
  Serial.println(ipAddressToString(WiFi.localIP()));
 
  Serial.println("Connected !");
  mqtt_client.subscribe("watering");
  
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(5, OUTPUT);
 
  setInternal();
}

void loop()
{
    CurrentTime = millis();
    CurrentTimeIFTTT= millis();
    tm1637led_2_3.display(analogRead(A0));
    float values=analogRead(A2)*0.49;
    String tempS= "Temp= "+String(values) +" Light= "+String(analogRead(A1));

      sendTOIFTTT(tempS);
    sendTOthingspeak();
    
 //mqtt_client.loop();
    
}

String getTopicData(String myTopic, String inTopic, String payload) {
  if(inTopic == myTopic)
     return payload;
  else
     return "";
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length)
{
  Serial.println(getTopicData("watering", topic, payload));
  if (getTopicData("watering", topic, payload) == "on") {
    analogWrite(5,255);
    digitalWrite(10,HIGH);

  } else if (getTopicData("watering", topic, payload) == "off") {
    analogWrite(5,0);
  }
}

void setInternal(){
  CurrentTime = 0;
  CurrentTimeIFTTT=0;
  PreTime = 0;
  PreTimeIFTTT = 0;
  IntervalTime = 900000;
  IntervalTimeIFTTT=59000;
  
}
void sendTOthingspeak() {
    if(CurrentTime-PreTime>=IntervalTime){
        PreTime=CurrentTime;

        float values=analogRead(A2)*0.49;
        if (esp_client_thingspeak.connect("api.thingspeak.com", 80)) {
            String  things_request = "GET /update?api_key=key&field1="+ String(analogRead(A0))+
                                                                         "&field2="+ String(analogRead(A3))+
                                                                         "&field3="+ String(values)+
                                                                         "&field4="+ String(analogRead(A1))+"\r\n\r\n";
            esp_client_thingspeak.print(things_request);
            esp_client_thingspeak.stop();
        }
         Serial.println(values);
 
   }
}

void sendTOIFTTT(String x) {
  if(CurrentTimeIFTTT-PreTimeIFTTT>=IntervalTime){
      float values=analogRead(A2)*0.49;
        
      //Serial.println(tempS);
      if (esp_client_ifttt.connect("maker.ifttt.com", 80)) {
        
         //  String data = "\r\n{\"value1\":"+ String(analogRead(A0))+",\"value2\": "+String(analogRead(A3))+",\"value3\": "+tempS +" }";
        // String data = "\r\n{\"value1\":"+ String(analogRead(A0))+",\"value2\": "+String(analogRead(A3))+",\"value3\":"+ String(x)+"}";
       //  esp_client_ifttt.println("POST /trigger/WaterFull/with/key/c5GgD-fri3adcKZ85TgJF9 HTTP/1.1");
       //String data = "\r\n{\"value1\":"+ String(analogRead(A0))+",\"value2\": "+String(analogRead(A3))+",\"value3\": "+x+"}";
          String data = "\r\n{\"value1\":"+ String(analogRead(A1))+",\"value2\": "+String(values)+",\"value3\": "+String(analogRead(A3))+"}";
          
         
         esp_client_ifttt.println("POST /trigger/SensorGetData/with/key/key HTTP/1.1");

       
         esp_client_ifttt.println("Host: maker.ifttt.com");
         esp_client_ifttt.println("User-Agent: Arduino");
         esp_client_ifttt.println("Accept: */*");
         esp_client_ifttt.print("Content-Length: ");
         esp_client_ifttt.println(data.length());
         esp_client_ifttt.println("Content-Type: application/json");
         esp_client_ifttt.println("Connection: close");
         esp_client_ifttt.println(data);
          esp_client_ifttt.stop();
    }
         Serial.println("IFTTT");
    PreTimeIFTTT=CurrentTimeIFTTT;
  }
}
