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
logger = logging.getLogger('tb_example')
formatstring = "%(asctime)s - %(name)s:%(funcName)s:%(lineno)i - %(levelname)s - %(message)s"
logging.basicConfig(format=formatstring, level=logging.DEBUG)

with open('keys.json') as f:
    keys = json.load(f)


class Device:
    def __init__(self):
        self.localization = Localization('127.0.0.1', 'FingerprintDB', 'DataSet')
        self.device_id = {'493332340046001f': 'iot_guide_arne', '493332340032001f': 'iot_guide_axel',
                          '4933323400370020': 'iot_guide_joris'}
        self.processor = threading.Thread()  # empty thread for incoming messages
        self.training = False

        self.location = None  # current location

        self.x = 1
        self.y = 0
        self.queue_d7 = {}  # queue for incoming D7 messages

        # Subscribe to Dash-7
        try:
            self.clientD7 = mqtt.Client()
            self.clientD7.on_message = self.on_message_dash7  # attach function to callback

            logger.info('connecting to broker Dash-7')
            self.clientD7.connect("backend.idlab.uantwerpen.be", 1883, 60)  # connect to broker
            self.clientD7.subscribe('/d7/#')
        except:
            logger.error('connection to Dash-7 failed')

        # Subscribe to Lora
        try:
            self.clientLora = mqtt.Client('Lora')  # create new instance
            self.clientLora.username_pw_set(keys['ttn']['app_id'], password=keys['ttn']['access_key'])
            self.clientLora.on_message = self.on_message_lora  # attach function to callback
            logger.info("connecting to broker Lora")
            self.clientLora.connect(keys['ttn']['broker_address'])  # connect to broker
            self.clientLora.subscribe("+/devices/+/up")
        except:
            logger.error('connection to Lora failed')

        self.tb = Thingsboard(keys['thingsboard']['url'], 1883, keys['thingsboard']['access_token'])

    def on_message_dash7(self, client, userdata, msg):
        topic = msg.topic.split("/")
        hardware_id = topic[2]
        dev_ids = self.device_id.keys()

        if hardware_id == dev_ids[0] or hardware_id == dev_ids[1] or hardware_id == dev_ids[2]:
            # Decode message
            decoded = []
            payloadString = str(msg.payload)
            payloadArray = bytearray(payloadString.decode("hex"))
            payloadself = AlpParser().parse(ConstBitStream(payloadArray), len(payloadArray))
            decoded.append(str(payloadself.interface_status.operand.interface_status.rx_level))

            logger.info('Dash7 from: ' + hardware_id)
            gateway_id = topic[3]
            self.queue_d7[gateway_id] = int(decoded[0])  # save the rx_level

            if not self.processor.is_alive():
                self.processor = threading.Thread(target=self.process_dash7_messages, args=[hardware_id])
                self.processor.start()

    def on_message_lora(self, client, userdata, message):
        msgDec = json.loads(message.payload.decode("utf-8"))
        logger.info('Lora from: ' + msgDec["dev_id"])
        payloadFields = msgDec["payload_fields"]
        # print("Payload fields ", payloadFields)

        tb_telemetry = {"gps_latitude": float(payloadFields["gps_latitude"]),
                        "gps_longitude": float(payloadFields["gps_longitude"])}
        print("Telemetry: ", tb_telemetry)
        current_ts_ms = int(round(time.time() * 1000))  # current timestamp in milliseconds, needed for Thingsboard
        # tbDeviceID = msgDec["dev_id"]
        # print("Dev-id: ", msgDec["dev_id"])

        self.tb.sendDeviceTelemetry(msgDec["dev_id"], current_ts_ms, tb_telemetry)

        # send non-numeric data ('attributes') to Thingsboard as JSON. Example:
        tb_attributes = {'last_data_rate': str(msgDec['metadata']['data_rate'])}
        # print("Attributes", tb_attributes)
        self.tb.sendDeviceAttributes(msgDec["dev_id"], tb_attributes)

    def process_dash7_messages(self, device_id):
        time.sleep(3)
        # print('queue', self.queue_d7)
        # if in training mode
        if self.training:
            self.localization.training(self.x, self.y, self.queue_d7)
            logger.info('added to DB')

        # localize mode
        logger.info('received by ' + str(len(self.queue_d7)) + ' gateways')
        if len(self.queue_d7) >= 3:
            self.location = self.localization.localize(self.queue_d7, 43)  # k-nearest number
            logger.info(
                'Location is approximately x: ' + str(self.location['x']) + ', ' + 'y: ' + str(self.location['y']))
        else:
            logger.warning('Not enough gateways to determine the location')

        if str(device_id) in self.device_id:
            device = self.device_id[str(device_id)]

        if not self.training & len(self.queue_d7) >= 3:  # estimate location if not training, 3 or more gateways
            self.data_to_tb(device, self.location)

        self.queue_d7 = {}  # clear queue

    def data_to_tb(self, device_id, location):
        logger.info('Sending data to ThingsBoard')
        tb_telemetry = {'x': float(location['x']), 'y': float(location['y']), 'gateways': len(self.queue_d7)}

        current_ts_ms = int(round(time.time() * 1000))  # current timestamp in milliseconds, needed for Thingsboard
        self.tb.sendDeviceTelemetry(device_id, current_ts_ms, tb_telemetry)
        logger.info("Done!")


# Create devices
dev1 = Device()
while True:
    dev1.clientD7.loop()
    dev1.clientLora.loop()
