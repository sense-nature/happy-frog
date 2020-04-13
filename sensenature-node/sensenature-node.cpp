// GNU Affero General Public License v3.0
// Based on examples from https://github.com/matthijskooijman/arduino-lmic

//This sketch works properly on Heltec Wireless Stick


#define LMIC_DEBUG_LEVEL 2

#include <Arduino.h>
#include <pins_arduino.h>
#include <SPI.h>

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


SSD1306Wire display(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_64_32);
Adafruit_BME280 bme;

#include <WiFi.h>
const char* ntpServer = "pool.ntp.org";

#include <time.h>

#include <SPIFFS.h>
bool spiffs_initialized = false;

#include <Update.h>

//https://github.com/yurilopes/SPIFFSIniFile
#include <SPIFFSIniFile.h>
const size_t ini_buffer_len = 80;
char ini_buffer[ini_buffer_len];

const char * wifi_settings = "/wifi.txt";

//https://github.com/JChristensen/DS3232RTC
#include <DS3232RTC.h> 
DS3232RTC ds3231(false);
volatile bool alarm_triggered = false;
uint8_t current_alarm = 0;


#include <WebServer.h>
WebServer server(80);
File fsUploadFile;


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
void goToDeepSleep(void *pUserData, int fSuccess){

   // Heltec.display->clear();
  Serial.write("Sending result: ");
   if( fSuccess){
	   //display.drawString (20, 0, " SENT");
	   Serial.println("OK");
   } else {
	   //display.drawString (20, 0, " FAILED");
	   Serial.println("FAILED");
   }
   // Schedule next transmission
   //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
   LedOFF();

   LMIC_shutdown();
   if( firstRun() )
	   delay(5000u);
   unsigned long delta = millis() - startTime;
   Serial.println("Work time: "+String(delta)+"ms");
   Serial.println("Going to sleep now");
   Serial.flush();
   esp_sleep_enable_timer_wakeup(TIME_BETWEEN_MEASUREMENTS * uS_TO_S_FACTOR - delta * mS_TO_S_FACTOR);
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


void push2BytesToMessage(std::vector<uint8_t> & vect, int16_t iValue){
	vect.push_back(getHigh(iValue));
	vect.push_back(getLow(iValue));
}


void pushTemperatureToMessage(std::vector<uint8_t> & vect, float fTemperature){
	int16_t iTemp = (int16_t)roundf(fTemperature*100.0f);
	push2BytesToMessage(vect, iTemp);
}


void do_send(osjob_t* j, void(*callBack)(void *, int)){

	display.drawString (0, 15, "0x"+ String(sessionStatus,16));
	display.display();
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
			 memcpy(addr, DS18B20_SENSORS[i], sizeof(addr));;
			 if(ds18b20.isConnected(addr) ){
				 Serial.print("Temperature @ ");
				 printAddress(addr);
				 temp[i] = ds18b20.getTempC(addr);
				 Serial.println(": " + String(temp[i]) + " *C");
				 display.drawString(22, i*10, String(i+1) + ": "+ String(temp[i]));
				 display.display();
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


void start_wifi() {  
  if( spiffs_initialized ) {
    SPIFFSIniFile wifi_ini(wifi_settings);
    if( SPIFFS.exists(wifi_settings) && wifi_ini.open() ){
      Serial.println("Connecting to wifi");
      wifi_ini.getValue("wifi", "ssid", ini_buffer, ini_buffer_len);
      String ssid(ini_buffer);
      Serial.println(ssid);
      wifi_ini.getValue("wifi", "password", ini_buffer, ini_buffer_len);
      String password(ini_buffer);
      Serial.println(password);
      WiFi.begin(ssid.c_str(), password.c_str());
      Serial.print("connecting to wifi ");
      for(int i = 0; i < 10000; i+=500){
        Serial.print(".");
        if(WiFi.status() == WL_CONNECTED){
          Serial.println("!");
          break;
        }
        delay(500);
      }
    }
  }
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Starting AP");
    WiFi.disconnect();
    WiFi.softAP("sensenature_node_AP", "sensenature");
  }
}


String getContentType(String filename) {
  if (server.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}


bool exists(String path){
  bool there = false;
  File file = SPIFFS.open(path, "r");
  if(!file.isDirectory()){
    there = true;
  }
  file.close();
  return there;
}


bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) {
    path += "index.htm";
  }
  String contentType = getContentType(path);
  if (exists(path)) {
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}


void handleFileUpload() {
  if (server.uri() != "/edit") {
    return;
  }
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    Serial.print("handleFileUpload Name: "); 
    Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    Serial.print("handleFileUpload Size: "); 
    Serial.println(upload.totalSize);
  }
}


void handleFileDelete() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  Serial.println("handleFileDelete: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (!exists(path)) {
    return server.send(404, "text/plain", "FileNotFound");
  }
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
}


void handleFileCreate() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  Serial.println("handleFileCreate: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (exists(path)) {
    return server.send(500, "text/plain", "FILE EXISTS");
  }
  File file = SPIFFS.open(path, "w");
  if (file) {
    file.close();
  } else {
    return server.send(500, "text/plain", "CREATE FAILED");
  }
  server.send(200, "text/plain", "");
  path = String();
}


void handleFileList() {
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = server.arg("dir");
  Serial.println("handleFileList: " + path);


  File root = SPIFFS.open(path);
  path = String();

  String output = "[";
  if(root.isDirectory()){
      File file = root.openNextFile();
      while(file){
          if (output != "[") {
            output += ',';
          }
          output += "{\"type\":\"";
          output += (file.isDirectory()) ? "dir" : "file";
          output += "\",\"name\":\"";
          output += String(file.name()).substring(1);
          output += "\"}";
          file = root.openNextFile();
      }
  }
  output += "]";
  server.send(200, "text/json", output);
}


void handleNtpUpdate() {
  // update rtc from ntp request
  Serial.println("updating from from NTP");
  if (server.hasArg("server")) {
    configTime(0L, 0, server.arg("server").c_str());
  } else {
    configTime(0L, 0, ntpServer);
  }
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    server.send(500, "text/plain", "Failed to obtain time");
    return;
  }
  
  time_t t_now = mktime(&timeinfo);
  setTime(t_now);
  ds3231.set(t_now);
  
  char time_buf[64];
  size_t written = strftime(time_buf, 64, "%A, %B %d %Y %H:%M:%S", &timeinfo);
  Serial.println(time_buf);
  server.send(200, "text/plain", time_buf);
}


void handleGetTime() {
  char time_buf[64];
  time_t t_now = now();
  size_t written = strftime(time_buf, 64, "%A, %B %d %Y %H:%M:%S", localtime(&t_now));
  Serial.println(time_buf);
  server.send(200, "text/plain", time_buf);
}


void handleSpiffsFree() {
  if(spiffs_initialized){
    String output = "used/total bytes: ";
    output += String(SPIFFS.usedBytes());
    output += "/";
    output += String(SPIFFS.totalBytes());
    server.send(200, "text/plain", output);
  } else {
    server.send(500, "spiffs not initialized");
  }
}


void handleFileSize() {
  if (!server.hasArg("file")) {
    Serial.println("missing file argument");
    server.send(500, "text/plain", "missing file argument");
    return;
  }
  
  String file_name = server.arg("file");
  if (!exists(file_name)) {
    Serial.println("file does not exist");
    server.send(500, "text/plain", "file does not exists");
    return;
  }
  
  File file = SPIFFS.open(file_name);
  if (!file) {
    Serial.println("file open failed");
    server.send(500, "text/plain", "file open failed");
    return;
  }  
  size_t file_size = file.size();
  String fsize = file_name + ": " + String(file_size) + " B";
  Serial.println(fsize);
  server.send(200, "text/plain", fsize);
}


void handleSpiffsFormat() {
  Serial.println("spiffs format requested");
  if (!server.hasArg("sure")) {
    Serial.println("formatting failed; send \"?sure=yes\" along");
    server.send(500, "text/plain", "BAD ARGS, send ?sure=yes along if you're sure");
    return;
  } 
  String sure = server.arg("sure");
  if (sure != "yes") {
    Serial.println("formatting failed; send \"?sure=yes\" along");
    server.send(500, "text/plain", "BAD ARGS, send ?sure=yes along if you're sure");
    return;
  }
  Serial.println("format request confirmed, formatting");
  bool formatted = SPIFFS.format();
  if(formatted) {
    Serial.println("formatting succesful"); 
    server.send(200, "text/plain", "format succesfull");
  } else {
    Serial.println("formatting failed");
    server.send(500, "text/plain", "format failed");
  }
}


void updateFirmware() {
  Serial.println("update fw");
  if (!server.hasArg("file")) {
    Serial.println("missing file argument");
    server.send(500, "text/plain", "missing file argument");
    return;
  }
  
  String fw_file = server.arg("file");
  if (!exists(fw_file)) {
    Serial.println("file does not exist");
    server.send(500, "text/plain", "fw-update: file does not exists");
    return;
  }
  
  File file = SPIFFS.open(fw_file);
  if (!file) {
    Serial.println("file open failed");
    server.send(500, "text/plain", "fw-update: file open failed");
    return;
  }  

  Serial.println("starting update");
  size_t file_size = file.size();
  if(!Update.begin(file_size)){
    Serial.println("cannot start update");
    server.send(500, "text/plain", "fw-update: cannot start update");
    return;
  }

  Serial.println("write stream");
  Update.writeStream(file);

  if(Update.end()) {
    Serial.println("update successful, rebooting");
    server.send(200, "text/plain", "fw-update: update successful, rebooting");
  } else {
    Serial.println("update failed, error Occurred: " + String(Update.getError()) );
    server.send(500, "text/plain", "fw-update failed: " + String(Update.getError()) );
  }

  file.close();
  SPIFFS.remove(fw_file);
  delay(1000);
  ESP.restart();
}


void start_webserver() {
  Serial.print("Open http://");
  Serial.print(WiFi.localIP());
  Serial.println("/edit to see the file browser");
  
  //SERVER INIT
  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });

  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);

  server.on("/ntp_update", HTTP_GET, handleNtpUpdate);
  server.on("/get_time", HTTP_GET, handleGetTime);

  server.on("/format", HTTP_GET, handleSpiffsFormat);
  server.on("/delete", HTTP_GET, handleFileDelete);

  server.on("/file_size", HTTP_GET, handleFileSize);
  server.on("/free_space", HTTP_GET, handleSpiffsFree);
  server.on("/update", HTTP_GET, updateFirmware);

  //called when the url is not defined here
  //use it to load content from FILESYSTEM
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  
  //get heap status, analog input value and all GPIO statuses in one json call
  server.on("/all", HTTP_GET, []() {
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += ", \"analog\":" + String(analogRead(A0));
    json += ", \"gpio\":" + String((uint32_t)(0));
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });
  server.begin();
  Serial.println("HTTP server started");
  
}



void setup() {
	startTime = millis();
	resetStatus();
	VextON();
	bootCount++;

	Serial.begin(115200ul);
	Serial.flush();
	delay(50ul);

  spiffs_initialized = SPIFFS.begin();

  Wire.begin(4, 15);

  time_t epoch = ds3231.get();
  setTime(epoch);
  
  if(spiffs_initialized) {
    Serial.println("spiffs initialized");
  } else {
    Serial.println("nope");
  }
  if(esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_EXT0) {
    // wake up not from rtc -> assume reset 
    start_wifi();
    start_webserver();
  } else {
    // rtc wakeup
  }
  Serial.println(esp_sleep_get_wakeup_cause());
  
	Serial.println();
	Serial.println("Device: ["+String(DEVICE_NAME)+"],  ID="+String(DEVICE_ID));
	Serial.println("Wake up #"+ String(bootCount));
	Serial.println("WU Reason: " + String(get_wakeup_reason()));

	Serial.print("Time between measurements: ");
	Serial.print(String((unsigned)TIME_BETWEEN_MEASUREMENTS/60) + "m ");
	Serial.println(String((unsigned)TIME_BETWEEN_MEASUREMENTS%60) + "s ");


	display.init();
	display.setFont(ArialMT_Plain_10);
	display.drawString(0, 0, "#"+String(DEVICE_ID));
	display.display();

	//ds18b20 must be read before other init, in particular before i2c devices
//    readDS18B20Sensors();
//	initLoRaWAN(bootCount);
//    readBME280Sensor();
//    readBatteryVoltage();
//    do_send(&sendjob, goToDeepSleep);
}


void loop() {
  //will not be called after the deepsleep wake up
  //os_runloop_once();
  //delay(100);
  server.handleClient();
}
