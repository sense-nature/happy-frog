// GNU Affero General Public License v3.0
// Based on examples from https://github.com/matthijskooijman/arduino-lmic

//This sketch works properly on Heltec Wireless Stick


#define LMIC_DEBUG_LEVEL 2

#include <Arduino.h>
#include <pins_arduino.h>
#include <SPI.h>

#include "ttn_keys.h"
#include <lmic.h>
#include <hal/hal.h>

#include <oled/SSD1306Wire.h>
//#include <heltec.h>

#include <esp_sleep.h>
#include "soc/efuse_reg.h"

//Sensors
#include <DallasTemperature.h>
#include <OneWire.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>



SSD1306Wire display(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_64_32);
Adafruit_BME280 bme;



#define MY_ONEWIRE_PIN  12

// Data Variable for Temperature
#define N_TEMP (sizeof(DS18B20_SENSORS)/sizeof(DS18B20_SENSORS[0]) )
float temp[N_TEMP] = {0};

uint8_t boxHumidity = 0;
uint16_t boxPressure = 0;
float boxTemperature = -127;


uint16_t batteryVoltage = 0; //[12bit raw]

void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static osjob_t sendjob;

void LedON(){
	pinMode(LED_BUILTIN,OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
}

void LedOFF(){
	pinMode(LED_BUILTIN,OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
}


void VextON(void)
{
	pinMode(Vext,OUTPUT);
	digitalWrite(Vext, LOW);
}

void VextOFF(void) //Vext default OFF
{
	pinMode(Vext,OUTPUT);
	digitalWrite(Vext, HIGH);
}

void ResetDisplay()
{
	pinMode(RST_OLED,OUTPUT);
	digitalWrite(RST_OLED, LOW);
	delay(50ul);
	digitalWrite(RST_OLED, HIGH);
}

// Pin mapping for Heltec Wireless Stick (taken from bastelgarage.ch, confirmed working)
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,
    .dio = {26, 34 , 35 }
};

unsigned long startTime, endTime;

#define mS_TO_S_FACTOR 1000ULL   /*Conversion factor for mili to seconds*/
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  (1*60)        /* Time ESP32 will go to sleep (in seconds) */

uint8_t sessionStatus = 0;



void resetStatus(){
	sessionStatus = 0x00;
}

void setStatusWUNotFromDeepSleep( ){
		sessionStatus |= 0x01;
}

void setStatusBME280Error(){
	sessionStatus |= (0x01 << 1);
}

void setStatusDS18B20Error(uint8_t index){
	sessionStatus |= (0x01 << (index+1));
}



RTC_DATA_ATTR uint32_t bootCount = 0;


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

  setStatusWUNotFromDeepSleep();
  return sDefaultReason;
}



void goToDeepSleep(void *pUserData, int fSuccess){

   // Heltec.display->clear();
  Serial.write("Sending result: ");
   if( fSuccess){
	   display.drawString (30, 0, "Sent: OK");
	   Serial.println("OK");
   } else {
	   display.drawString (30, 0, "Send: FAILED");
	   Serial.println("FAILED");
   }
   // Schedule next transmission
   //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
   LedOFF();


   display.drawString (0, 10, "Package# "+ String (bootCount));
   display.display ();

   LMIC_shutdown();
   unsigned long delta = millis() - startTime;
   Serial.println("Work time: "+String(delta)+"ms");
   Serial.println("Going to sleep now");
   Serial.flush();

   esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR - delta * mS_TO_S_FACTOR);

   VextOFF();
   esp_deep_sleep_start();
}


uint8_t getHigh(int16_t val){

	return (val & 0xFF00) >> 8;
}

uint8_t getHigh(uint16_t val){
	return (val & 0xFF00) >> 8;
}


int8_t getLow(int16_t val){
	return val & 0xFF;
}
uint8_t getLow(uint16_t val){
	return val & 0xFF;
}



void pushTemperatureToMessage(std::vector<uint8_t> & vect, float fTemperature){
	int16_t iTemp = (int16_t)roundf(fTemperature*100.0f);
	vect.push_back(getHigh(iTemp));
	vect.push_back(getLow(iTemp));
}

void do_send(osjob_t* j, void(*callBack)(void *, int)){
    // Payload to send (uplink)

    //Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + "s");

	uint8_t serialNo = DEVICE_SERIAL_NO;

	std::vector<uint8_t> message;
	message.push_back(serialNo);
	message.push_back(sessionStatus);
	message.push_back(getHigh(batteryVoltage));
	message.push_back(getLow(batteryVoltage));
	message.push_back(boxHumidity);
	message.push_back(getHigh(boxPressure));
	message.push_back(getLow(boxPressure));
	pushTemperatureToMessage(message, boxTemperature);

	for(int i=0 ; i< N_TEMP; i++)
		pushTemperatureToMessage(message, temp[i]);

	/*
    uint8_t message[1+1+2+2+2*(N_TEMP)] = {serialNo,
    		sessionStatus,
			getHigh(batteryVoltage),
			getLow(batteryVoltage),
    		(uint8_t)(getHigh(tempLora0)),
			(uint8_t)(getLow(tempLora0)),
    		(uint8_t)(getHigh(tempLora1)),
			(uint8_t)(getLow(tempLora1)),
    };   // I know sending a signed int with a unsigned int is not good.
*/

	// Prepare upstream data transmission at the next possible time.
	lmic_tx_error_t ret = LMIC_sendWithCallback(1, message.data(), message.size(), 0, callBack, (void*)0);
	if( ret != LMIC_ERROR_SUCCESS ){
		Serial.println("Cannot register sending the LoRaWAN package. Error = "+String(ret));
	} else {
		LedON();
		Serial.println("Sending uplink packet.");
		display.clear();
		display.drawString (0, 0, "SND...");
		display.display ();
	}
}


//
//int getChipRevision()
//{
//  return (REG_READ(EFUSE_BLK0_RDATA3_REG) >> (EFUSE_RD_CHIP_VER_REV1_S)&&EFUSE_RD_CHIP_VER_REV1_V) ;
//}


// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {

	  if(i>0)
		  Serial.print(", 0x");
	  else
		  Serial.print("{ 0x");
	  if (deviceAddress[i] < 16)
		  Serial.print("0");
	  Serial.print(deviceAddress[i], HEX);
  }
  Serial.print(" }");
}





void readDS18B20Sensors(){
	OneWire w1(MY_ONEWIRE_PIN);
	DallasTemperature ds18b20(&w1);
	ds18b20.begin();
	ds18b20.setResolution(12);
	ds18b20.setCheckForConversion(false);
	ds18b20.requestTemperatures();
	 uint8_t n = ds18b20.getDS18Count();

	 if( n > 0 ){
		Serial.println("Detected "+String(n)+" DS18B20 sensor(s) on the bus");
	    for(uint8_t i=0; i < N_TEMP ; i++){
			 DeviceAddress addr = {0};
			 memcpy(addr, DS18B20_SENSORS[i], sizeof(addr));;
			 if(ds18b20.isConnected(addr) ){
				 Serial.print("Temperature @ ");
				 printAddress(addr);
				 temp[i] = ds18b20.getTempC(addr);
				 Serial.println(": " + String(temp[i]) + " *C");
			 } else{
				 setStatusDS18B20Error(i);
				 Serial.print("ds18b20 @ ");
				 printAddress(addr);
				 Serial.println(" found address is not connected");
			 }
	    }
	 } else {
		 for(uint8_t i =0; i< N_TEMP; i++)
			 setStatusDS18B20Error(i);
		 Serial.println("No ds18b20 detected on the bus");
	 }
}
void readBatteryVoltage()
{
	//analogSetSamples(8);
	pinMode(13,OPEN_DRAIN);
	delay(10u);
	batteryVoltage = analogRead(13); // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
	Serial.println("Battery voltage reading: "+ String(batteryVoltage));
}

void readBME280Sensor(){
	bool status = bme.begin(0x76, &Wire);
	if (!status) {
		Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
		setStatusBME280Error();
	} else {
		boxTemperature = bme.readTemperature();
		Serial.print("Temperature = ");
		Serial.print(boxTemperature);
		Serial.println(" *C");

		Serial.print("Pressure = ");

		boxPressure = roundf(bme.readPressure() / 100.0F);
		Serial.print(boxPressure);
		Serial.println(" hPa");

		boxHumidity = roundf(bme.readHumidity());
		Serial.print("Humidity = ");
		Serial.print(boxHumidity);
		Serial.println(" %");
	}

}

void initLoRaWAN(u4_t seqNo) {
	// LMIC init
	os_init_ex(&lmic_pins);
	// Reset the MAC state. Session and pending data transfers will be discarded.
	LMIC_reset();
	LMIC_setClockError(MAX_CLOCK_ERROR * 3 / 100);
	// Set static session parameters. TTN network has id 0x13
	LMIC_setSession(0x13, DEVADDR, NWKSKEY, APPSKEY);

	LMIC_setSeqnoUp(seqNo);
	//(bootCount);
	// Set up the channels used by the Things Network, which corresponds
	// to the defaults of most gateways. Without this, only three base
	// channels from the LoRaWAN specification are used, which certainly
	// works, so it is good for debugging, but can overload those
	// frequencies, so be sure to configure the full frequency range of
	// your network here (unless your network autoconfigures them).
	// Setting up channels should happen after LMIC_setSession, as that
	// configures the minimal channel set.
	///*
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
	// Disable link check validation
	LMIC_setLinkCheckMode(0);
	// TTN uses SF9 for its RX2 window.
	LMIC.dn2Dr = DR_SF9;
	// Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
	//LMIC_setDrTxpow(DR_SF11,14);
	//LMIC_setDrTxpow(DR_SF9,14);
	LMIC_setDrTxpow(DR_SF7, 14);
	LMIC_startJoining();
}





void setup() {
	startTime = millis();
	resetStatus();
	VextON();
	bootCount++;

	Serial.begin(115200ul);
	Serial.flush();
	delay(50ul);
	Serial.print("Wake up #");
    Serial.print(bootCount);
    Serial.print(", reason: ");
    Serial.println(get_wakeup_reason());


	display.init();
	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_10);
	display.drawString(0, 0, "OLED init!");
	display.display();
    display.clear();

	//ds18b20 must be read before other init, in particular before i2c devices
    readDS18B20Sensors();
	initLoRaWAN(bootCount);
    readBME280Sensor();
    readBatteryVoltage();
    do_send(&sendjob, goToDeepSleep);

}


void loop() {
	//will not be called after the deepsleep wake up
     os_runloop_once();
}
