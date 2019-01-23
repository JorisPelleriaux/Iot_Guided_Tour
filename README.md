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
  Repo
    |_ README <- YOU ARE HERE
    |_ Boards
    |   \_ octa
    |_ Drivers
    |   \_ lsm303agr
    |_ GPS
    |   |_ MQTT
    |   \_ Thingsboard
    \_ Handheld
        |_ README
        |_ loara_d7
        |_ main
        \_ XM1110_I2C

Onder /Handheld/README staat uitgeschreven hoe en waar de nodige libraries moeten geplaatst worden om het project te uit te voeren. 

## TODO
- [ ] Museumvisualitatie
- [ ] Merge D7, GPS tot 1 backend
- [ ] Testen met 1 en meerdere devices
- [ ] Documentatie project
- [ ] Presentatie

- [x] Fingerprinting (Joris)
- [x] GPS uitlezen (Arne)
- [x] Custom kaart in Thingsboard
- [x] Low Power emission in D7 en LoRa (Axel)
- [x] RFID onderzoek -> vervangen door button
- [x] README.md voor gebruik programma en projectstructuur
- [x] Power measurements (Axel)
- [x] database maken (MongoDB) (Joris)
- [x] D7 berichten met pushbutton (Joris)
- [x] Accelerometer (Axel)
- [x] Kaart fingerprinting maken (Axel)
