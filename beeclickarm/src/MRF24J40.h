/*
 * MRF24J40.h
 *
 *  Created on: 14. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#ifndef MRF24J40_H_
#define MRF24J40_H_

class MRF24J40 {
public:
	MRF24J40();
	virtual ~MRF24J40();

	void setChannel(int channelNo);
};

#endif /* MRF24J40_H_ */
