// GNU Affero General Public License v3.0
// Based on examples from https://github.com/matthijskooijman/arduino-lmic

//This sketch works properly on Heltec Wireless Stick


#define LMIC_DEBUG_LEVEL 2


#include <Arduino.h>
#include <lmic.h>
#include <lmic/lmic_bandplan_eu868.h>
#include <lmic/lorabase_eu868.h>
#include <hal/hal.h>
#include <SPI.h>
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
    .dio = {26, 34, 35 } // Pins for the Heltec ESP32 Lora board/ TTGO Lora32 with 3D metal antenna
};

void do_send(osjob_t* j){
    // Payload to send (uplink)
	TempMean = 18.3;
    TempMean *= 100;
    TempLora = (int16_t)TempMean;        // Teperatur range -127.00 ... 126.00
    TempLoraHigh = TempLora >> 8;
    TempLoraLow = TempLora & 0x00FF;
    PHMean = 5.53;
    PHMean *= 100;
    PHLora = (uint16_t)PHMean;           // PH range 0 ... 140 (devide by 10 for actual value)
    PHLoraHigh = PHLora >> 8;
    PHLoraLow = PHLora & 0x00FF;
    uint8_t message[] = {TempLoraHigh,TempLoraLow,PHLoraHigh,PHLoraLow};   // I know sending a signed int with a unsigned int is not good.

    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        lmic_tx_error_t ret = LMIC_setTxData2(1, message, sizeof(message), 0);
        Serial.println("Sending uplink packet..., returned: "+String(ret));
        digitalWrite(LEDPIN, HIGH);
        Heltec.display->clear();
        Heltec.display->drawString (0, 0, "SND...");
        //Heltec.display->drawString (0, 15, "Tp: " + String(TempMean) + " �C");    //displaying Temp Messurment Data
        //Heltec.display->drawString (0, 30, "PH: " + String(PHMean));      //displaying PH Messurment Data as Decimal
        Heltec.display->drawString (0, 10,"# "+String (counter));
        Heltec.display->display ();
        counter++;
       // os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }

            Heltec.display->clear();
                   Heltec.display->drawString (0, 0, "SND done!");
                   Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
                   if (LMIC.txrxFlags & TXRX_ACK) {
                     Serial.println(F("Received ack"));
                     Heltec.display->drawString (0, 20, "Got ACK");
                   } else {
                       Serial.println(F("NO ACK received!"));
                       Heltec.display->drawString (0, 20, "No ACK.");
                     }
                   // Schedule next transmission
                   os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
                   digitalWrite(LEDPIN, LOW);
                   Heltec.display->drawString (0, 10, "## "+ String (counter));
                   Heltec.display->display ();


            // Schedule next transmission
            //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}







int getChipRevision()
{
  return (REG_READ(EFUSE_BLK0_RDATA3_REG) >> (EFUSE_RD_CHIP_VER_REV1_S)&&EFUSE_RD_CHIP_VER_REV1_V) ;
}

void printESPRevision() {
  Serial.print("REG_READ(EFUSE_BLK0_RDATA3_REG): ");
  Serial.println(REG_READ(EFUSE_BLK0_RDATA3_REG), BIN);

  Serial.print("EFUSE_RD_CHIP_VER_REV1_S: ");
  Serial.println(EFUSE_RD_CHIP_VER_REV1_S, BIN);

  Serial.print("EFUSE_RD_CHIP_VER_REV1_V: ");
  Serial.println(EFUSE_RD_CHIP_VER_REV1_V, BIN);

  Serial.println();

  Serial.print("Chip Revision (official version): ");
  Serial.println(getChipRevision());

  Serial.print("Chip Revision from shift Operation ");
  Serial.println(REG_READ(EFUSE_BLK0_RDATA3_REG) >> 15, BIN);

}


void setup() {
    Heltec.begin(true, false);
    // LMIC init
     os_init();

     // Reset the MAC state. Session and pending data transfers will be discarded.
     LMIC_reset();
     LMIC_setClockError(MAX_CLOCK_ERROR * 3 / 100);

///	Serial.begin(115200);
    delay(1500);   // Give time for the seral monitor to start up
    Serial.println(F("Starting..."));


    printESPRevision();

    // Use the Blue pin to signal transmission.
    //pinMode(LEDPIN,OUTPUT);

   // reset the OLED
///   pinMode(OLED_RESET,OUTPUT);
///   digitalWrite(OLED_RESET, LOW);
///   delay(50);
///   digitalWrite(OLED_RESET, HIGH);

    ///Heltec.display->init ();
    ///Heltec.display->flipScreenVertically ();
    ///Heltec.display->setFont (ArialMT_Plain_10);

    ///Heltec.display->setTextAlignment (TEXT_ALIGN_LEFT);

    ///Heltec.display->drawString (0, 0, "Init!");
    ///Heltec.display->display ();




    LMIC_setSession (0x13, DEVADDR, NWKSKEY, APPSKEY);

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


    // Start job
    do_send(&sendjob);

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


}

void loop() {


    unsigned long now;
      now = millis();
      if ((now & 512) != 0) {
        digitalWrite(13, HIGH);
      }
      else {
        digitalWrite(13, LOW);
      }

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