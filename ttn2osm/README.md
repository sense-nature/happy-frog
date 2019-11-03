# TTN to OpenSenseMap

Connect to the TTN server via MQTT and send the arrived events to OpenSenseMap.

## Installation
```
python -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

## Setup
Copy the 'config_TEMPLATE.json' to config.json and fill in the missing values.

## Run
```
python ./ttn2osm.py --conf <PATH_TO_CONFIG_FILE>
```
or
```
python ./ttn2osm.py
```
when the 'config.json' is at the same pleace where you start the application.

## Communication TTN
The communication to the TTN network is done over the MQTT. This means that the sent data is wrapped in a Base64 encoded string.
The data from the sensors is represented by 4 bytes. First two bytes represents the temperature and the last two bytes represents the pH value.
The sensors should always send 4 bytes. If not the data will be discarded.

| Bytes | Value       | Signed |
| ----- |:------------|:-------|
| [0,1] | Temperature | True   |
| [2,3] | pH          | False  |

The temperature as the pH values need to be divided by 100 to get the real value.
