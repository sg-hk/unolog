# DHT22 to FIFO

Reads DHT22 sensor data on an Arduino Uno and sends it to a named pipe.

## Arduino Code
- Reads temp/humidity every minute
- Formats it as "00.0C 00.0%"
- Sends data via serial

## C Code
- Reads `/dev/ttyUSB0` with `select()`, avoiding double sleep()s
- Writes to `/tmp/bar/fifo_dht22`
