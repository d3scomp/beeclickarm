/*
 * GPS.h
 *
 *  Created on: 28. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#ifndef GPS_H_
#define GPS_H_

#include "stm32f4xx.h"
#include "UART.h"

class GPSL10 {
public:
	static constexpr auto MAX_GPS_SENTENCE_LENGTH = 255;

	GPSL10(UART& uart);
	~GPSL10();

	void init();

	inline char *getSentence() {
		return gpsSentence[validGPSSentenceIdx];
	}

	typedef void (*SentenceListener)(void *);

	void setSentenceListener(SentenceListener sentenceListener, void* obj);

private:
	UART& uart;

	static void uartRecvListenerStatic(void *obj);
	void uartRecvListener();

	SentenceListener sentenceListener;
	void* sentenceListenerObj;

	char gpsSentence[2][256];
	int validGPSSentenceIdx = 1;
	int workGPSSentenceIdx = 0;
	int workGPSSentenceBufPos = 0;
};

class GPSL30 : public GPSL10 {
public:
	struct Properties {
		GPIO_TypeDef *gpioPWR, *gpioRST, *gpioWUP;
		uint32_t pinPWR, pinRST, pinWUP;
		uint32_t clkPWR, clkRST, clkWUP;
	};

	GPSL30(Properties& initProps, UART& uart);
	~GPSL30();

	void init();

private:
	Properties props;
};

#endif /* GPS_H_ */
