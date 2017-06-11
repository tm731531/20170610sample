#include "motoWiFiEsp.h"
#include <motoTM1637.h>

#include <SoftwareSerial.h>
TM1637 tm1637led_2_3(2,3);
long  CurrentTime;
long  PreTime;
long  CurrentTimeIFTTT;
long  PreTimeIFTTT;
long  IntervalTime;
long  IntervalTimeIFTTT;
SoftwareSerial esp8266_Serial(A5,A4);

WiFiEspClient esp_client;

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
   connect_status = WiFi.begin("wifiname","password");
  }
  Serial.println(ipAddressToString(WiFi.localIP()));
  if (esp_client.connect("api.thingspeak.com", 80)) {
     String  things_request = "GET /update?api_key=thingspeakKey&field1="+ String(analogRead(A0))+"&field2="+ String(analogRead(A3))+"\r\n\r\n";
       esp_client.print(things_request);
       esp_client.stop();
  }
  Serial.println(" Send thingspeakKey");
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  
  pinMode(10, OUTPUT);
  pinMode(5, OUTPUT);
  CurrentTime = 0;
  PreTime = 0;
  IntervalTime = 60000;
  IntervalTimeIFTTT=59000;

}


void loop()
{
    CurrentTime = millis();
    CurrentTimeIFTTT= millis();
    tm1637led_2_3.display(analogRead(A0));
    // if (analogRead(A0) > 500 && analogRead(A3)<500) {
    if (analogRead(A3)< analogRead(A0)) {
       sendTOIFTTT(2);
       sendTOthingspeak(2); 
       digitalWrite(10,HIGH);
       analogWrite(5,255);
    }  
    
   
    else{
       sendTOIFTTT(1);
      sendTOthingspeak(1);
    digitalWrite(10,LOW);
       analogWrite(5,0);

      }
    Serial.println("A0"+String(analogRead(A0))+"A3"+String(analogRead(A3)));
 
    
   
}
void mainLoop(){
  
  }
void sendTOthingspeak(long x) {
    if(CurrentTime-PreTime>=IntervalTime){
     
    
   PreTime=CurrentTime;
    if (esp_client.connect("api.thingspeak.com", 80)) {
     String  things_request = "GET /update?api_key=thingspeakKey&field1="+ String(analogRead(A0))+"&field2="+ String(analogRead(A3))+"\r\n\r\n";
       esp_client.print(things_request);
       esp_client.stop();
   }
   
   }
}

void sendTOIFTTT(long x) {
  if(CurrentTimeIFTTT-PreTimeIFTTT>=IntervalTime){
    if (esp_client.connect("maker.ifttt.com", 80)) {
         String data = "\r\n{\"value1\":"+ String(analogRead(A0))+",\"value2\": "+String(analogRead(A3))+",\"value3\": "+String(x)+"}";
         esp_client.println("POST /trigger/WaterFull/with/key/iftttKey HTTP/1.1");
         esp_client.println("Host: maker.ifttt.com");
         esp_client.println("User-Agent: Arduino");
         esp_client.println("Accept: */*");
         esp_client.print("Content-Length: ");
         esp_client.println(data.length());
         esp_client.println("Content-Type: application/json");
         esp_client.println("Connection: close");
         esp_client.println(data);
          esp_client.stop();
    }
    PreTimeIFTTT=CurrentTimeIFTTT;
  }
}
