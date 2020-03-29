/*
 * ttn_keys.h
 *
 *  Created on: Mar 21, 2020
 *      Author: zdroyer
 */

#ifndef TTN_KEYS_H_
#define TTN_KEYS_H_

#include <lmic.h>
#include <DallasTemperature.h>

extern uint8_t DEVICE_ID;
extern const char DEVICE_NAME[];

extern uint64_t TIME_BETWEEN_MEASUREMENTS; //[s]

extern u1_t NWKSKEY[16];
extern u1_t APPSKEY[16];
extern u4_t DEVADDR;

extern DeviceAddress DS18B20_SENSORS[3];



#endif /* TTN_KEYS_H_ */
