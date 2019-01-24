# Iot_Guided_Tour
## Concept

Met dit project proberen we de IoT-business uit te breiden tot in musea. Waar audio- en videotours de lokale gidsen al vervangen, willen we dit verder uitbreiden naar het automatiseren hiervan. Een bezoeker wil graag, zonder enige moeite, informatie ontvangen van het object waar hij/zij zich voor begeeft. Dit voor zowel indoor als outdoor exposities.

## Technologie

Zowel indoor als outdoor data wordt gemanaged door Thingsboard. Dit door zijn eenvoud en simpele visualisatie. Data wordt hier verzameld met MQTT.

**INDOOR**

Aan de hand van D7 fingerprinting kunnen we de bezoeker nauwkeurig traceren binnenin het museum. De nauwkeurigheid van dit systeem hangt af van de duur van het trainingsproces. Uit verder onderzoek zal kunnen blijken of we met deze technologie genoeg hebben.
Verder wordt de accelerometer gebruikt om het systeem in deep sleep te brengen. Wanneer de gebruiker stilstaat voor een object hoeft er niet onnodig data verzonden te worden.

**OUTDOOR**

Door gebrek aan LoRa-Gateways in onze omgeving zijn we niet in staat om op grote afstand low-power LoRaWAN-triangulatie uit te voeren. We gebruiken hiervoor de GPS-coördinaten die we versturen via het LoRaWAN netwerk.

## Hardware
* NUCLEO-L496ZG (STM32 Nucleo-144 development board)
* OCTA-Connect development board
* sierra wireless xm1110 (GNSS positioning module)
* CMWX1ZZABZ (LPWAN wireless module)

## Projectstructuur

De repository is opgedeeld in volgende structuur
```bash
.
├── README <- YOU ARE HERE  
├── Boards  
|   └── octa  
├── Drivers  
|   └── lsm303agr  
├── Database  
|   ├── FingerprintDB  
├── Handheld  
|   ├── README  
|   ├── loara_d7  
|   ├── main  
|   └── XM1110_I2C  
├── device.py
├── localization.py
└── thingsboard.py

```

De backend wordt gestart met het device.py-script. Deze gebruikt localization.py om het localisatiealgoritme uit te voeren en thingsboard.py als API van thingsboard.

De handheldapplicatie kan gevonden worden onder Handheld. Daar staat uitgeschreven hoe en waar de nodige libraries moeten geplaatst worden om het project te uit te voeren. 

Onder Database kan men de gebruikte database voor het fingerprintingproces terugvinden.

Boards en Drivers bevatten bestanden die moeten gekopiëerd worden naar de aangewezen locaties die omschreven staan in Handheld/README.md.
