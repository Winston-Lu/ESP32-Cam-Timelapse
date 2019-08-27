# ESP32-Cam-Timelapse
A power-efficient and customizable program to create time-lapses or record picture sequences using deep sleep in between pictures and before starting up.

Can be uploaded to the module using the default partition scheme\
&nbsp;

# Programming
Here is the ESP32-CAM module, I added a 32GB Micro SD card in the top, however only 4GB is usable
![IMG_20190826_213019](https://user-images.githubusercontent.com/33874247/63743509-4e1de900-c851-11e9-8895-05932f5e27d4.jpg)\
&nbsp;

Plug it in into a FTDI programmer board like the red one above. Remember to set the FTDI board to 3.3v (As 5v may damage the board), which is the right position on the switch when the USB port is facing up. In my programmer, I am supplying a 5v VCC supply voltage to the ESP32-CAM module so less uploading and brownout problems occur when using a less-powerful USB port or a module that has a "less-than-optimal" power regulator. When programming the board, pull GPIO 0 to ground (which is the small switch I put on the side in the left position). Remember to pull GPIO 0 to ground before plugging the USB port into your computer, or else the next step may not work and get stuck in a `Connecting....._____.....` loop and errors out.\
![IMG_20190826_213059](https://user-images.githubusercontent.com/33874247/63743510-4e1de900-c851-11e9-8e15-4be11c4a8a56.jpg)\
&nbsp;

Set the board to the ESP Wrover Module and the settings below. If you do not see the ESP32 boards, follow [this guide here.](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/) Once you set it to these options and selected the COM port that is connected to the FTDI programmer, press the upload button on the Arduino IDE.\
![Untitled](https://user-images.githubusercontent.com/33874247/63743513-4eb67f80-c851-11e9-9d97-67e0be1ba1ff.png)\
&nbsp;

When the serial monitor on the bottom starts saying `Connecting......._____ `, press the button on the bottom of the ESP32-CAM Module. This resets the board and allows the code to upload to the ESP32 module. The serial monitor will hard reset and tell you when it hard resets and is done uploading.
![IMG_20190826_213123](https://user-images.githubusercontent.com/33874247/63743511-4e1de900-c851-11e9-835b-2d610601327e.jpg)\
&nbsp;

On some of my boards, the power regulator seemed to not operate well when plugged into any of my desktop's USB ports, and was in a constant restart loop. Due to this, I had to set the board to Flash Frequency: 40MHz, instead of the usual Flash Frequency: 80MHz in order to be able to run the code, though uploading on 80MHz ran without error until it ran the code iteself. I labeled these boards as "40" to keep track of them.\
&nbsp;

# Running
If you want to test the board, unplug the VCC power connection or the USB connection to the FTDI programmer and disconnect GPIO 0 from ground (right position in my board). Plug the power back in, and open the serial monitor running at a baud rate of 115200. Press the reset button located underneath the ESP32-CAM module and it should output a bunch of text, and then some serial output.

If you want to run the board without the FTDI programmer, you need to supply the camera with 5v through the top left pin labeled as 5V (or 3.3v in the top left pin labaled 3V3), and connect the ground pin to GND. In my case, I used 3 standard 18650 3.7v Lithium Ion battery in series with a buck converter and charger. In most cases, using a lithium battery would be one of the better options, so modules stepping the voltage up to 5v and offing charge protection is quite common and cheap. You could either solder on a male USB header, connecting the red wire to 5V, black wire to GND, and optionally connecting the white and green data pins together. 
![IMG_20190826_213206](https://user-images.githubusercontent.com/33874247/63743512-4e1de900-c851-11e9-8350-7a3d2d86ebb7.jpg)\
&nbsp;

# Customizable Variables
### STARTUP_DELAY
This is the initial delay from when the ESP32-CAM starts up, then  put into deep sleep before waking up and taking pictures. By default, I set it to 30 seconds, and the range can be from 0 seconds to 584,542 years (Max 64 bit integer in microseconds).

### PICTURE_DELAY
This is the time the camera waits before taking the next picture. By default, I set it to 3 seconds.

### Camera Effects
If you want to play around with effects, I would recommend using the `File > Examples > ESP32 > Camera > CameraWebServer.` example, upload using the HUGE APP (3MB no OTA) partition scheme and check what effects work best for you. In most cases, no extra effects need to be added, but it has been provide if you need it.\
&nbsp;

Uncomment the `sensor_t * s = esp_camera_sensor_get();` line if you want to start modifying any effects\
quality: 10 to 63 — does the same as config.frame_size (see below)\
brightness: -2 to 2 — Adds/removes brightness in dark/really sunny areas\
contrast: -2 to 2 — As implied, effect not very noticable\
saturation: -2 to 2 — As implied.\
sharpness: -2 to 2 — As implied, sharpens image, though not too noticable\
wb_mode: 0 or 1 — White balance mode. 0:auto, 1:manual. Balances the brightness/darkness of surrounding\
awb_gain: 0 or 1 — Auto White Balance gain.\
awc_value: 0 or 1 — Auto White Correction\
aec2: 0 or 1 — Automatic exposure control\
ae_level: -2 to 2 — Automatic exposure level, limit the maximum range for aec\
aec_value: 0 or 1 — Default value for exposure\
agc_gain: 0 or 1 — Auto gain control\
gainceiling: 1 to 64 — As implied\
bpc: 0 or 1 — Bits Per Component, or depth level\
wpc: 0 or 1 — No idea\
raw_gma: 0 or 1 — No idea\
lenc: 0 or 1 — Lens correction\
vflip: 0 or 1 — Flip camera vertically\
hmirror: 0 or 1 — Flip camera horizontally\
whitebal: 0 or 1 — White balance\
dcw: 0 or 1 — No idea

### config.frame_size 
This is how big you want the picture to be taken. By default, I set it to UXGA (1600px by 1200px), which averaged around 180kb - 230kb per picture. Changing the frame size allows for more photos to be stored onto the SD card (limited by the 4GB).

### config.jpeg_quality 
This is how good the picture should be. By default, I set it to 10, and the lower it is, the better quality it is. The range for this is 10-63, with 10 being the best quality and 63 being the worst.

### config.fb_count 
This allocates the size of the frame buffer. From the examples including PSRAM (which this module does), it defaults to 2, whereas without PSRAM it goes to 1. I would not recommend changing this unless there are some quality issues with the photos that can not be filtered with other filters as seen under `File > Examples > ESP32 > Camera > CameraWebServer.`\
&nbsp;

# Additional Information
The built-in flash LED is connected to one of the pins of the SD card, and I am unsure if removing it may damage the SD card operation. Whenever the SD card is being written, the flash will turn on, and is a hardware issue that can not be fixed in software (as far as I know). I would recommend taping over the light with something like electrical tape if this is an issue.
