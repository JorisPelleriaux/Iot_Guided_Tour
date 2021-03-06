from pymongo import MongoClient
import numpy as np


class Localization:
    gateway_ids = ['4337313400210032', '433731340023003d', '42373436001c0037', '463230390032003e']

    def __init__(self, host, database, collection):
        try:
            # connect to MongoDB
            dbclient = MongoClient(host)
            db = dbclient[database]
            self.collection = db[collection]
        except:
            print('cannot connect to database')

    def training(self, x, y, rx):
        document = {'x': x, 'y': y, 'gateways': rx}
        self.collection.insert_one(document)

    def localize(self, rx, k):
        probabilistic = []
        for document in self.collection.find():
            diff = []
            for gateway_id in self.gateway_ids:
                if gateway_id not in rx:
                    rx[gateway_id] = 200  # out of range
                if gateway_id not in document['gateways']:
                    document['gateways'][gateway_id] = 200  # out of range
                diff.append(int(rx[gateway_id]) - int(document['gateways'][gateway_id]))
            rms = np.sqrt(np.mean(np.square(diff)))
            man = np.mean(np.absolute(diff))
            probabilistic.append({'x': document['x'], 'y': document['y'], 'rms': rms})
            # probabilistic.append({'x': document['x'], 'y': document['y'], 'man': man})

        # k-nearest neighbors
        ordered_locations = sorted(probabilistic, key=lambda i: i['rms'])  # sort on RMS value
        # ordered_locations = sorted(probabilistic, key=lambda i: i['man'])  # sort on manhattan value
        nearest_neighbors = ordered_locations[:k]
        print('knn: ' + str(nearest_neighbors))

        # Weighted Average
        x = 0
        y = 0
        total_weight = 0
        for fingerprint in nearest_neighbors:
            if fingerprint['rms'] == 0:
                weight = 100
            else:
                weight = 1 / fingerprint['rms']
            x += int(fingerprint['x']) * weight
            y += int(fingerprint['y']) * weight
            total_weight += weight
        x = x / total_weight
        y = y / total_weight

        return {'x': x, 'y': y}
