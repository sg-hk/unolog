#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 2        // pin where DHT22 is connected
#define DHTTYPE DHT22   // define sensor type (DHT22)

DHT dht(DHTPIN, DHTTYPE);

void setup() {
        Serial.begin(9600);
        dht.begin();
}

void loop() {
        float temp = dht.readTemperature();
        float hum = dht.readHumidity();  

        if (isnan(temp) || isnan(hum)) {
                Serial.println("Failed to read from DHT sensor"); 
        } else {
                Serial.print(temp);
                Serial.print(",");
                Serial.println(hum);
        }

        delay(60000); // read every minute
}
