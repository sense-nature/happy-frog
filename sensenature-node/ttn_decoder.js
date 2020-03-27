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
	var boxHumidity = bytes[3];

	var boxPressure = ((bytes[4] << 8) | bytes[5]);
	var boxTemperature = ((bytes[6] << 8) | bytes[7]) / 100.0;

	
	var T1 = ((bytes[8] << 8) | bytes[9]) / 100.0;
	var T2 = ((bytes[10] << 8) | bytes[11]) / 100.0;
	var T3 = ((bytes[12] << 8) | bytes[13]) / 100.0;

	return {
		"boxid": boxid,
		"sessionStatus": sessionStatus,
		"batteryVoltage": batteryVoltage,
		"boxHumidity": boxHumidity,
		"boxPressure": boxPressure,
		"boxTemperature": boxTemperature,
		"T1": T1,
		"T2": T2,
		"T3": T3
	};
}