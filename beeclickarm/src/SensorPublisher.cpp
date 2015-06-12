#include "main.h"
#include "SensorPublisher.h"
#include "SHT1x.h"
#include "TOHQueue.h"

#include <stdint.h>

unsigned int SensorPublisher::publishersCount = 0;
SensorPublisher* SensorPublisher::publishers[MAX_PUBLISHERS];

SensorPublisher::SensorPublisher(unsigned long ticksToFire, SHT1x &sht1x, TOHQueue &tohQueue) :
		ticksToFire(ticksToFire), tickCounter(0), sht1x(sht1x), tohQueue(tohQueue) {
	// Register this publisher
	if(publishersCount < MAX_PUBLISHERS) {
		publishers[publishersCount++] = this;
	}
}

/**
 * Static tick handler for all publishers
 */
void SensorPublisher::tickHandler() {
	for(unsigned int i = 0; i < publishersCount; ++i) {
		publishers[i]->handleTick();
	}
}

/**
 * Per publisher tick handler
 */
void SensorPublisher::handleTick() {
	if (tickCounter > ticksToFire) {
		tickCounter = 0;
		publish();
	}

	tickCounter++;
}

/**
 * Publishing method
 *
 * TODO: Make this a general publisher and make this method virtual
 */
void SensorPublisher::publish() {
	publishTemperature();
	publishHumidity();
}

void SensorPublisher::publishTemperature() {
	// Publish temperature
	int16_t temp = sht1x.readTemperature();

	TOHMessage::Temperature& outMsg = tohQueue.getCurrentMsgWrite().temperature;
	outMsg.type = TOHMessage::Type::TEMPERATURE;
	outMsg.temperature = temp;

	tohQueue.moveToNextMsgWrite();
}

void SensorPublisher::publishHumidity() {
	// Publish humidity
	int16_t humid = 4242;//sht1x.readHumidity();

	TOHMessage::Humidity& outMsg = tohQueue.getCurrentMsgWrite().humidity;
	outMsg.type = TOHMessage::Type::HUMIDITY;
	outMsg.humidity = humid;

	tohQueue.moveToNextMsgWrite();
}
