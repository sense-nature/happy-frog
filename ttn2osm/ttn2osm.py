import time
import binascii
import ttn
import base64
import json
import requests
import argparse
import logging


BASE_URL = "http://www.opensensemap.org:8000/boxes/"

# TODO: Set the correct ids
TEMP_SENSORS = {
    "happy-frog01": "",
    "happy-frog02": ""
}

PH_SENSORS = {
    "happy-frog02": ""
}

SENSOR_LOCATIONS = {
    "happy-frog01": {
        "lat": 47.53636, "lng": 7.55045, "height": 312.1

    },
    "happy-frog02": {
        "lat": 47.536, "lng": 7.551, "height": 312.6
    }
}

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)


def send_temp_to_osm(app_id, device_id, temp=None, ts=None):
    if not temp:
        logger.info("No temp data to send")
        return
    
    url = BASE_URL + TEMP_SENSORS.get(device_id, "")
    payload = {
        "value": temp,
        "location": SENSOR_LOCATIONS.get(device_id, None)
        }
    send_data_to_osm(url, payload)

def send_ph_to_osm(app_id, device_id, ph=None, ts=None):
    if not ph:
        logger.info("No ph data to send")
        return
    
    url = BASE_URL + PH_SENSORS.get(device_id, "")
    payload = {
        "value": ph,
        "location": SENSOR_LOCATIONS.get(device_id, None)
        }
    send_data_to_osm(url, payload)
    

def send_data_to_osm(url, payload):
    logger.debug(f"Sending {payload} to '{url}'")
    r = requests.post(url, json=payload)
    if r.status_code != 201:
        logger.error("Could not send data, " + r.status_code)
    else:
        logger.info(f"Successful  sent {payload} to '{url}'")

def uplink_callback(msg, client):
    logger.info(f"Received uplink from {msg.dev_id}, {msg.payload_raw}")
    try:
        # Prevent python3 to convert bytes into ASCII chars
        payload = base64.b64decode(msg.payload_raw).hex()
        logger.debug(f"Payload: {payload}, {type(payload)}")
    except Exception as ex:
        logger.error(f"Error in parsing payload, cause: {ex}, payload={payload}")
        return
    
    try:
        # First byte represents the temprature
        temp = int.from_bytes(bytearray.fromhex(payload[0:2]), byteorder='big', signed=True)
        send_temp_to_osm(msg.app_id, msg.dev_id, temp, None)

        # Second byte represents the ph value, may be missing
        if len(payload) > 2:
            ph = int.from_bytes(bytearray.fromhex(payload[2:4]), byteorder='big', signed=False)

            # The ph value is multiplied with 10 to send single integers. We need to retransform it
            send_ph_to_osm(msg.app_id, msg.dev_id, ph/10.0, None)
    except Exception as ex:
        logger.error(f"Error in handling callback, {ex}")


def run(app_id, access_key):
    logger.info(f"Start client on '{app_id}'\n")
    handler = ttn.HandlerClient(app_id, access_key)
    
    # using mqtt client
    mqtt_client = handler.data()
    mqtt_client.set_uplink_callback(uplink_callback)
    mqtt_client.connect()
    try:
        while True:
            time.sleep(10)
    except KeyboardInterrupt:
        logger.info("Closing client, bye")
        mqtt_client.close()


def main(args):
    run(args.app_id, args.access_key)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("app_id", help="The TTN application identifier")
    parser.add_argument("access_key", help="The TTN application access key")
    args = parser.parse_args()
    main(args)
