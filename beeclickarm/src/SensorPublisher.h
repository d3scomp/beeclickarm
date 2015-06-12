#ifndef SENSOR_PUBLISHER_H
#define SENSOR_PUBLISHER_H

#include "SHT1x.h"
#include "TOHQueue.h"

class SensorPublisher {
public:
	SensorPublisher(unsigned long ticksToFire, SHT1x &sht1x, TOHQueue &tohQueue);
	static void tickHandler();

private:
	static const unsigned int MAX_PUBLISHERS = 5;
	static unsigned int publishersCount;
	static SensorPublisher* publishers[MAX_PUBLISHERS];


	const unsigned long ticksToFire;
	unsigned long tickCounter;

	SHT1x &sht1x;
	TOHQueue &tohQueue;

	void handleTick();
	void publish();

	void publishTemperature();
	void publishHumidity();
};

#endif
