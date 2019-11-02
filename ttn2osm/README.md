# TTN to OpenSenseMap

Connect to the TTN server via MQTT and send the arrived events to OpenSenseMap.

## Installation
```
python -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

## Setup
Set the right 'SENSORS' endpoints in the 'ttn2osm.py' file. 

## Run
python ./ttn2osm.py <APP_ID> <ACCESS_KEY>