//final

#include "WiFi.h"
#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"           
#include "soc/rtc_cntl_reg.h"  
#include "driver/rtc_io.h"
#include <SPIFFS.h>
#include <FS.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#define CAMERA_MODEL_AI_THINKER // Has PSRAM

const char* ssid = "PAPPURAJ";
const char* password = "0000000.";

#define DATABASE_URL "railproject-bf850-default-rtdb.asia-southeast1.firebasedatabase.app" 


#define API_KEY "AIzaSyB4haoV7KkmB0Sv7LMX1qerCf3G1JK7ti4"

#define USER_EMAIL "pappuraj.duet@gmail.com"
#define USER_PASSWORD "12345678"

#define STORAGE_BUCKET_ID "railproject-bf850.appspot.com"

#define FILE_PHOTO "/data/photo.jpg"

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

boolean takeNewPhoto = true;

FirebaseData fbdo,db;
FirebaseAuth auth;
FirebaseConfig configF;


bool taskCompleted = false;

bool checkPhoto( fs::FS &fs ) {
  File f_pic = fs.open( FILE_PHOTO );
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}

void capturePhotoSaveSpiffs( void ) {
  camera_fb_t * fb = NULL; // pointer
  bool ok = 0; 
  do {
    
  digitalWrite(4, 1);
    // Take a photo with the camera
    Serial.println("Taking a photo...");

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    Serial.printf("Picture file name: %s\n", FILE_PHOTO);
    File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);
 
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    }
    else {
      file.write(fb->buf, fb->len); // payload (image), payload length
      Serial.print("The picture has been saved in ");
      Serial.print(FILE_PHOTO);
      Serial.print(" - Size: ");
      Serial.print(file.size());
      Serial.println(" bytes");
    }
    digitalWrite(4, 0);

    file.close();
    esp_camera_fb_return(fb);

    ok = checkPhoto(SPIFFS);
  } while ( !ok );
}

void initWiFi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}

void initSPIFFS(){
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }
}

void initCamera(){
  pinMode(4,OUTPUT);
  digitalWrite(4,0);
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  } 
}

void setup() {
  Serial.begin(9600);
  initWiFi();
  initSPIFFS();
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  initCamera();

  configF.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  configF.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  configF.database_url = DATABASE_URL;

  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);
}



void loop() {

  int i=random(1, 10000);

  if (!digitalRead(12)) {
    return;
  }
  
  if (takeNewPhoto) {
    capturePhotoSaveSpiffs();
  }
  delay(1);
  if (Firebase.ready() && !taskCompleted){
    //taskCompleted = true;
    Serial.print("Uploading picture... ");
    if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, FILE_PHOTO /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, "image/"+String(i)+".jpg" /* path of remote file stored in the bucket */, "image/jpeg" /* mime type */)){
      Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());


//"image/"+String(i)+".jpg"

      String s=fbdo.downloadURL().c_str() ;

      if (Firebase.RTDB.setString(&db, "Image/"+String(i),s)){
      Serial.println("PASSED");
      Serial.println("PATH: " + db.dataPath());
      Serial.println("TYPE: " + db.dataType());
    }else{
      Serial.println("Path not saved!");
    }
      
    }
    else{
      Serial.println(fbdo.errorReason());
    }
  }
  
  SPIFFS.format();
}
