#define BLYNK_PRINT Serial
// get these infos from your blynk console
#define BLYNK_TEMPLATE_ID "TMPxxxxxxx"
#define BLYNK_DEVICE_NAME "device name"
#define BLYNK_AUTH_TOKEN "Token_from_blynk"

// Uncomment for ESP32
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "esp_camera.h"

// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings

#define MAfront 32 //IN1
#define MAback 33 //IN2
#define MBfront 2 //IN3
#define MBback 13 //IN4

// Comment this out to disable prints and save space
//#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "SSID";
char password[] = "PASSWD";

BlynkTimer timer;

void MA_stop()
{
  digitalWrite(MAback,LOW);
  digitalWrite(MAfront,LOW);
}

void MA_Fwd()
{
  digitalWrite(MAback,LOW);
  digitalWrite(MAfront,HIGH);
}

void MA_Bck()
{
  digitalWrite(MAback,HIGH);
  digitalWrite(MAfront,LOW);
}


void MB_stop()
{
  digitalWrite(MBback,LOW);
  digitalWrite(MBfront,LOW);
}

void MB_Fwd()
{
  digitalWrite(MBback,LOW);
  digitalWrite(MBfront,HIGH);
}


void MB_Bck()
{
  digitalWrite(MBback,HIGH);
  digitalWrite(MBfront,LOW);
}



// In Blynk console or App, define a joystick(V0) as string, label X coords (V1) as enumerate, label Y coords (V2)as enumerate, label Status (V4) as string
// When the joystick is moved, X,Y coordonates and status will show. 
// This function is called every time the Joystick Virtual Pin 0 state changes
BLYNK_WRITE(V0) {
  int X = param[0].asInt();
  int Y = param[1].asInt();
  //Serial.print(X);
  //Serial.print(Y);
  if ((X == 128)&&(Y == 128)) {
    // stop
    MA_stop();
    MB_stop();
    Blynk.virtualWrite(V4,"Robot stop");
  }
  else {
    // forward
    if ((X>218)) {
      MA_Fwd();
      MB_Fwd();
      
      Blynk.virtualWrite(V4,"move forward");
      }
    if ((X<38)) {
      // backward
      MA_Bck();
      MB_Bck();
     
      Blynk.virtualWrite(V4,"move backward");
      }
    if ((Y>218)&&(38<X<218)) {
      // right
      MA_Bck();
      MB_Fwd();

      Blynk.virtualWrite(V4,"turn right");
      }
    if ((Y<38)&&(38<X<218)) {
      // left
      MB_Bck();
      MA_Fwd();
      
      Blynk.virtualWrite(V4,"turn left");
      }
  }
  Blynk.virtualWrite(V2, X);  // affiche X
  Blynk.virtualWrite(V3, Y);  // affiche Y
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  // Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  // Serial.println("Cloud connected");
}
  
// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  //Blynk.virtualWrite(V1, millis() / 1000);
}

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//

// Select camera model
#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM

#include "camera_pins.h"

void startCameraServer();

void setup()
{
  // stop all motors
  pinMode(MAfront, OUTPUT);
  pinMode(MAback, OUTPUT);
  pinMode(MBfront, OUTPUT);
  pinMode(MBback, OUTPUT);
  MA_stop();
  MB_stop();
  
  Serial.begin(115200);
  Blynk.begin(auth, ssid, password);

  // Setup a function to be called every second
  timer.setInterval(1000L, myTimerEvent);


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
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }
  //Serial.println("");
  //Serial.println("WiFi connected");

  startCameraServer();

  //Serial.print("Camera Ready! Use 'http://");
  //Serial.print(WiFi.localIP());
  //Serial.println("' to connect");
}

void loop() {
  // put your main code here, to run repeatedly:
  //delay(10000);

  Blynk.run();
  timer.run();
}
