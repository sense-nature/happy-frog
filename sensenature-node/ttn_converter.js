function Converter(decoded, port) {
  // Merge, split or otherwise
  // mutate decoded fields.
  var converted = {};
    	  // Measurement status
  converted["5e81b625f7afec001bbfe726"] = decoded.sessionStatus;
  // Battery
  converted["5e81b625f7afec001bbfe725"] = decoded.batteryVoltage;

// Inside Device Temperature
  converted["5e81b625f7afec001bbfe724"] = decoded.boxTemperature;

// Inside Device Humidity
  converted["5e81b625f7afec001bbfe723"] = decoded.boxHumidity;

// Inside Device Pressure
  converted["5e81b625f7afec001bbfe722"] = decoded.boxPressure;

// T1 Temperature
  converted["5e81b625f7afec001bbfe721"] = decoded.T1;

// T2 Temperature
  converted["5e81b625f7afec001bbfe720"] = decoded.T2;

// T3 Temperature
  converted["5e81b625f7afec001bbfe71f"] = decoded.T3;

  return converted;
}