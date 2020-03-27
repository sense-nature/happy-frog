function Decoder(bytes, port) {
	// Decode an uplink message from a buffer to an object of fields
	// bytes: array of uitn8_t, the message data
	// port: uint8_t, the LoRaWAN FPort param
	var boxids = ["none", "bufo01", "bufo01", "bufo03"]
	var serialNo = port;
	var boxid = "unknown";
	if (serialNo > 0 && serialNo < boxids.length)
		boxid = boxids[serialNo];

	var sessionStatus = bytes[0];	
	var batteryVoltage = ((bytes[1] << 8) | bytes[2]);
	
	var retObject = { boxid: boxid, sessionStatus: sessionStatus , batteryVoltage: batteryVoltage};
	if( ! (sessionStatus & (0x01<<1)) ){
		//there was not BME280 error, read the data
		retObject.boxHumidity = bytes[3];
		retObject.boxPressure = ((bytes[4] << 8) | bytes[5]);
		retObject.boxTemperature = ((bytes[6] << 8) | bytes[7]) / 100.0;
	}
	
	for(var i=1; i<=3; i++ ){
	  var idx = 8 + 2 * (i-1);
		if( ! (sessionStatus & (0x01<<(i+1))) )
			retObject["T"+i] = ((bytes[idx] << 8) | bytes[idx+1]) / 100.0;
	}

	return retObject;
}