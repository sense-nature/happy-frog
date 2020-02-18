// GNU Affero General Public License v3.0
// Based on examples from https://github.com/matthijskooijman/arduino-lmic

//This sketch works properly on Heltec Wireless Stick


#define LMIC_DEBUG_LEVEL 2

#include <Arduino.h>
#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>

#include <lmic.h>
#include <hal/hal.h>
#include <heltec.h>
#include "soc/efuse_reg.h"

#define LEDPIN 2

#define OLED_I2C_ADDR 0x3C
#define OLED_RESET 16
#define OLED_SDA 4
#define OLED_SCL 15

unsigned int counter = 0;


//Sensor
#include <DallasTemperature.h>
#include <OneWire.h>

#include <SoftwareSerial.h>
SoftwareSerial soft_serial;

#define ph_serial soft_serial

const uint8_t bufferlen = 32;                         //total buffer size for the response_data array
char response_data[bufferlen];
String inputstring = "";

#define MY_ONEWIRE_PIN 13
OneWire oneWire(MY_ONEWIRE_PIN);
DallasTemperature ds18b20(&oneWire);
float last_temp;

// Data Variable for PH and Temp
String STemp;
String SPH;
float TempMean = 0;
float PHMean = 0;
int16_t TempLora = 0;
uint16_t PHLora = 0;
uint8_t TempLoraHigh = 0;
uint8_t TempLoraLow = 0;
uint8_t PHLoraHigh = 0;
uint8_t PHLoraLow = 0;


/*************************************
 * TODO: Change the following keys
 * NwkSKey: network session key, AppSKey: application session key, and DevAddr: end-device address
 *************************************/
static u1_t NWKSKEY[16] = { 0xA9, 0x2F, 0x7D, 0x2B, 0xCA, 0xB4, 0x71, 0x34, 0x0D, 0x6D, 0xAA, 0xED, 0x89, 0xBC, 0xE9, 0x4F };  // Paste here the key in MSB format

static u1_t APPSKEY[16] = { 0x98, 0x92, 0xD5, 0xC0, 0xA8, 0x71, 0x20, 0x64, 0x52, 0x96, 0x41, 0xB4, 0xA2, 0x8E, 0x39, 0xA5 };  // Paste here the key in MSB format

static u4_t DEVADDR = 0x26011955;   // Put here the device id in hexadecimal form.

void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static osjob_t sendjob;


// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;
char TTN_response[30];

// Pin mapping for Heltec Wireless Stick (taken from bastelgarage.ch, confirmed working)
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,
    .dio = {26, 34 , 35 }
};

unsigned long startTime, endTime;


#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  30        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;


const char *  get_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  static char sDefaultReason[30];

  sprintf(sDefaultReason, "NotDS: %d\0", wakeup_reason);
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : return "DSWU:ext RTC_IO"; break;
    case ESP_SLEEP_WAKEUP_EXT1 : return "DSWU:ext RTC_CNTL"; break;
    case ESP_SLEEP_WAKEUP_TIMER : return "DSWU: timer"; break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : return "DSWU: touch"; break;
    case ESP_SLEEP_WAKEUP_ULP : return "DSWU: ULP"; break;
  }
  return sDefaultReason;
}



void onSent(void *pUserData, int fSuccess){

   // Heltec.display->clear();
  Serial.write("Sending result: ");
   if( fSuccess){
	   Heltec.display->drawString (30, 0, "OK");
	   Serial.println("OK");
   } else {
	   Heltec.display->drawString (30, 0, "FAILED");
	   Serial.println("FAILED");
   }
   // Schedule next transmission
   //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
   digitalWrite(LEDPIN, LOW);
   Heltec.display->drawString (0, 10, "Package# "+ String (bootCount));
   Heltec.display->display ();

   LMIC_shutdown();
   unsigned long delta = millis() - startTime;
   Serial.println("Work time: "+String(delta)+"ms");
   Serial.println("Going to sleep now");
   Serial.flush();

   esp_deep_sleep_start();

    // Schedule next transmission
    //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);

}

void do_send(osjob_t* j){
    // Payload to send (uplink)
    Heltec.display->clear();
    Serial.print("Wake up #");
    Serial.print(bootCount);
    Serial.print(", reason: ");
    Serial.println(get_wakeup_reason());
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR - 2228000);
    //2228ms is the average work time (10 samplese)

    //Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + "s");

	uint8_t serialNo = 0x01;
	TempMean = 18.31;
    TempMean *= 100;
    TempLora = (int16_t)TempMean;        // Teperatur range -127.00 ... 126.00
    TempLoraHigh = TempLora >> 8;
    TempLoraLow = TempLora & 0x00FF;
    PHMean = 5.54;
    PHMean *= 100;
    PHLora = (uint16_t)PHMean;           // PH range 0 ... 140 (devide by 10 for actual value)
    PHLoraHigh = PHLora >> 8;
    PHLoraLow = PHLora & 0x00FF;
    uint8_t message[] = {TempLoraHigh,TempLoraLow,PHLoraHigh,PHLoraLow /*, serialNo*/};   // I know sending a signed int with a unsigned int is not good.

	// Prepare upstream data transmission at the next possible time.
	lmic_tx_error_t ret = LMIC_sendWithCallback(1, message, sizeof(message), 0, onSent, (void*)0);
	if( ret != LMIC_ERROR_SUCCESS ){
		Serial.println("Cannot register sending the LoRaWAN package. Error = "+String(ret));
	} else {
		digitalWrite(LEDPIN, HIGH);
		Serial.println("Sending uplink packet.");
		Heltec.display->clear();
		Heltec.display->drawString (0, 0, "SND...");
		Heltec.display->display ();
	}
}


int getChipRevision()
{
  return (REG_READ(EFUSE_BLK0_RDATA3_REG) >> (EFUSE_RD_CHIP_VER_REV1_S)&&EFUSE_RD_CHIP_VER_REV1_V) ;
}


void setup() {
	startTime = millis();
    Heltec.begin(true, false);
    delay(1500);
    bootCount++;
    // LMIC init
    os_init_ex(&lmic_pins);
     // Reset the MAC state. Session and pending data transfers will be discarded.
	LMIC_reset();
	LMIC_setClockError(MAX_CLOCK_ERROR * 3 / 100);
	LMIC_setSession (0x13, DEVADDR, NWKSKEY, APPSKEY);
	LMIC_setSeqnoUp(bootCount);

	//(bootCount);

    // Set up the channels used by the Things Network, which corresponds
    // to the defaults of most gateways. Without this, only three base
    // channels from the LoRaWAN specification are used, which certainly
    // works, so it is good for debugging, but can overload those
    // frequencies, so be sure to configure the full frequency range of
    // your network here (unless your network autoconfigures them).
    // Setting up channels should happen after LMIC_setSession, as that
    // configures the minimal channel set.


    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(EU868_DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(EU868_DR_SF12, EU868_DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(EU868_DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(EU868_DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(EU868_DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(EU868_DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(EU868_DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(EU868_DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(EU868_DR_FSK,  EU868_DR_FSK),  BAND_MILLI);      // g2-band

    //*/

    // TTN defines an additional channel at 869.525Mhz using SF9 for class B
    // devices' ping slots. LMIC does not have an easy way to define set this
    // frequency and support for class B is spotty and untested, so this
    // frequency is not configured here.

    // Set static session parameters.
   // LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    //LMIC_setDrTxpow(DR_SF11,14);
    //LMIC_setDrTxpow(DR_SF9,14);
    LMIC_setDrTxpow(DR_SF7,14);
    LMIC_startJoining();

//Sensor
// put your setup code here, to run once:
  // Serial.begin(115200);
    ///  ph_serial.begin(9600, 22, 17);
  //inputstring.reserve(20);

  ///  ph_serial.print("*ok,0\r");
  //pH_sensor.send_cmd_no_resp("c,0");       //send the command to turn off continuous mode
                                           //in this case we arent concerned about waiting for the reply
  // delay(100);
  // pH_sensor.send_cmd_no_resp("*ok,0");     //send the command to turn off the *ok response
                                           //in this case we wont get a reply since its been turned off
  //delay(100);
  //pH_sensor.flush_rx_buffer();
  // pinMode(16, OUTPUT);
  // digitalWrite(16, HIGH);

    // Start job
    do_send(&sendjob);

}


void loop() {

	//will not be called after the deepsleep wake up
     os_runloop_once();

//Sensor
// delay(1000);
 /*
if (Serial.available() > 0) {                       //if we get data from the computer
    inputstring = Serial.readStringUntil(13);                 //parse the data to either switch ports or send it to the circuit
    if (inputstring != "") {                          //if we have a command for the modules
      //pH_sensor.send_cmd(inputstring, response_data, bufferlen); // send it to the module of the port we opened
      //Serial.println(response_data);                  //print the modules response
      //response_data[0] = 0;                           //clear the modules response
      Serial.println("send command: " + inputstring);
      ph_serial.print(inputstring + "\r");
      Serial.println("answer: " + ph_serial.readStringUntil(13));
      // Serial.println(String(TempMean));
    }
  }

*/
// pH_sensor.send_read();
  ///  ph_serial.print("r\r");                 // opens a connection to the Sensor
  ///  SPH = ph_serial.readStringUntil(13);    // only one read possible than cloesd untill its opened again
  ///Serial.println("pH: " + SPH);
  ///PHMean += SPH.toFloat();
  ///PHMean /= 2;

  ///ds18b20.requestTemperatures();
  //for now, we want to read only the first sensor
  ///last_temp = ds18b20.getTempCByIndex(0);

  /*
  STemp = String(last_temp);
  Serial.println("ds18b20: " + String(STemp) + "�C");
  TempMean += STemp.toFloat();
  TempMean /= 2;
  Serial.println( String(PHMean) + " / " + String(TempMean));
  //*/
//	delay(1000);
	//Serial.println("Next loop iteration. counter="+String(counter ));
}
