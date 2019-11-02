import axios from "axios";

// New interface: multiple sensebox ids, also keep track of their name and location
export const senseBoxes = [
  {"id": "5dbd75c3a68df4001a658ed8", "name": "happyfrog01", "location": [7.551, 47.536, 312.6]},
  {"id": "5dbd82e0a68df4001a69631d", "name": "happyfrog02", "location": [7.55045, 47.53636, 312.1]},
]

// Old interface: assumes single sensebox
export const senseBoxID = "5dbd75c3a68df4001a658ed8";

export function widgetURL(senseBoxID) {
  // Compute the URL for the overview widget for a given sensebox ID.
  return "https://sensebox.github.io/opensensemap-widget/iframe.html?senseboxId=" + senseBoxID + "&initialTab=sensors";
};

export function retrieveSensors(senseBoxID) {
  // Retrieve information for sensors belonging to a given senseBoxID. 
  // Warning: untested code.
  axios.get("https://api.opensensemap.org/boxes/" + senseBoxID + "?format=json").then(
    response => {
      response.data.sensors.map((sensor) => {
        name: sensor.title,
        unit: sensor.unit,
        coordinates: response.data.currentLocation.coordinates,
        senseBoxName: response.data.name,
        senseBoxID: senseBoxID,
      }
    }
  );
};
