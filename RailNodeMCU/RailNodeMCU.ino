#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>


#define FIREBASE_HOST "railproject-bf850-default-rtdb.asia-southeast1.firebasedatabase.app"   //Database link
#define FIREBASE_AUTH "Pbxwh9dfkKjzU8LtJMtQ0OBt6d1wZNp4HQMtwOwd"  //Database secrate

const String WIFI_SSID = "PAPPURAJ";
const String WIFI_PASSWORD = "0000000.";

FirebaseData firebaseData;



void initFire(){
  pinMode(D4,OUTPUT);
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(D4,0);
    Serial.print(".");
    delay(200);
    digitalWrite(D4,1);
    Serial.print(".");
    delay(200);
    
  }
  
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();


  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}




void setup() {
  Serial.begin(9600);
  pinMode(D0,INPUT);
  pinMode(D1,INPUT_PULLUP);
  pinMode(D5,OUTPUT);
  pinMode(D6,OUTPUT);
  pinMode(D3,OUTPUT);
  
  
  initFire();

  delay(500);
}




int fire=0,duration=5;

void loop() {


    if(!digitalRead(D0)){
      digitalWrite(D5,1);
      digitalWrite(D6,1);
      digitalWrite(D3,1);
      if(!fire){
         Firebase.setString(firebaseData,"/Danger","1");
         fire=1;
      }

      for(int i=0;i<duration;i++)
        delay(1000);
      
    }else if(digitalRead(D1)){
       digitalWrite(D5,0);
       digitalWrite(D6,1);
       digitalWrite(D3,0);
       if(!fire){
         Firebase.setString(firebaseData,"/Danger","2");
         fire=2;
      }
    }else{
      
      digitalWrite(D5,0);
       digitalWrite(D6,0);
       digitalWrite(D3,0);
      if(fire){
         Firebase.setString(firebaseData,"/Danger", "0");
         fire=1;
         fire=0;
   }
  }
}
