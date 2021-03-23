#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "dl_lib.h"
#include "driver/rtc_io.h"

#define PICTURE_DELAY  3        /* Delay for picture taking (in seconds) */
#define STARTUP_DELAY 30            /* Optional Time ESP32 will sleep on first boot in seconds. Max delay is 584942 years before it overflows, so it wont be an issue soon*/
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

#ifdef STARTUP_DELAY 
  const uint64_t totalTimeSleep = uS_TO_S_FACTOR * STARTUP_DELAY; /*Avoid integer overflow using unsigned 64 bit int.*/
#else
  const uint64_t totalTimeSleep = 0; //skip through the initial sleep and continue with regular code execution
#endif

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
#define LED_1             33 //built-in back LED

RTC_DATA_ATTR uint32_t bootCount = 0; //times restarted

void setup() {
  Serial.begin(115200);
  Serial.println("Startup");
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  if(!bootCount){ //Make sure this scope runs only on the first run
    ++bootCount; //first boot
    esp_sleep_enable_timer_wakeup(totalTimeSleep); //Init timer
    //Flash built-in LED 3 times indicating startup
    pinMode(LED_1,OUTPUT);
    for(byte i = 0; i<3; i++){
      digitalWrite(LED_1, HIGH);delay(200);
      digitalWrite(LED_1, LOW);delay(200);
    }
    //Go to sleep
    Serial.printf("Going to sleep for %d seconds",STARTUP_DELAY);
    esp_deep_sleep_start();
  }else{rtc_gpio_hold_dis(GPIO_NUM_4);} //Re-enable GPIO 4 for SD Card
  
  //Flash once when waking up from deep sleep
  if(bootCount == 1){
    pinMode(LED_1,OUTPUT);
    digitalWrite(LED_1, HIGH);delay(1000);
    digitalWrite(LED_1, LOW);
  }
  //Setup camera pins
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

  //**Camera Effects**
  //sensor_t * s = esp_camera_sensor_get();
  //s->set_quality(s, 10);
  //s->set_brightness(s, 0);
  //s->set_contrast(s, 0);
  //s->set_saturation(s, 1);
  //s->set_sharpness(s, 0);
  //s->set_wb_mode(s, 0);
  //s->set_awb_gain(s, 1);
  //s->set_aec_value(s, 1);
  //s->set_aec2(s, 0);
  //s->set_ae_level(s, 0);
  //s->set_aec_value(s, 168);
  //s->set_agc_gain(s, 0);
  //s->set_gainceiling(s, (gainceiling_t)1); 
  //s->set_bpc(s, 0);
  //s->set_wpc(s, 1);
  //s->set_raw_gma(s, 1);
  //s->set_lenc(s, 1);
  //s->set_vflip(s, 0);
  //s->set_hmirror(s, 0); 
  //s->set_whitebal(s, 1);
  //s->set_dcw(s, 1);
  
  // Camera Quality assuming 3.8GB of usable space
  // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  // UXGA 1600x1200 ~200kb ~19000 pictures ~15h @ 3s |  ~10hr @ 2s |  ~5hr @ 1s
  // SXGA 1280x1024 ~150kb ~25300 pictures ~21h @ 3s |  ~14hr @ 2s |  ~7hr @ 1s
  // XGA  1280x768  ~100kb ~38000 pictures ~31h @ 3s |  ~20hr @ 2s | ~10hr @ 1s
  // SVGA  800x600   ~60kb ~63300 pictures ~52h @ 3s |  ~34hr @ 2s | ~17hr @ 1s
  // VGA   640x480   ~40kb ~95000 pictures ~79h @ 3s |  ~52hr @ 2s | ~26hr @ 1s
  config.frame_size = FRAMESIZE_UXGA; 
  config.jpeg_quality = 10;
  config.fb_count = 2;
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {Serial.printf("Camera init failed with error 0x%x", err);return;}
  
  // Init SD Card
  uint8_t cardType = SD_MMC.cardType();
  if(!SD_MMC.begin()){Serial.println("SD Card Mount Failed");return;}
  if(cardType == CARD_NONE){Serial.println("No SD Card attached");return;}
  
  // Take Picture with Camera
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {Serial.println("Camera capture failed");return;}
  
  // Change picture number/name as each boot count increases
  uint32_t pictureNumber = bootCount++;

  // Path where new picture will be saved in SD Card
  String path = "/picture" + String(pictureNumber) +".jpg";
  fs::FS &fs = SD_MMC; 
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file) Serial.println("Failed to open file in writing mode");
  else file.write(fb->buf, fb->len); 
  file.close();
  
  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4); //Latch value when going into deep sleep
  esp_sleep_enable_timer_wakeup(PICTURE_DELAY * uS_TO_S_FACTOR);
  esp_deep_sleep_start(); //Restart cam
}

void loop() {
  //Should never loop
  delay(1000);
}
