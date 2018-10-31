# Iot_Guided_Tour
## Concept

Met dit project proberen we de IoT-business uit te breiden tot in de musea. Waar audio- en videotours de lokale gidsen al vervangen, willen we dit verder uitbreiden naar het automatiseren hiervan. Een bezoeker wil graag, zonder enige moeite, informatie ontvangen van het object waar hij/zij zich voor begeeft. Dit voor zowel indoor als outdoor exposities.

## Technologie

Zowel indoor als outdoor data wordt gemanaged door Thingsboard. Dit door zijn eenvoud en simpele visualisatie. Data wordt hier verzameld met MQTT.

**INDOOR**

Aan de hand van D7 fingerprinting kunnen we de bezoeker nauwkeurig traceren binnenin het museum. De nauwkeurigheid van dit systeem hangt af van de duur van het trainingsproces. Uit verder onderzoek zal kunnen blijken of we met deze technologie genoeg hebben.
Verder wordt de accelerometer gebruikt om het systeem in deep sleep te brengen. Wanneer de gebruiker stilstaat voor een object hoeft er niet onnodig data verzonder te worden.

**OUTDOOR**

Door gebrek aan LoRa-Gateways in onze omgeving zijn we niet in staat om op grote afstand low-power te traceren. We gebruiken hiervoor de GPS-coördinaten die we versturen via het LoRaWAN netwerk.


##TODO
[] Fingerprinting
