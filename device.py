from localization import Localization
import paho.mqtt.client as mqtt
import threading
import time
import logging
import json
from thingsboard import Thingsboard
from AlpParser import parse_alp

# Create global logger
#logger = logging.getLogger('tb_example')
#formatstring = "%(asctime)s - %(name)s:%(funcName)s:%(lineno)i - %(levelname)s - %(message)s"
#logging.basicConfig(format=formatstring, level=logging.DEBUG)

with open('keys.json') as f:
    keys = json.load(f)

class Device:
    def __init__(self):
        self.localization = Localization('127.0.0.1', 'FingerprintDB', 'DataSet')
        self.device_id = '4933323400370020'
        self.processor = threading.Thread()  # empty thread
        self.training = False

        self.x_training = 1
        self.y_training = 0
        self.queue_d7 = {}

        # Subscribe to Dash-7
        try:
            self.client = mqtt.Client()
            self.client.on_message = self.on_message

            print('connecting to broker')
            self.client.connect("backend.idlab.uantwerpen.be", 1883, 60)
            self.client.subscribe('/d7/' + self.device_id + '/#')
        except:
            print ('connection to Dash-7 failed')

        self.tb = Thingsboard(keys['thingsboard']['url'], 1883, keys['thingsboard']['access_token'])

    def on_message(self, client, userdata, msg):
        raw = str(msg.payload.decode('utf-8'))
        topic = msg.topic.split("/")
        hardware_id = topic[2]
        gateway_id = topic[3]
        dict = parse_alp(raw)
        print ('ontvangen van ' + gateway_id)
        self.queue_d7[gateway_id] = int(dict['rx_level'])  # save rx_level for every receiving gateway

        if not self.processor.is_alive():
            print('Thread started')
            self.processor = threading.Thread(target=self.process_data_counter, args=[dict['data'], hardware_id])
            print('Thread created')
            self.processor.start()
        print('Thread started')

    def process_data_counter(self, data, device_id):
        time.sleep(3)
        print('queue', self.queue_d7)
        #training mode
        if self.training:
            self.localization.training(self.x_training, self.y_training, self.queue_d7)
            print('added to DB')

        #localize mode
        location = self.localization.localize(self.queue_d7, 20) #k-nearest
        print ('Location is approximately x: ' + str(location['x']) + ', ' + 'y: ' + str(location['y']))

        if not self.training:
            self.data_to_tb('IoT_Tour_Joris', location)

        self.queue_d7 = {}  #clear queue

    def data_to_tb(self, device_id, location):
        print('Sending data to ThingsBoard')
        tb_telemetry = {'x': float(location['x']),'y': float(location['y']), 'gateways': len(self.queue_d7)}

        current_ts_ms = int(round(time.time() * 1000))  # current timestamp in milliseconds, needed for Thingsboard
        self.tb.sendDeviceTelemetry(device_id, current_ts_ms, tb_telemetry)
        #logger.info("Done!")

# Create devices
dev1 = Device()
while True:
    dev1.client.loop()
