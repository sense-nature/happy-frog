import time
import binascii
import ttn
import base64
import json
import requests
import argparse

BASE_URL = "http://www.opensensemap.org:8000/boxes/"

# TODO: Set the correct ids
SENSORS = {
    "happy-frog01": "",
    "happy-frog02": ""
}


def send_data_to_osm(app_id, device_id, temp, ts):
    url = BASE_URL + SENSORS[device_id]
    payload = {"value": temp}
    r = requests.post(url, json=payload)
    if r.status_code != 201:
        print("Could not send temprature, " + r.status_code)
    else:
        print(f"Sucess sending {temp} to {url}")


def uplink_callback(msg, client):
    print(f"Received uplink from {msg.dev_id}")
    try:
        payload = base64.urlsafe_b64decode(msg.payload_raw)
        data = json.loads(payload.decode('utf-8'))
    except Exception as ex:
        print(f"Error in parsing payload, cause: {ex}, payload={payload}")
        return
    send_data_to_osm(msg.app_id, msg.dev_id, data['temperature'], None)


def run(app_id, access_key):
    print(f"Start client on '{app_id}'\n")
    handler = ttn.HandlerClient(app_id, access_key)
    
    # using mqtt client
    mqtt_client = handler.data()
    mqtt_client.set_uplink_callback(uplink_callback)
    mqtt_client.connect()
    try:
        while True:
            time.sleep(10)
    except KeyboardInterrupt:
        print("Closing client, bye")
        mqtt_client.close()


def main(args):
    run(args.app_id, args.access_key)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("app_id", help="The TTN application identifier")
    parser.add_argument("access_key", help="The TTN application access key")
    args = parser.parse_args()
    main(args)
