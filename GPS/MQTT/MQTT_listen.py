import sys
import os

# Import the Thingsboard files with relative path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..\Thingsboard')))

import paho.mqtt.client as mqtt

from thingsboard import Thingsboard
import logging
import time
import time
import json

with open('../../keys.json') as f:
    keys = json.load(f)

# http://www.steves-internet-guide.com/into-mqtt-python-client/

#####################
## Constants
#####################
app_id = "gps_nxt"
access_key = keys['ttn']['access_key']
broker_address = "eu.thethings.network"
tbBroker = "thingsboard.idlab.uantwerpen.be"
tbPort = 1883
tbAccessToken = keys['ttn']['tbAccessToken']
tbDeviceID = keys['ttn']['tbDeviceID']


def on_messageLora(client, userdata, message):
    msgDec = json.loads(message.payload.decode("utf-8"))
    # print("message received ", str(message.payload.decode("utf-8")))
    payloadFields = msgDec["payload_fields"]
    # print("Payload fields ", payloadFields)

    tb_telemetry = {"gps_latitude": float(payloadFields["gps_latitude"]),
                    "gps_longitude": float(payloadFields["gps_longitude"])}
    print("Telemetry: ", tb_telemetry)
    current_ts_ms = int(round(time.time() * 1000))  # current timestamp in milliseconds, needed for Thingsboard
    # tbDeviceID = msgDec["dev_id"]

    tb.sendDeviceTelemetry(tbDeviceID, current_ts_ms, tb_telemetry)

    # send non-numeric data ('attributes') to Thingsboard as JSON. Example:
    tb_attributes = {'last_data_rate': str(msgDec['metadata']['data_rate'])}
    # print("Attributes", tb_attributes)
    tb.sendDeviceAttributes(tbDeviceID, tb_attributes)


#####################
## Init
#####################
logger = logging.getLogger('tb_example')
formatstring = "%(asctime)s - %(name)s:%(funcName)s:%(lineno)i - %(levelname)s - %(message)s"
logging.basicConfig(format=formatstring, level=logging.DEBUG)
tb = Thingsboard(tbBroker, tbPort, tbAccessToken)

client = mqtt.Client("Nxt-1")  # create new instance
client.username_pw_set(app_id, password=access_key)
client.on_message = on_messageLora  # attach function to callback
print("connecting to broker")
client.connect(broker_address)  # connect to broker

#####################
## Listen
#####################
client.loop_start()  # start the loop
print("Subscribing to topic", "+/devices/+/up")
client.subscribe("+/devices/+/up")
# print("Publishing message to topic","+/devices/+/up")
# client.publish("+/devices/+/up","OFF")
time.sleep(600)  # wait
client.loop_stop()  # stop the loop

logger.info("Done!")
