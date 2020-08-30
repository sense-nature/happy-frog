// GNU Affero General Public License v3.0
// Based on examples from https://github.com/matthijskooijman/arduino-lmic

//This sketch works properly on Heltec Wireless Stick


#define LMIC_DEBUG_LEVEL 2

#include <Arduino.h>
#include <pins_arduino.h>
#include <SPI.h>
#include <driver/adc.h>
#include <esp_wifi.h>
#include <esp_bt.h>

#include "device_data.h"

#include <lmic.h>
#include <hal/hal.h>

//#include <heltec.h>
//the Heltec version of SSD1306: 64x32 + OLED_RST
#include <oled/SSD1306Wire.h>


#include <esp_sleep.h>
#include "soc/efuse_reg.h"

//Sensors
#include <DallasTemperature.h>
#include <OneWire.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>







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


SSD1306Wire * getDisplay(){
	static SSD1306Wire * pDisplay = 0;
	if( pDisplay == 0 )
		pDisplay = new SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_64_32);
	return pDisplay;
}


Adafruit_BME280 * getBme(){
	static Adafruit_BME280 * pBme =0;
	if(pBme == 0 )
		pBme = new Adafruit_BME280();
	return pBme;
}


void LedON(){
//	pinMode(LED_BUILTIN,OUTPUT);
//	digitalWrite(LED_BUILTIN, HIGH);
}

void LedOFF(){
//	pinMode(LED_BUILTIN,OUTPUT);
//	digitalWrite(LED_BUILTIN, LOW);
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

/*

void ResetDisplay()
{
	pinMode(RST_OLED,OUTPUT);
	digitalWrite(RST_OLED, LOW);
	delay(50ul);
	digitalWrite(RST_OLED, HIGH);
}
*/


// Pin mapping for Heltec Wireless Stick (taken from bastelgarage.ch, confirmed working)
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,
    .dio = {26, 34 , 35 }
};

unsigned long startTime, endTime;
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
	sessionStatus |= (0x01 << (index+2));
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

bool firstRun()
{
	return sessionStatus & 0x01;
}



#define mS_TO_S_FACTOR 1000ULL   /*Conversion factor for mili to seconds*/
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */


void goToDeepSleep(){

   LMIC_shutdown();
   unsigned long delta = millis() - startTime;
   Serial.println("Work time: "+String(delta)+"ms");
   Serial.println("Going to sleep now");
   Serial.flush();
   LedOFF();
   if( firstRun() )
	   delay(5000u);

   getDisplay()->end();

   digitalWrite(RST_OLED, LOW);
   pinMode(MY_ONEWIRE_PIN,OUTPUT);
   digitalWrite(MY_ONEWIRE_PIN, LOW);




   if( bootCount < 15 ) //first 10 times run go to sleep only for 60s - useful for installation process
	   esp_sleep_enable_timer_wakeup(60 * uS_TO_S_FACTOR - delta * mS_TO_S_FACTOR);
   else
	   esp_sleep_enable_timer_wakeup(TIME_BETWEEN_MEASUREMENTS * uS_TO_S_FACTOR - delta * mS_TO_S_FACTOR);
   VextOFF();
   esp_deep_sleep_start();
}


void afterLoraPacketSent(void *pUserData, int fSuccess){
   // Heltec.display->clear();
  Serial.write("Sending result: ");
   if( fSuccess){
	   //getDisplay()->drawString (20, 0, " SENT");
	   Serial.println("OK");
   } else {
	   //getDisplay()->drawString (20, 0, " FAILED");
	   Serial.println("FAILED");
   }
   // Schedule next transmission
   //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
   goToDeepSleep();

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


void push2BytesToMessage(std::vector<uint8_t> & vect, int16_t iValue){
	vect.push_back(getHigh(iValue));
	vect.push_back(getLow(iValue));
}

void pushTemperatureToMessage(std::vector<uint8_t> & vect, float fTemperature){
	int16_t iTemp = (int16_t)roundf(fTemperature*100.0f);
	push2BytesToMessage(vect, iTemp);
}

void do_send(osjob_t* j, void(*callBack)(void *, int)){
	getDisplay()->drawString(0, 11, "#" + String(bootCount));
	getDisplay()->drawString (0, 22, "0x"+ String(sessionStatus,16));
	getDisplay()->display();
	if( firstRun() || DEVICE_ID==0x03 ){
		//don't send anything once in the first  after turning on
		LedON();
		Serial.println("Skipping sending packet #"+ String (bootCount));
		goToDeepSleep();
	} else {
		// Payload to send (uplink)
		uint8_t serialNo = DEVICE_ID;
		std::vector<uint8_t> message;
		message.push_back(sessionStatus);
		push2BytesToMessage(message, batteryVoltage);
		message.push_back(boxHumidity);
		push2BytesToMessage(message, boxPressure);
		pushTemperatureToMessage(message, boxTemperature);
		for(int i=0 ; i< N_TEMP; i++)
			pushTemperatureToMessage(message, temp[i]);

		// Prepare upstream data transmission at the next possible time.
		lmic_tx_error_t ret = LMIC_sendWithCallback(serialNo, message.data(), message.size(), 0, callBack, (void*)0);
		if( ret != LMIC_ERROR_SUCCESS ){
			Serial.println("Cannot register sending the LoRaWAN package. Error = "+String(ret));
		} else {
			LedON();
			Serial.println("Sending uplink packet #"+ String (bootCount));
		}
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
	delay(200u);
	 uint8_t n = ds18b20.getDS18Count();

	 if( n > 0 ){
		 DeviceAddress addr = {0};
		 Serial.println("Detected "+String(n)+" DS18B20 sensor(s) on the bus");
		if( firstRun() ){
		    Serial.println("Detected DS18B20 sensors:");
		    Serial.print("{");
		    for(uint8_t i=0; i < n ; i++){
				ds18b20.getAddress(addr, i);
				if(i >0)
					Serial.println(",");
				printAddress(addr);
		    }
		    Serial.println("};");
		}
	    for(uint8_t i=0; i < N_TEMP ; i++){
			 memcpy(addr, DS18B20_SENSORS[i], sizeof(addr));
			 Serial.print("T"+String(i+1)+ " @ds18b20 ");
			 printAddress(addr);
			 String present = "-";
			 if(ds18b20.isConnected(addr) ){
				 temp[i] = ds18b20.getTempC(addr);
				 Serial.println(": " + String(temp[i]) + " *C");
				 present = "+";
			 } else{
				 setStatusDS18B20Error(i);
				 Serial.println(" - sensor not connected");
			 }
			 getDisplay()->drawString(29+(i/4)*15, (i*7)%28, String(i+1) + present);
			 getDisplay()->display();


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
	pinMode(13, OUTPUT);
	digitalWrite(13, LOW);
	adc_power_off();
}

void readBME280Sensor(){
	bool status = getBme()->begin(0x76, &Wire);
	if (!status) {
		Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
		setStatusBME280Error();
	} else {
		boxTemperature = getBme()->readTemperature();
		Serial.print("Temperature = ");
		Serial.print(boxTemperature);
		Serial.println(" *C");

		Serial.print("Pressure = ");

		boxPressure = roundf(getBme()->readPressure() / 100.0F);
		Serial.print(boxPressure);
		Serial.println(" hPa");

		boxHumidity = roundf(getBme()->readHumidity());
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
	LMIC_setDrTxpow(DR_SF10, 14);
	LMIC_startJoining();
}





void setup() {
	startTime = millis();
	//esp_wifi_start();
	//esp_wifi_stop();
	esp_bt_controller_disable();
	resetStatus();
	VextON();
	bootCount++;
	Serial.begin(115200ul);
	Serial.flush();
	delay(50ul);
	Serial.println();
	Serial.println("Device: ["+String(DEVICE_NAME)+"],  ID="+String(DEVICE_ID));
	Serial.println("Wake up #"+ String(bootCount));
	Serial.println("WU Reason: " + String(get_wakeup_reason()));
	Serial.println("Session status with WU reason : "+String(sessionStatus));

	Serial.print("Time between measurements: ");
	Serial.print(String((unsigned)TIME_BETWEEN_MEASUREMENTS/60) + "m ");
	Serial.println(String((unsigned)TIME_BETWEEN_MEASUREMENTS%60) + "s ");

	getDisplay()->init();
	getDisplay()->setFont(ArialMT_Plain_10);
	getDisplay()->drawString(0, 0, "bufo"+String(DEVICE_ID));
	getDisplay()->display();

	//ds18b20 must be read before other init, in particular before i2c devices
    readDS18B20Sensors();
	initLoRaWAN(bootCount);
    readBME280Sensor();
    readBatteryVoltage();
    do_send(&sendjob, afterLoraPacketSent);
}


void loop() {
	//will not be called after the deepsleep wake up
     os_runloop_once();
}
