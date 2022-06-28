#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <HardwareSerial.h>
#include <SSLClient.h>
#include "secrets.h"
#include "esp_camera.h"
#include <MQTTClient.h>
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems
#include "esp_http_server.h"



#define PART_BOUNDARY "123456789000000000000987654321"

// This project was tested with the AI Thinker Model, M5STACK PSRAM Model and M5STACK WITHOUT PSRAM
//Ai Thinker Module Ports
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

#define flashLight 16
#define trig 13
#define EEPROM_SIZE 1

#define ESP32CAM_PUBLISH_TOPIC   "esp32/cam_0"
#define ESP32CAM_ENABLE "esp32/cam_0_enable"


const int bufferSize = 40000; // 23552 
bool isEnabled = false;
const unsigned long timeout = 30000;

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(bufferSize);


//Connect With WIFI
void setupWifi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("\n\n=====================");
  Serial.println("Connecting to Wi-Fi");
  Serial.println("=====================\n\n");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n\n=====================");
  Serial.println("Connected to Wi-Fi");
  Serial.println("=====================\n\n");
  

}

//Capture the Image and send to the mqtt clients
void grabImage(){
 
  camera_fb_t * fb = esp_camera_fb_get();
  Serial.println("\n\n=====================");
  Serial.println("Publishing");
  Serial.println("=====================\n\n");
  
  if(fb != NULL && fb->format == PIXFORMAT_JPEG && fb->len < bufferSize){
    bool result = client.publish(ESP32CAM_PUBLISH_TOPIC, (const char*)fb->buf, fb->len);
    if(!result){
      ESP.restart();
    }
    digitalWrite(33,LOW);
  }
  esp_camera_fb_return(fb);
  delay(1000);
}



//Initilialize the camera for monitoring
void cameraInit(){
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);
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
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 30;
  config.fb_count = 2;
  
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
    return;
  }

}



void callback(String &topic, String &payload) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  Serial.println(payload);

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == ESP32CAM_ENABLE) {
    Serial.print("Changing output to ");
    if(payload == "on"){
      Serial.println("on");
      digitalWrite(33, LOW);
      isEnabled = true;
    }
    else if(payload == "off"){
      Serial.println("off");
      digitalWrite(33, HIGH);
      isEnabled = false;
    }
  }
}


void setup() {
  cameraInit(); 
  setupWifi(); 
  pinMode(33,OUTPUT);
  delay(1000);
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
//net.setCertificate(AWS_CERT_CRT);
//net.setPrivateKey(AWS_CERT_PRIVATE);
  


  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(BROKER_URL, 8883, net);
  client.setCleanSession(true);

  client.onMessage(callback);

  while (!client.connect(CLIENTID,"test1234","test1234")) {
    Serial.print(".");
    delay(100);
  }

  client.subscribe(ESP32CAM_ENABLE);







  
}

void loop() {
  client.loop();
  
  if(isEnabled && client.connected())grabImage();
}