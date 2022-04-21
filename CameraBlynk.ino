#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

const char* ssid = "Irfan.A";
const char* password = "irfan0204";
char auth[] = "xq7WcIs5C-c4E88y4k-YzXOw7Nhr_EgR";  //sent by Blynk

// Select camera model
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"
#define butLampu 14//lampu1 14
#define lampu 2
#define pir1   13
#define pir2   15
#define indikator 12
#define PHOTO 33
#define ButSecurity 3
#define FLASH 4

byte pir_1=0;
byte pir_2=0;
bool stateLam = false;
bool statePir1 = false;
bool statePir2 = false;
bool stateInd = false;
bool stateSensor = false;
bool stateSecurity = false;

unsigned long Sindi = 0;
unsigned long SLedV = 0;
unsigned long SRecon = 0;
unsigned long SBack = 0;
byte Dindi = 1000;
byte DRecon = 1000;
byte DBack = 1000;
bool stateI = false;
int counter=0;
int timer=0;
byte batas = 30;

String local_IP;
void startCameraServer();

#define BLYNK_GREEN     "#23C48E"
#define BLYNK_BLUE      "#04C0F8"
#define BLYNK_YELLOW    "#ED9D00"
#define BLYNK_RED       "#D3435C"
#define BLYNK_DARK_BLUE "#5F7CD8"

WidgetLED led1(V2);
WidgetLED led2(V3);

//BlynkTimer timer;

void blinkLedWidget()
{ 
  led1.setColor(BLYNK_YELLOW);
  unsigned long tmr = millis();
  
  if(tmr - SLedV > Dindi){
    SLedV = tmr;
  if (led1.getValue()) {
    led1.off();
    //Serial.println("LED on V1: off");
  } else {
    led1.on();
    //Serial.println("LED on V1: on");
       }
    }
}

BLYNK_WRITE(V0)
{
  byte pinValue = param.asInt();
  if(pinValue == 1){
    Blynk.notify("CONTROLLER DIRESET");
    delay(200);
    ESP.restart();
  }
}

void takePhoto()
{
  uint32_t randomNum = random(50000);
  Serial.println("http://"+local_IP+"/capture?_cb="+ (String)randomNum);
  Blynk.setProperty(V4, "urls", "http://"+local_IP+"/capture?_cb="+(String)randomNum);
  led1.setColor(BLYNK_BLUE);
  delay(1000);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  pinMode(lampu,OUTPUT);
  pinMode(pir_1,INPUT);
  pinMode(pir_2,INPUT);
  pinMode(indikator,OUTPUT);
  
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

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
   // digitalWrite(33,HIGH);
    digitalWrite(indikator,LOW);
    Serial.print(".");
    delay(500);
  }
  //digitalWrite(33,LOW);
  digitalWrite(indikator,HIGH);
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  local_IP = WiFi.localIP().toString();
  Serial.println("' to connect");
  Blynk.begin(auth, ssid, password);
  //timer.setInterval(1000L, blinkLedWidget);
   led1.on();
   led2.on();
  Blynk.notify("CAMERA AKTIVE");
}

void loop() {
  
  Blynk.run();
 // timer.run();
  Indikator();
  Reconnect();
  
  if(stateSecurity==true){
  
    sensorPripare();
    blinkLedWidget();
   // Serial.println("security aktif");
    if(stateSensor==true){
      stateLam=true;
      takePhoto();     
      led2.setColor(BLYNK_YELLOW);
      Blynk.notify("OBJEK TERDETEKSI");
      timer=0;
      SBack=0;
    }

    else if(stateSensor == false){
      TimerBack(1);

       while(timer >= batas){
        stateLam=false;
        led2.setColor(BLYNK_RED);
        timer=0;
        break;
    }
  }
  }
  else{
  led1.on();
  led1.setColor(BLYNK_GREEN); 
  }
  
  systemSecurity();
  outLamp();
  
}

void systemSecurity(){

     if(digitalRead(PHOTO) == LOW){
       Serial.println("Capture Photo");
       takePhoto();
     }
 
     if(digitalRead(ButSecurity)==HIGH){
      stateSecurity=true; 
      
     }
     
     else if(digitalRead(ButSecurity)==LOW){
      stateSecurity=false;
      timer=0;
     }

     if(digitalRead(butLampu) == HIGH && stateSecurity == false){
       stateLam=true;
       led2.setColor(BLYNK_GREEN);
      } 

     else if(digitalRead(butLampu) == LOW && stateSecurity == false){
       stateLam=false;
       led2.setColor(BLYNK_RED);
      } 

      
  
}

void sensorPripare(){
  pir_1 = digitalRead(pir1);
  pir_2 = digitalRead(pir2);

  if(pir_1 == HIGH && pir_2 == HIGH){
    stateSensor = true;
  }
  if(pir_1 == HIGH && pir_2 == LOW){
    stateSensor = false;
  }
  if(pir_1 == LOW && pir_2 == HIGH){
    stateSensor = false;
  }
  if(pir_1 == LOW && pir_2 == LOW){
    stateSensor = false;
  }
//  Serial.println(pir_1);
//  Serial.println(pir_2);
}

void outLamp(){
  if(stateLam == true){
    digitalWrite(lampu,HIGH);
   // Serial.println("lampu");
  }
  else if(stateLam == false){
    digitalWrite(lampu,LOW);
  }

  if(stateI == true){
    digitalWrite(indikator,HIGH);
  }
  else if(stateI == false){
    digitalWrite(indikator,LOW);
  }
}

void Reconnect(){
  unsigned long tmr = millis();

if (WiFi.status() != WL_CONNECTED) {
  if(tmr - SRecon > DRecon){
        SRecon = tmr;
        counter++;
        if(counter == 120){
        Serial.println("RESTART");
        ESP.restart();
        }
   }
}
}

void Indikator(){
  unsigned long tmr = millis();
  
  if (WiFi.status() != WL_CONNECTED) {
    if(tmr - Sindi > Dindi){
      Sindi = tmr;
      stateI = !stateI;     
    }
  }
  
  else if(WiFi.status() == WL_CONNECTED){
    stateI = true;
    }
}

int TimerBack(bool state){
  
  if(state==1){
  unsigned long tmr = millis();
 if(tmr - SBack > DBack){
  SBack = tmr;
  timer++;
  Serial.println(timer); 
 }
  }

  else if(state==0){
    timer=0;
    SBack=0;
  }

}
