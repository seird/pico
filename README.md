# Remote temperature measurement

Measure temperature on a remote location. The measurement is sent with an nrf24l01 transceiver to a server. An intermediate relay node is used to cover more distance.

Parts:
- **Temperature measurement**:
    - pico
    - nrf24l01+
    - DS18B20
- **Relay**:
    - pico
    - nrf24l01+
- **Server**:
    - raspberry pi
    - nrf24l01+


## Temperature measurement

~1.17mA @ SM_SLEEP

See the [code](src/temperature/main.cpp).

![layout](images/temperature.png)

## Relay

See the [code](src/relay/main.cpp).

![layout](images/relay.png)
