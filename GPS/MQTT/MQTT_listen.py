import sys

# sys.path.append("/home/vagrant/RIOT/Iot_Guided_Tour/Thingsboard")
sys.path.append("D:\Dropbox\Documents\UA\I-IoT\RIOT-students\Iot_Guided_Tour\Thingsboard")
sys.path.append("C:\Users\Arne\AppData\Local\Programs\Python\Python37\Lib\site-packages")

import paho.mqtt.client as mqtt

from thingsboard import Thingsboard
import logging
import time
import time
import json

# http://www.steves-internet-guide.com/into-mqtt-python-client/

#####################
## Constants
#####################
app_id = "gps_nxt"
access_key = "ttn-account-v2.ODS_7P5rFOgY5X9qMrkBj0ftUhvbYCyTyks56syDHc4"
broker_address = "eu.thethings.network"
tbBroker = "thingsboard.idlab.uantwerpen.be"
tbPort = 1883
tbAccessToken = "BSUetfyzscwX4L41ENOP"
# tbDeviceID = "8e46dd30-1806-11e9-85af-89570d31e3bf"


def on_message(client, userdata, message):
    msgDec = json.loads(message.payload.decode("utf-8"))
    # print("message received ", str(message.payload.decode("utf-8")))
    payloadFields = msgDec["payload_fields"]
    # print("Payload fields ", payloadFields)

    tb_telemetry = {"gps_latitude": float(payloadFields["gps_latitude"]),
                    "gps_longitude": float(payloadFields["gps_longitude"])}
    print("Telemetry: ", tb_telemetry)
    current_ts_ms = int(round(time.time() * 1000))  # current timestamp in milliseconds, needed for Thingsboard
    tbDeviceID = msgDec["dev_id"]

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
client.on_message = on_message  # attach function to callback
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
