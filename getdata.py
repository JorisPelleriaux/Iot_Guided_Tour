import paho.mqtt.client as mqtt
from bitstring import ConstBitStream
from pymongo import MongoClient

import sys
sys.path.insert(0,'C:\pyd7a')

from pprint import pprint
from d7a.alp.parser import Parser as AlpParser
from d7a.dll.parser import Parser as DllFrameParser, FrameType
from d7a.serial_console_interface.parser import Parser as SerialParser
from d7a.system_files.system_file_ids import SystemFileIds
from d7a.system_files.system_files import SystemFiles

import json

def write_to_jsonData(counter, deviceID, gatewayID, rx_level, x, y):
    if db.DataSet.find({'counter': counter}).count() == 1:
        db.DataSet.update({"counter": counter}, {"$addToSet": {'gateways': {'gatewayID': gatewayID,'rx_level': rx_level}}})
    else:
        jsonData = {
            'counter': counter,
            'deviceID': deviceID,
            'gateways': [{
                'gatewayID': gatewayID,
                'rx_level': rx_level
            }],
            'x': x,
            'y': y
        }
        # Insert data object directly into MongoDB via insert_one
        result = db.DataSet.insert_one(jsonData)  # Collection name
        # Print to the console the ObjectID of the new document
        print('Done with id: {0}'.format(result.inserted_id))

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    # subscribe for all devices of user
    client.subscribe('+/d7/#')


# The callback for when a PUBLISH message is received from the server.
# "Device-ID: ", topic[2]
# "Gateway-ID: ", topic[3]
# "File-ID: ", payload.actions[0].operand.offset.id
# "rx-level: ", payload.interface_status.operand.interface_status.rx_level
#
def on_message(client, userdata, msg):
    data = []
    topic = msg.topic.split("/")

    if str(topic[2]) == "4933323400370020":
        payloadString = str(msg.payload)
        payloadArray = bytearray(payloadString.decode("hex"))
        payload = AlpParser().parse(ConstBitStream(payloadArray), len(payloadArray))

        data.append(str(payload.actions[0].operand.data))
        data.append(str(topic[2]))
        data.append(str(topic[3]))
        data.append(str(payload.interface_status.operand.interface_status.rx_level))

        print "Counter: ", data[0]
        print "Device-ID: ", data[1]
        print "Gateway-ID: ", data[2]
        print "Rx-level: ", data[3]
        print "---------------------------"

        if raw_input("Confirm? y/n ") == 'y':
            location_raw = raw_input("Give x,y ")
            location = location_raw.split(",")
            print data[0], " ", data[1], " ", data[2], " ", location[0], " ", location[1]
            write_to_jsonData(data[0], data[1], data[2], data[3], location[0], location[1])
            data.clear()

client = mqtt.Client()

client.on_connect = on_connect
client.on_message = on_message

client.connect("backend.idlab.uantwerpen.be", 1883, 60)

# connect to MongoDB
DBclient = MongoClient("127.0.0.1")
db = DBclient.FingerprintDB  #Database name

# and listen to server
run = True
while run:
    client.loop()
