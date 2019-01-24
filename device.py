from localization import Localization
import paho.mqtt.client as mqtt
import threading
import time
import logging
import json
from thingsboard import Thingsboard
from bitstring import ConstBitStream
from pyd7a.d7a.alp.parser import Parser as AlpParser

# Create global logger
# logger = logging.getLogger('tb_example')
# formatstring = "%(asctime)s - %(name)s:%(funcName)s:%(lineno)i - %(levelname)s - %(message)s"
# logging.basicConfig(format=formatstring, level=logging.DEBUG)

with open('keys.json') as f:
    keys = json.load(f)

#####################
## Constants
#####################
app_id = "gps_nxt"


class Device:
    def __init__(self):
        self.localization = Localization('127.0.0.1', 'FingerprintDB', 'DataSet')
        self.device_id = '493332340046001f'  # axel '493332340032001f', arne:493332340046001f, joris: 4933323400370020
        self.processor = threading.Thread()  # empty thread
        self.training = False

        self.x_training = 1
        self.y_training = 0
        self.queue_d7 = {}  # queue for incoming D7 messages

        # Subscribe to Dash-7
        try:
            self.clientD7 = mqtt.Client()
            self.clientD7.on_message = self.on_messageD7

            print('connecting to broker Dash-7')
            self.clientD7.connect("backend.idlab.uantwerpen.be", 1883, 60)
            self.clientD7.subscribe('/d7/' + self.device_id + '/#')
        except:
            print ('connection to Dash-7 failed')

        # Subscribe to Lora
        try:
            self.clientLora = mqtt.Client('Lora')  # create new instance
            self.clientLora.username_pw_set(app_id, password=keys['ttn']['access_key'])
            self.clientLora.on_message = self.on_messageLora  # attach function to callback
            print("connecting to broker Lora")
            self.clientLora.connect(keys['ttn']['broker_address'])  # connect to broker
            self.clientLora.subscribe("+/devices/+/up")
        except:
            print ('connection to Lora failed')

        self.tb = Thingsboard(keys['thingsboard']['url'], 1883, keys['thingsboard']['access_token'])

    def on_messageD7(self, client, userdata, msg):
        topic = msg.topic.split("/")

        # Decode message
        decoded = []
        payloadString = str(msg.payload)
        payloadArray = bytearray(payloadString.decode("hex"))
        payloadself = AlpParser().parse(ConstBitStream(payloadArray), len(payloadArray))

        decoded.append(str(payloadself.interface_status.operand.interface_status.rx_level))

        hardware_id = topic[2]
        print(hardware_id)
        gateway_id = topic[3]
        self.queue_d7[gateway_id] = int(decoded[0])  # save rx_level for every receiving gateway

        if not self.processor.is_alive():
            print('Thread started')
            self.processor = threading.Thread(target=self.process_data_counter, args=[hardware_id])
            print('Thread created')
            self.processor.start()
        print('Thread started')

    def on_messageLora(self, client, userdata, message):
        msgDec = json.loads(message.payload.decode("utf-8"))
        # print("message received ", str(message.payload.decode("utf-8")))
        payloadFields = msgDec["payload_fields"]
        # print("Payload fields ", payloadFields)

        tb_telemetry = {"gps_latitude": float(payloadFields["gps_latitude"]),
                        "gps_longitude": float(payloadFields["gps_longitude"])}
        print("Telemetry: ", tb_telemetry)
        current_ts_ms = int(round(time.time() * 1000))  # current timestamp in milliseconds, needed for Thingsboard
        # tbDeviceID = msgDec["dev_id"]

        self.tb.sendDeviceTelemetry(keys['ttn']['tbDeviceID'], current_ts_ms, tb_telemetry)

        # send non-numeric data ('attributes') to Thingsboard as JSON. Example:
        tb_attributes = {'last_data_rate': str(msgDec['metadata']['data_rate'])}
        # print("Attributes", tb_attributes)
        self.tb.sendDeviceAttributes(keys['ttn']['tbDeviceID'], tb_attributes)

    def process_data_counter(self, device_id):
        time.sleep(3)
        print('queue', self.queue_d7)
        # training mode
        if self.training:
            self.localization.training(self.x_training, self.y_training, self.queue_d7)
            print('added to DB')

        # localize mode
        print ('ontvangen door ' + str(len(self.queue_d7)))
        if len(self.queue_d7) >= 3:
            location = self.localization.localize(self.queue_d7, 10)  # k-nearest
            print ('Location is approximately x: ' + str(location['x']) + ', ' + 'y: ' + str(location['y']))
        else:
            print ('Not enough gateways to determine the location')

        if not self.training & len(self.queue_d7) >= 3:  # estimate location if not training, 3 or more gateways
            self.data_to_tb(keys['ttn']['tbDeviceID'], location)  # change to device_id'IoT_Tour_Joris'

        self.queue_d7 = {}  # clear queue

    def data_to_tb(self, device_id, location):
        print('Sending data to ThingsBoard')
        tb_telemetry = {'x': float(location['x']), 'y': float(location['y']), 'gateways': len(self.queue_d7)}

        current_ts_ms = int(round(time.time() * 1000))  # current timestamp in milliseconds, needed for Thingsboard
        self.tb.sendDeviceTelemetry(device_id, current_ts_ms, tb_telemetry)
        # logger.info("Done!")


# Create devices
dev1 = Device()
while True:
    dev1.clientD7.loop()
    dev1.clientLora.loop()
