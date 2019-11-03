import time
import binascii
import ttn
import base64
import json
import requests
import argparse
import logging


BASE_URL = "http://www.opensensemap.org:8000/boxes/"

TEMP_PHENOMENON = "temp"
PH_PHENOMENON = "ph"

class Location(object):

    def __init__(self, lat, lng, height):
        self.lat = lat
        self.lng = lng
        self.height = height

    def locationAsDict(self):
        if not self.lat or not self.lng or not self.height:
            return None

        return {
            "lat": self.lat,
            "lng": self.lng,
            "height": self.height
        }

    def __repr__(self):
        return json.dumps(self.locationAsDict())

class Sensor(object):
    def __init__(self, id, phenomenon):
        self.id = id
        self.phenomenon = phenomenon

    def hasTemp(self):
        return self.phenomenon == TEMP_PHENOMENON

    def hasPh(self):
        return self.phenomenon == PH_PHENOMENON


class Box(object):
    def __init__(self, id, box_id, location, sensors):
        self.id = id
        self.box_id = box_id
        self.location = location
        self.sensors = sensors

    def findSensor(self, phenomenon):
        for sensor in self.sensors:
            if sensor.phenomenon == phenomenon:
                logger.debug(f"Found sensor {sensor.id} for {phenomenon}")
                return sensor
        logger.debug(f"Did not find a sensor with {phenomenon} for box '{box.id}'")

    def updateBoxMeasurements(self, temp, ph):
        temp_sensor = self.findSensor(TEMP_PHENOMENON)
        if self.check_update_temp(temp_sensor, temp):
            self.sendMeasurementsUpdate(temp_sensor, temp, TEMP_PHENOMENON)
        else:
            logger.info(f"Do not update temp for '{self.box_id}', temp={temp}, sensor={temp_sensor.id}")

        ph_sensor = self.findSensor(PH_PHENOMENON)
        if self.check_update_ph(ph_sensor, ph):
            self.sendMeasurementsUpdate(ph_sensor, ph, PH_PHENOMENON)
        else:
            logger.info(f"Do not update ph for '{self.box_id}', ph={ph}, sensor={ph_sensor.id}")

    def check_update_temp(self, sensor, temp):
        if not sensor:
            return False
        
        if temp < -100:
            return False

        return True

    def check_update_ph(self, sensor, ph):
        if not sensor:
            return False
        
        if ph <= 0 or ph > 14:
            return False

        return True

    def sendMeasurementsUpdate(self, sensor, value, phenomenon):
        url = BASE_URL + self.box_id + "/" + sensor.id
        payload = {
            "value": value,
            "location": self.location.locationAsDict()
        }

        logger.debug(f"Try sending {payload} to '{url}'")
        r = requests.post(url, json=payload)
        if r.status_code != 201:
            logger.error(f"Error in sending data, {r.text} ({r.status_code})")
        else:
            logger.info(f"Successful updated {phenomenon} measurements to '{self.id}'")


class Ttn2Osm(object):

    def __init__(self, app_id, access_key, boxes):
        self.app_id = app_id
        self.access_key = access_key
        self.boxes = boxes

    def run(self):
        handler = ttn.HandlerClient(self.app_id, self.access_key)

        # using mqtt client
        mqtt_client = handler.data()
        mqtt_client.set_uplink_callback(self.uplink_callback)
        logger.info(f"Start listening on '{self.app_id}'")
        mqtt_client.connect()
        try:
            while True:
                time.sleep(10)
        except KeyboardInterrupt:
            logger.info("Closing MQTT client")
            mqtt_client.close()

    def uplink_callback(self, msg, client):
        logger.info(f"Received uplink from {msg.dev_id}, with '{msg.payload_raw}'")
        try:
            # Prevent python3 to convert bytes into ASCII chars
            bytesAsHexString = base64.b64decode(msg.payload_raw).hex()  # string with hex values
            logger.debug(f"Payload: {bytesAsHexString}")
        except Exception as ex:
            logger.error(f"Error in parsing payload, cause: {ex}, with payload={bytesAsHexString}")
            return

        temp, ph = extractIntValues(bytesAsHexString)
        if not temp or not ph:
            return
        self.updateOsm(msg.dev_id, temp, ph)

    def updateOsm(self, dev_id, temp, ph):
        logger.info(f"New values '{dev_id}', temp={temp}, ph={ph}")
        for box in self.boxes:
            if box.id != dev_id:
                continue
            box.updateBoxMeasurements(temp, ph)

    @staticmethod
    def fromConfig(config):
        """ Generatas a new object from the given config file
        """
        app_id = config.get("api_id", None)
        if not app_id:
            raise Exception("No app id specified")

        access_key = config.get("access_key", None)
        if not access_key:
            raise Exception("No access key specified")

        boxes = []
        for box in config.get("boxes", []):
            id = box.get("id", None)
            if not id:
                raise Exception(f"Invalid box {box}, missing identifier")

            box_id = box.get("box_id", None)
            if not box_id:
                raise Exception(f"Invalid box {id}, missing box identifier")

            location = Location(**box.get("location", None))

            sensors = []
            for sensor in box.get("sensors", []):
                sensor_id = sensor.get("id", None)
                if not sensor_id:
                    raise Exception(f"Invalid sensor in box '{id}', missing 'id'")

                phenomenon = sensor.get("phenomenon", None)
                if not phenomenon:
                    raise Exception(f"Invalid sensor in box '{id}', missing 'phenomenon'")
                sensors.append(Sensor(sensor_id, phenomenon))

            boxes.append(Box(id, box_id, location, sensors))

        return Ttn2Osm(app_id, access_key, boxes)
    

def extractIntValue(bytesAsHexString, signed, startIdx, endIdx):
    if (startIdx < endIdx and endIdx <= len(bytesAsHexString)):
            byte = bytesAsHexString[startIdx:endIdx]

            try:
                value = int.from_bytes(bytearray.fromhex(byte), byteorder='big', signed=signed)
                logger.debug(f"Extracted {value} from {byte}")
                return value
            except Exception as ex:
                logger.error(f"Could not extract integer from {bytesAsHexString}, cause {ex}")

    logger.info(f"Missing bytes to extract value, expected 2 got {int(len(bytesAsHexString)/2)}")
    return None

def extractIntValues(bytesAsHexString):
    if (len(bytesAsHexString) < 8):
        logger.info(f"Missing bytes to extract value, expected 4 got {int(len(bytesAsHexString)/2)}")
        return None, None

    temp_byte = bytesAsHexString[0:4]
    temp = int.from_bytes(bytearray.fromhex(temp_byte), byteorder='big', signed=True)

    ph_byte = bytesAsHexString[4:8]
    ph = int.from_bytes(bytearray.fromhex(ph_byte), byteorder='big', signed=False)

    return temp*.01, ph*.01


logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

def main(args):
    conf_file_path = args.conf
    try:
        conf = json.load(open(conf_file_path, "r"))
        ttn2osm = Ttn2Osm.fromConfig(conf)
        ttn2osm.run()
    except json.JSONDecodeError as ex:
        logger.error(f"Could not read {conf_file_path} cause {ex}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-c", "--conf", help="Path to the config file (default=config.json)", default = "config.json")
    args = parser.parse_args()
    main(args)
