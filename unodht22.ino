#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 2      
#define DHTTYPE DHT22 
#define SERIAL_BAUD 9600

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(SERIAL_BAUD);
  dht.begin();
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (!isnan(humidity) && !isnan(temperature)) {
    /* first build the string, then output it
     * this prevents partial reads from Serial.print()s */
    char output[16];  
    snprintf(output, sizeof(output), "%.1fC %.1f%%", temperature, humidity);
    Serial.println(output);
  }

  delay(60000); // 1m
}
