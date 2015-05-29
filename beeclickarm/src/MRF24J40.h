/*
 * MRF24J40.h
 *
 *  Created on: 14. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#ifndef MRF24J40_H_
#define MRF24J40_H_

#include "stm32f4xx.h"
#include "LED.h"
#include "TOHQueue.h"

class MRF24J40 {
public:
	struct Properties {
		GPIO_TypeDef* gpioRST;
		GPIO_TypeDef* gpioCS;
		GPIO_TypeDef* gpioTXRX;
		GPIO_TypeDef* gpioRFINT;
		SPI_TypeDef* spi;
		uint32_t pinRST;
		uint32_t pinCS;
		uint32_t pinSCK;
		uint32_t pinMISO;
		uint32_t pinMOSI;
		uint32_t pinRFINT;
		uint8_t pinSourceRST;
		uint8_t pinSourceCS;
		uint8_t pinSourceSCK;
		uint8_t pinSourceMISO;
		uint8_t pinSourceMOSI;
		uint32_t clkGPIOs;
		void (*clkSPICmdFun)(uint32_t periph, FunctionalState newState);
		uint32_t clkSPI;
		uint8_t afConfig;
		uint32_t extiLineRFINT;
		uint8_t extiPortSourceRFINT;
		uint8_t extiPinSourceRFINT;
		IRQn irqnRFINT;
		IRQn irqnSPI;
	};

	typedef void (*RecvListener)(void *);
	typedef void (*BroadcastCompleteListener)(void *, bool);

	MRF24J40(Properties& initProps, PulseLED& recvLed, PulseLED& sendLed);
	~MRF24J40();

	void init();
	void reset();

	void rfInterruptHandler();

	inline void spiInterruptHandler() {
		if (props.spi->SR & SPI_I2S_FLAG_RXNE) {
			lastReadValue = props.spi->DR;
		}
	}

	void setRecvListener(RecvListener recvListener, void* obj);
	void setBroadcastCompleteListener(BroadcastCompleteListener broadcastCompleteListener, void* obj);

	void setRFPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority);
	void setSPIPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority);

	void setChannel(uint8_t channel);
	uint8_t readChannel();

	/**
	 * Sets output Tx power in dB
	 *
	 * Supported range is from -36.3 dB to 0 dB
	 *
	 * @param power
	 *            Requested Tx power in dB * 10 (-36.3 -> -363)
	 */
	void setTxPower(int16_t power);

	void setPANId(uint8_t panId[2]);
	uint16_t readPANId();

	void setSAddr(uint8_t sAddr[2]);
	uint16_t readSAddr();

	void broadcastPacket(uint8_t *data, uint8_t dataLength);
	bool recvPacket(uint8_t (&data)[TOHMessage::MAX_RF_PACKET_LENGTH], uint8_t& dataLength, uint8_t (&srcPanId)[2], uint8_t (&srcSAddr)[2], uint8_t (&fcs)[2], uint8_t& lqi, uint8_t& rssi);


	uint8_t getChannel() const {
		return channel;
	}

	uint16_t getPANId() const {
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
	Properties props;

	PulseLED& recvLed;
	PulseLED& sendLed;

	uint8_t channel;
	uint8_t panId[2];
	uint8_t sAddr[2];

	int txCount;
	int rxCount;

	uint8_t irqSPIPreemptionPriority;
	uint8_t irqSPISubPriority;
	uint8_t irqRFPreemptionPriority;
	uint8_t irqRFSubPriority;

	RecvListener recvListener;
	void* recvListenerObj;

	BroadcastCompleteListener broadcastCompleteListener;
	void* broadcastCompleteListenerObj;

	void writeShort(uint8_t addr, uint8_t value);
	void writeLong(uint16_t addr, uint8_t value);
	uint8_t readShort(uint8_t addr);
	uint8_t readLong(uint16_t addr);

	uint8_t lastReadValue;

	/* short registers */
	static constexpr uint8_t RXMCR = 0x00;
	static constexpr uint8_t PANIDL = 0x01;
	static constexpr uint8_t PANIDH = 0x02;
	static constexpr uint8_t SADRL = 0x03;
	static constexpr uint8_t SADRH = 0x04;
	static constexpr uint8_t EADR0 = 0x05;
	static constexpr uint8_t EADR1 = 0x06;
	static constexpr uint8_t EADR2 = 0x07;
	static constexpr uint8_t EADR3 = 0x08;
	static constexpr uint8_t EADR4 = 0x09;
	static constexpr uint8_t EADR5 = 0x0A;
	static constexpr uint8_t EADR6 = 0x0B;
	static constexpr uint8_t EADR7 = 0x0C;
	static constexpr uint8_t RXFLUSH = 0x0D;
	static constexpr uint8_t ORDER = 0x10;
	static constexpr uint8_t TXMCR = 0x11;
	static constexpr uint8_t ACKTMOUT = 0x12;
	static constexpr uint8_t ESLOTG1 = 0x13;
	static constexpr uint8_t SYMTICKL = 0x14;
	static constexpr uint8_t SYMTICKH = 0x15;
	static constexpr uint8_t PACON0 = 0x16;
	static constexpr uint8_t PACON1 = 0x17;
	static constexpr uint8_t PACON2 = 0x18;
	static constexpr uint8_t TXBCON0 = 0x1A;
	static constexpr uint8_t TXNCON = 0x1B;
	static constexpr uint8_t TXG1CON = 0x1C;
	static constexpr uint8_t TXG2CON = 0x1D;
	static constexpr uint8_t ESLOTG23 = 0x1E;
	static constexpr uint8_t ESLOTG45 = 0x1F;
	static constexpr uint8_t ESLOTG67 = 0x20;
	static constexpr uint8_t TXPEND = 0x21;
	static constexpr uint8_t WAKECON = 0x22;
	static constexpr uint8_t FRMOFFSET = 0x23;
	static constexpr uint8_t TXSTAT = 0x24;
	static constexpr uint8_t TXBCON1 = 0x25;
	static constexpr uint8_t GATECLK = 0x26;
	static constexpr uint8_t TXTIME = 0x27;
	static constexpr uint8_t HSYMTMRL = 0x28;
	static constexpr uint8_t HSYMTMRH = 0x29;
	static constexpr uint8_t SOFTRST = 0x2A;
	static constexpr uint8_t SECCON0 = 0x2C;
	static constexpr uint8_t SECCON1 = 0x2D;
	static constexpr uint8_t TXSTBL = 0x2E;
	static constexpr uint8_t RXSR = 0x30;
	static constexpr uint8_t INTSTAT = 0x31;
	static constexpr uint8_t INTCON = 0x32;
	static constexpr uint8_t GPIO = 0x33;
	static constexpr uint8_t TRISGPIO = 0x34;
	static constexpr uint8_t SLPACK = 0x35;
	static constexpr uint8_t RFCTL = 0x36;
	static constexpr uint8_t SECCR2 = 0x37;
	static constexpr uint8_t BBREG0 = 0x38;
	static constexpr uint8_t BBREG1 = 0x39;
	static constexpr uint8_t BBREG2 = 0x3A;
	static constexpr uint8_t BBREG3 = 0x3B;
	static constexpr uint8_t BBREG4 = 0x3C;
	static constexpr uint8_t BBREG6 = 0x3E;
	static constexpr uint8_t CCAEDTH = 0x3F;

	/* long registers */
	static constexpr uint16_t RFCON0 = 0x200;
	static constexpr uint16_t RFCON1 = 0x201;
	static constexpr uint16_t RFCON2 = 0x202;
	static constexpr uint16_t RFCON3 = 0x203;
	static constexpr uint16_t RFCON5 = 0x205;
	static constexpr uint16_t RFCON6 = 0x206;
	static constexpr uint16_t RFCON7 = 0x207;
	static constexpr uint16_t RFCON8 = 0x208;
	static constexpr uint16_t SLPCAL0 = 0x209;
	static constexpr uint16_t SLPCAL1 = 0x20A;
	static constexpr uint16_t SLPCAL2 = 0x20B;
	static constexpr uint16_t RFSTATE = 0x20F;
	static constexpr uint16_t RSSI = 0x210;
	static constexpr uint16_t SLPCON0 = 0x211;
	static constexpr uint16_t SLPCON1 = 0x220;
	static constexpr uint16_t WAKETIMEL = 0x222;
	static constexpr uint16_t WAKETIMEH = 0x223;
	static constexpr uint16_t REMCNTL = 0x224;
	static constexpr uint16_t REMCNTH = 0x225;
	static constexpr uint16_t MAINCNT0 = 0x226;
	static constexpr uint16_t MAINCNT1 = 0x227;
	static constexpr uint16_t MAINCNT2 = 0x228;
	static constexpr uint16_t MAINCNT3 = 0x229;
	static constexpr uint16_t ASSOEADR0 = 0x230;
	static constexpr uint16_t ASSOEADR1 = 0x231;
	static constexpr uint16_t ASSOEADR2 = 0x232;
	static constexpr uint16_t ASSOEADR3 = 0x233;
	static constexpr uint16_t ASSOEADR4 = 0x234;
	static constexpr uint16_t ASSOEADR5 = 0x235;
	static constexpr uint16_t ASSOEADR6 = 0x236;
	static constexpr uint16_t ASSOEADR7 = 0x237;
	static constexpr uint16_t ASSOSADR0 = 0x238;
	static constexpr uint16_t ASSOSADR1 = 0x239;
	static constexpr uint16_t UPNONCE0 = 0x240;
	static constexpr uint16_t UPNONCE1 = 0x241;
	static constexpr uint16_t UPNONCE2 = 0x242;
	static constexpr uint16_t UPNONCE3 = 0x243;
	static constexpr uint16_t UPNONCE4 = 0x244;
	static constexpr uint16_t UPNONCE5 = 0x245;
	static constexpr uint16_t UPNONCE6 = 0x246;
	static constexpr uint16_t UPNONCE7 = 0x247;
	static constexpr uint16_t UPNONCE8 = 0x248;
	static constexpr uint16_t UPNONCE9 = 0x249;
	static constexpr uint16_t UPNONCE10 = 0x24A;
	static constexpr uint16_t UPNONCE11 = 0x24B;
	static constexpr uint16_t UPNONCE12 = 0x24C;

	static constexpr uint16_t RXFIFO = 0x300;
	static constexpr uint16_t TXNFIFO = 0x000;
};

#endif /* MRF24J40_H_ */
