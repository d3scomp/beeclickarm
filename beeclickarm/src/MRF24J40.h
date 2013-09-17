/*
 * MRF24J40.h
 *
 *  Created on: 14. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#ifndef MRF24J40_H_
#define MRF24J40_H_

#include "stm32f4xx.h"

class MRF24J40 {
public:
	MRF24J40();
	~MRF24J40();

	void setPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority);
	void init();

	void setChannel(uint8_t channel);
	void setAddr(uint8_t panId[2], uint8_t sAddr[2]);
	void sendPacket(uint8_t *data, uint8_t length);

	uint8_t getChannel() const {
		return channel;
	}

	uint16_t getPanId() const {
		return panId[1] << 8 | panId[0];
	}

	uint16_t getSAddr() const {
		return sAddr[1] << 8 | sAddr[0];
	}

	int getRXCount() const {
		return rxCount;
	}

	int getTXCount() const {
		return txCount;
	}

private:
	uint8_t channel;
	uint8_t panId[2];
	uint8_t sAddr[2];

	int txCount;
	int rxCount;

	uint8_t irqPreemptionPriority;
	uint8_t irqSubPriority;

};

#endif /* MRF24J40_H_ */
