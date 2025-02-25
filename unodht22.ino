#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 2      
#define DHTTYPE DHT22 
#define SERIAL_BAUD 9600
#define REFRESH_RATE 2000 // 2s

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(SERIAL_BAUD);
  dht.begin();
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(humidity) || isnan(temperature))
    goto sleep;

  char temp_str[8], hum_str[8], output[32];  
  dtostrf(temperature, 4, 1, temp_str); // snprintf %f directly doesn't work
  dtostrf(humidity, 4, 1, hum_str);  
  snprintf(output, sizeof(output), "%sC %s%%\n", temp_str, hum_str);
  Serial.print(output);

sleep:
  delay(REFRESH_RATE);
}
