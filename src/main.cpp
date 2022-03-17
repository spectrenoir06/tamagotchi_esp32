#include <Arduino.h>

void setup() {
	Serial.begin(115200);
	Serial.printf("Start\n");
}

void loop() {
	vTaskDelay(1 / portTICK_PERIOD_MS);
}	